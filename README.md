# 📦 Counting fasteners project with YOLOv11

## 🧠 Mục tiêu dự án

Dự án này sử dụng **YOLOv11** để phát hiện và đếm các **phụ kiện công nghiệp** từ hình ảnh. Hệ thống được chia làm ba phần:

- 🧠 **Model**: Sử dụng YOLOv11 để huấn luyện và phát hiện phụ kiện, sử dụng ONNX giúp việc xử lý nhanh hơn.
- 🌐 **Web**: Ứng dụng web xây dựng bằng **Next.js** (frontend) và **NestJS** (backend) để tải ảnh và hiển thị kết quả.
- **ESP32**: Sử dụng esp32-cam để chụp ảnh và gửi lên server để đếm và hiển thị kết quả lên màn hình

---

## 📁 Cấu trúc thư mục

```
├── model/ # YOLOv11 model, training and inference scripts
│ ├── train/ # train mô hình với google colab
│ └── detect/ # sử dụng mô hình đã được train
├── esp32/ # code firmware cho esp32
├── web/
│ ├── frontend/ # Next.js frontend
│ └── backend/ # NestJS backend API
└── README.md
```

---

## 🚀 Tính năng chính

- Phát hiện và đếm các phụ kiện công nghiệp từ ảnh
- Giao diện web trực quan, dễ sử dụng
- Hệ thống backend hỗ trợ phân tích ảnh và quản lý dữ liệu
- Có thể mở rộng với nhiều loại phụ kiện khác nhau
- Xử lý request nhanh chóng với ONNX
- Có triển khai trên thiết bị với bộ nhớ và tốc độ hạn chế là esp32

---

## 🔧 Cài đặt

### 1. Yêu cầu hệ thống

- Python 3.8+
- Node.js 20+

---
### 2. Clone project

```bash
git clone https://github.com/baokieuv/counting_fasteners_project.git
```
---

### 3. Cài đặt YOLOv11 (Model) 
- Có thể up ảnh và chạy đơn lẻ trực tiếp trên máy cá nhân

```bash
cd model
pip install -r requirements.txt
# Chạy thử nghiệm
python detect/model.py --input test.jpg --type washer
```
### 4. Cài đặt Web frontend (Next.js)

```bash
cd web/frontend
npm install
npm run build
npm run dev
# Ứng dụng frontend sẽ chạy tại http://localhost:3001
```
### 5.  Cài đặt backend API (NestJS)

```bash
cd web/backend
npm install
npm run build
npm run start:prod
# API sẽ chạy tại http://localhost:3000
```
### 6.  Cài đặt và chạy socket python

```bash
cd web/backend/model
pip install -r requirements.txt
python socket_server.py
# code sẽ mở 1 socket TCP lắng nghe tại cổng 8888
```
## Demo

Web đã được triển khai tại: https://kvbhust.site

![Giao diện web](https://github.com/user-attachments/assets/b09d710a-592b-4fbf-a917-5e21b96efb83)
![Web khi detect object](https://github.com/user-attachments/assets/59787d8d-23fb-4cf0-950f-0c8d4cf0e9c7)
![ESP32](https://github.com/user-attachments/assets/32bf701c-3caf-42ea-8a3c-ea23bbf4d5bb)



