# 🌐 Web Module – Counting fasteners project with YOLOV11

Phần **Web UI** cho hệ thống đếm phụ kiện công nghiệp sử dụng YOLOv11. Gồm 2 phần chính:

- 🖼️ **Frontend** – Xây dựng với **Next.js**: Giao diện người dùng (client-side).
- 🛠️ **Backend API** – Xây dựng với **NestJS**: Xử lý logic và gọi model phía server.

---

## 📌 Mục tiêu

- Cung cấp giao diện để người dùng tải lên ảnh
- Gửi ảnh từ frontend tới API `/api/detect` để xử lý.
- Từ route `/api/detect` gọi fetch tới phía backend để gửi form gồm ảnh và type
- Gọi NestJS backend để chạy mô hình và nhận kết quả.
- Trả lại kết quả phát hiện và hiển thị lên giao diện.

---

## 📁 Cấu trúc thư mục

```
web/
├── frontend/ # Giao diện người dùng - Next.js
└── backend/ # Xử lý API và gọi mô hình - NestJS
```

---

## 🚀 Luồng hoạt động

```plaintext
Client (Next.js)
  ↓ (submit image)
API Route (Next.js - /api/detect)
  ↓ (fetch)
NestJS Backend (http://localhost:3001)
  ↓ (xử lý ảnh và chạy model bằng cách excute cmd)
Kết quả trả về → Next.js API → Giao diện (hiển thị kết quả)
```
---

## Cài đặt và chạy

### 1. Chạy Frontend (Next.js)

```
cd web/frontend
npm install
npm run dev
# Ứng dụng chạy tại http://localhost:3000
```

### 2. Chạy Backend (NestJS)

```
cd web/backend
npm install
npm run start:dev
# API backend chạy tại http://localhost:3001
```

## Chi tiết API

POST /api/detect – Frontend API Route
- Nhận ảnh từ người dùng (form-data hoặc base64).
- Gửi ảnh đến backend NestJS (http://localhost:3001/detect) qua fetch.
- Nhận kết quả và trả về client để hiển thị.

POST /detect – Backend NestJS
- Nhận ảnh từ frontend.
- Gọi tới module YOLOv11 bằng cmd.
- Lưu kết quả nhận diện tại thư mục /uploads
- Trả về kết quả dưới dạng JSON.
- Gửi lại kết quả cho phía frontend để hiển thị

## Demo
![Screenshot 2025-05-28 224437](https://github.com/user-attachments/assets/d2b1d180-9f9d-45b0-b772-1c2a9cce66ec)
