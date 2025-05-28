# 📦 Counting fasteners project with YOLOv11

## 🧠 Mục tiêu dự án

Dự án này sử dụng **YOLOv11** để phát hiện và đếm các **phụ kiện công nghiệp** từ hình ảnh. Hệ thống được chia làm hai phần:

- 🧠 **Yolo**: Sử dụng YOLOv11 để huấn luyện và phát hiện phụ kiện.
- 🌐 **Web**: Ứng dụng web xây dựng bằng **Next.js** (frontend) và **NestJS** (backend) để tải ảnh và hiển thị kết quả.

---

## 📁 Cấu trúc thư mục

.
├── model/ # YOLOv11 model, training and inference scripts
├── web/
│ ├── frontend/ # Next.js frontend
│ └── backend/ # NestJS backend API
├── data/ # Dữ liệu huấn luyện (tùy chọn)
└── README.md


---

## 🚀 Tính năng chính

- Phát hiện và đếm các phụ kiện công nghiệp từ ảnh hoặc video
- Giao diện web trực quan, dễ sử dụng
- Hệ thống backend hỗ trợ phân tích ảnh và quản lý dữ liệu
- Có thể mở rộng với nhiều loại phụ kiện khác nhau

---

## 🔧 Cài đặt

### 1. Yêu cầu hệ thống

- Python 3.8+
- Node.js 18+

---

### 2. Cài đặt YOLOv11 (Model)

```bash
cd yolo
pip install -r requirements.txt
# Chạy thử nghiệm
python detect/model.py --input test.jpg --type washer
```
### 3. Cài đặt giao diện người dùng (Next.js)

```bash
cd web/frontend
npm install
npm run dev
# Ứng dụng frontend sẽ chạy tại http://localhost:3001
```
### 3.  Cài đặt backend API (NestJS)

```bash
cd web/backend
npm install
npm run start:dev
# API sẽ chạy tại http://localhost:3000
```
## Demo



