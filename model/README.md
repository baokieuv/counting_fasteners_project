# 🧠 Counting Fasteners – YOLOv11 Model

## 🗂️ Thông tin chung

Phần này là module **model** của dự án đếm phụ kiện công nghiệp (bu lông, ốc vít, long đen, v.v.) sử dụng **YOLOv11**.

- Mô hình được huấn luyện trên Google Colab.
- Sau khi huấn luyện, mô hình được chuyển sang định dạng **ONNX** để tăng tốc suy luận nhờ **ONNX Runtime**.
- Code phát hiện (detect) chạy cục bộ, sử dụng mô hình đã huấn luyện.

---

## 🔧 Cấu trúc module model
```
model/
├── train/ # Code huấn luyện mô hình (YOLOv11 - Colab)
│ ├── project2.ipynb # Notebook huấn luyện trên Google Colab
├── detect/ # Sử dụng mô hình đã huấn luyện để phát hiện
│ ├── model.py # Mã nguồn phát hiện dùng ONNX
├── requirements.txt
├── best.onnx
└── README.md
```


---

## 🚀 Hướng dẫn sử dụng

### 📦 Bước 1: Cài đặt thư viện

```bash
cd model/detect
pip install -r requirements.txt
```
### ▶️ Bước 2: Chạy mô hình

```
python model.py --input "duong_dan_anh.jpg" --type "loai_phu_kien"
```
### 📸 Kết quả

Ảnh sau xử lý sẽ hiển thị kết quả nhận diện:

![washer_20250528_224427](https://github.com/user-attachments/assets/b512d8e8-c225-4db2-b34c-8c3717b6c3f2)

## Đánh giá mô hình

Confusion matrix:

![confusion_matrix](https://github.com/user-attachments/assets/7c051f0b-9ba1-41c6-b777-33cbf003828d)

Biểu đồ đánh giá:
![results](https://github.com/user-attachments/assets/c2fa11a0-f96c-480c-bc68-8f31ff7f9595)


## ✨ Tính năng chính
- Nhận diện chính xác các loại phụ kiện công nghiệp phổ biến.
- Sử dụng mô hình nhẹ và nhanh nhờ ONNX Runtime.
- Dễ dàng triển khai trong hệ thống có giao diện web hoặc camera.
