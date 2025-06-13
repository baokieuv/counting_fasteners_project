'use client'
import Image from "next/image";
import { useState } from "react";

export default function Home() {

  const [image, setImage] = useState<File | null>(null);
  const [imageUrl, setImageUrl] = useState<string | null>(null);
  const [ob_type, setType] = useState('washer');
  const [ndet, setNdet] = useState<number | null>(null);
  const [error, setError] = useState<string | null>(null);

  const handleSubmit = async (e : React.FormEvent) => {
    e.preventDefault();
    if (!image){
      setError('Chưa có ảnh');
      return;
    }

    const formData = new FormData();
    formData.append('image', image);
    formData.append('type', ob_type);

    try{
      const res = await fetch('/api/detect', {
        method: 'POST',
        body: formData,
      });

      const res_data = await res.json();

      if(!res.ok) throw new Error(res_data.message || 'Upload failed!');
      
      console.log("done fetch");

      setImageUrl(res_data.image); 
      setNdet(res_data.ndet);
      setError(null);
    }catch (err: any){
      setError(err.message);
    }
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-blue-100 to-blue-200 flex flex-col items-center justify-between px-4 py-6">
      <header className="text-2xl font-bold text-blue-800 mb-4 text-center">
        DEMO ĐẾM ĐỒ VẬT BẰNG YOLOV11
      </header>

      <main className="w-full max-w-xl bg-white p-6 rounded-2xl shadow-xl">
        {error && <p className="text-red-600 font-semibold">{error}</p>}
        <form onSubmit={handleSubmit} encType="multipart/form-data" className="space-y-4">
          <input
            type="file"
            accept="image/*"
            required
            onChange={(e) => setImage(e.target.files?.[0] || null)}
            className="w-full px-4 py-2 border rounded-lg cursor-pointer"
            style={{color: 'gray'}}
          />

          <select
            value={ob_type}
            onChange={(e) => setType(e.target.value)}
            className="w-full px-4 py-2 border rounded-lg"
            style={{color:'black'}}
          >
            <option value="washer">Washer</option>
            <option value="screw">Screw</option>
            <option value="nut">Nut</option>
          </select>

          <button
            id="myButton"
            type="submit"
            className="w-full bg-blue-600 text-white py-2 rounded-lg hover:bg-blue-700 transition"
            style={{cursor: 'pointer'}}
          >
            Submit
          </button>

          <div className="text-sm text-gray-500 text-center"><i>Các định dạng được hỗ trợ: PNG, JPG và JPEG.</i></div>
          <div className="text-sm text-gray-500 text-center"><i>Hãy để ảnh kích thước 640x640 để được kết quả tốt nhất.</i></div>
        </form>

        {imageUrl && (
          <div className="mt-8">
            <h2 className="text-lg font-bold text-gray-700 mb-2">Kết quả nhận diện</h2>
            <p className="text-gray-800 mb-4">
              Nhận diện được <span className="font-bold text-blue-600">{ndet} vật thể</span>
            </p>
            <img
              src={imageUrl}
              alt="Kết quả"
              className="w-full max-w-md rounded-xl shadow-md mx-auto"
            />
          </div>
        )}
      </main>

      <footer className="text-sm text-center text-gray-600 mt-10">
        <p>&copy; KvB - 20225261. All rights reserved.</p>
        <p>Liên hệ: <a href="mailto:example@email.com" className="text-blue-600">example@email.com</a></p>
        <div className="flex gap-3 justify-center mt-2 text-blue-600">
          <a href="https://www.facebook.com" target="_blank"><i className="fab fa-facebook"></i> Facebook</a>
          <a href="https://twitter.com" target="_blank"><i className="fab fa-twitter"></i> Twitter</a>
          <a href="https://www.instagram.com" target="_blank"><i className="fab fa-instagram"></i> Instagram</a>
        </div>
      </footer>
    </div>
  );
}
