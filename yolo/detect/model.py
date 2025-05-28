import os
import cv2
import numpy as np
import onnxruntime as ort
from typing import List
import argparse
from datetime import datetime

class Yolov11_Onnx:
    def __init__(self, onnx_model_path: str, input_shape: tuple[int, int] = (640, 640), 
                 confidence_threshold: float = 0.3, nms_threshold: float = 0.65, 
                 label_list: List[str] = None):
        """Khởi tạo model ONNX"""
        self.onnx_model_path = onnx_model_path
        self.input_shape = input_shape
        self.confidence_threshold = confidence_threshold
        self.nms_threshold = nms_threshold
        self.label_list = label_list if label_list else ["Washer"]

        # Load mô hình ONNX
        self.session = ort.InferenceSession(self.onnx_model_path)
        
    def _preprocessing(self, frame):
        """Tiền xử lý ảnh"""
        original_height, original_width = frame.shape[:2]
        self.resize_ratio_w = original_width / self.input_shape[0]
        self.resize_ratio_h = original_height / self.input_shape[1]

        # Resize ảnh
        input_img = cv2.resize(frame, self.input_shape)
        input_img = cv2.cvtColor(input_img, cv2.COLOR_BGR2RGB)  # Chuyển sang RGB
        input_img = input_img.transpose(2, 0, 1)  # Đổi từ HWC -> CHW
        input_img = np.ascontiguousarray(input_img) / 255.0
        input_tensor = input_img[np.newaxis, :, :, :].astype(np.float32)

        return input_tensor
    
    def _postprocessing(self, output):
        """Hậu xử lý kết quả mô hình"""
        output = np.array(output)
        x_center, y_center, w, h = output[0, 0, :4, :]
        confidence = output[0, 0, 4:, :]
                       
        class_id = np.argmax(confidence, axis=0)
        max_class_prob = np.max(confidence, axis=0)

        # Lọc các bounding box có độ tin cậy lớn hơn ngưỡng
        mask = max_class_prob > self.confidence_threshold
        detections = [
            [
                x_center[i] * self.resize_ratio_w,  
                y_center[i] * self.resize_ratio_h,  
                w[i] * self.resize_ratio_w,         
                h[i] * self.resize_ratio_h,         
                class_id[i],  
                max_class_prob[i]
            ]
            for i in range(len(mask)) if mask[i]
        ]

        # Áp dụng NMS để loại bỏ box trùng lặp
        if detections:
            boxes = np.array([[int(d[0] - d[2] / 2), int(d[1] - d[3] / 2), d[2], d[3]] for d in detections])
            confidences = np.array([d[5] for d in detections])
            indices = cv2.dnn.NMSBoxes(boxes.tolist(), confidences.tolist(), self.confidence_threshold, self.nms_threshold)
      
            if len(indices) > 0:
                detections = [detections[i] for i in indices.flatten()]

        return detections
    
    def drawbox(self, frame, detections):
        """Vẽ bounding box lên ảnh"""
        num_object = 0
        for x_center, y_center, w, h, class_id, conf in detections:
            x, y = x_center - w / 2, y_center - h / 2
            x_max, y_max = x_center + w / 2, y_center + h / 2
            class_name = self.label_list[class_id]

            if class_name == "Washer":
                num_object += 1

            cv2.rectangle(frame, (int(x), int(y)), (int(x_max), int(y_max)), (0, 255, 0), 2)
            cv2.putText(frame, class_name, (int(x), int(y) - 5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)

        cv2.putText(frame, f"Object(s): {num_object}", (10, 30), 
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        return frame
    
def detect_and_save(model: Yolov11_Onnx, image_path, type, output_dir="detection"):
    frame = cv2.imread(image_path)
    if frame is None:
        print(f"⚠️ Không thể đọc ảnh: {image_path}")


    input_tensor = model._preprocessing(frame)
    
    input_name = model.session.get_inputs()[0].name
    output = model.session.run(None, {input_name: input_tensor})

    detections = model._postprocessing(output)
    result_frame = model.drawbox(frame, detections)
    
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    output_path = os.path.join(output_dir, f"{type}_{timestamp}.jpg")
    cv2.imwrite(output_path, result_frame)
    print(f"Đã lưu ảnh tại: {output_path}")
    
    # cv2.imshow("Detection Result", result_frame)
    # cv2.waitKey(0)  # Hiển thị nhanh từng ảnh
       
def main():
    model_path = "D:/code/projectTest/computer_vision/project2/best.onnx"
    washer_model = Yolov11_Onnx(model_path)
    
    parser = argparse.ArgumentParser(description="YoloV11 Object Detection")
    parser.add_argument("--input", type=str, required=True, help="Đường dẫn đến ảnh đầu vào")
    parser.add_argument("--type", type=str, required=True, help="Loại object")
    parser.add_argument("--output", type=str, required=True, help="Đường dẫn để lưu ảnh đầu ra")
    args = parser.parse_args()
    
    if not os.path.exists(args.input):
        print(f"⚠️ Đường dẫn ảnh không tồn tại: {args.input}")
        return
    
    if args.type == "washer":
        detect_and_save(washer_model, args.input, args.type)
    elif args.type == "dryer":
        pass  # Chưa có model cho dryer
    elif args.type == "fridge":
        pass
    else:
        print("⚠️ Loại object không hợp lệ. Vui lòng chọn 'washer', 'dryer' hoặc 'fridge'.")
        return
        
    
if __name__ == "__main__":
    main()