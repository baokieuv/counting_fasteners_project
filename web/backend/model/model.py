import os
import cv2
import numpy as np
import onnxruntime as ort
from typing import List
from datetime import datetime
import logging

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

class Yolov11_Onnx:
    def __init__(self, onnx_model_path: str, input_shape: tuple[int, int] = (640, 640), 
                 confidence_threshold: float = 0.3, nms_threshold: float = 0.65, 
                 label_list: List[str] = None):
        """Khởi tạo model ONNX"""
        self.onnx_model_path = onnx_model_path
        self.input_shape = input_shape
        self.confidence_threshold = confidence_threshold
        self.nms_threshold = nms_threshold
        self.label_list = label_list if label_list else ["Object"]

        # Load mô hình ONNX
        if not os.path.exists(onnx_model_path):
            raise FileNotFoundError(f"Model file not found: {onnx_model_path}")
        self.session = ort.InferenceSession(self.onnx_model_path)
        logging.info(f"Loaded ONNX model from {onnx_model_path}")
        
    def _preprocessing(self, frame):
        """Tiền xử lý ảnh"""
        if frame is None:
            raise ValueError("Invalid input image")
        original_height, original_width = frame.shape[:2]
        self.original_size = (original_width, original_height)

        max_dim = max(original_height, original_width)
        top = (max_dim - original_height) // 2
        bottom = max_dim - original_height - top
        left = (max_dim - original_width) // 2
        right = max_dim - original_width - left
        
        self.padding = (top, bottom, left, right)
        padded_img = cv2.copyMakeBorder(frame, top, bottom, left, right, borderType=cv2.BORDER_CONSTANT, value=(114, 114, 114))
        
        # Resize ảnh
        input_img = cv2.resize(padded_img, self.input_shape)
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

        pad_top, pad_bottom, pad_left, pad_right = self.padding
        padded_size = max(self.original_size)
        # Lọc các bounding box có độ tin cậy lớn hơn ngưỡng
        mask = max_class_prob > self.confidence_threshold
        detections = [
            [
                x_center[i] * padded_size / self.input_shape[0] - pad_left,  
                y_center[i] * padded_size / self.input_shape[1] - pad_top,  
                w[i] * padded_size / self.input_shape[0],         
                h[i] * padded_size / self.input_shape[1],         
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
        num_object = len(detections)
        
        for x_center, y_center, w, h, class_id, conf in detections:
            x, y = x_center - w / 2, y_center - h / 2
            x_max, y_max = x_center + w / 2, y_center + h / 2
            class_name = self.label_list[class_id]

            cv2.rectangle(frame, (int(x), int(y)), (int(x_max), int(y_max)), (0, 255, 0), 2)
            cv2.putText(frame, class_name, (int(x), int(y) - 5), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (255, 255, 255), 2)

        cv2.putText(frame, f"Object(s): {num_object}", (10, 30), 
                    cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0), 2)

        return [frame, num_object]
    
def detect_and_save(model: Yolov11_Onnx, image_path, type, output_dir="uploads"):
    if not os.path.exists(image_path):
        raise FileNotFoundError(f"Image not found: {image_path}")
    
    frame = cv2.imread(image_path)
    if frame is None:
        return ValueError(f"Failed to load image: {image_path}")


    input_tensor = model._preprocessing(frame)
    
    input_name = model.session.get_inputs()[0].name
    output = model.session.run(None, {input_name: input_tensor})

    detections = model._postprocessing(output)
    result_frame, numObjects = model.drawbox(frame, detections)
        
    base_dir = os.path.dirname(os.path.abspath(__file__))
    upload_dir = os.path.join(os.path.dirname(base_dir), output_dir)
    
    results_dir = os.path.join(upload_dir, "results")
    labels_dir = os.path.join(upload_dir, "labels")
    
    os.makedirs(results_dir, exist_ok=True)
    os.makedirs(labels_dir, exist_ok=True)
    
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    
    image_filename = f"{type}_{timestamp}.jpg"
    image_path = os.path.join(results_dir, image_filename)
    cv2.imwrite(image_path, result_frame)
    
    label_filename = f"{type}_{timestamp}.txt"
    label_path = os.path.join(labels_dir, label_filename)
    
    with open(label_path, "w") as f:
        for x_center, y_center, w, h, class_id, conf in detections:
            x_min, y_min = x_center - w / 2, y_center - h / 2
            x_max, y_max = x_center + w / 2, y_center + h / 2
            f.write(f"{int(x_min)} {int(y_min)} {int(x_max)} {int(y_max)}\n")
    
    return [os.path.join("uploads", "results", image_filename), os.path.join("uploads", "labels", label_filename), numObjects]