import os
import socket, json, threading
import model

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
washer_model_path = os.path.join(BASE_DIR, "best.onnx")
washer_model: model.Yolov11_Onnx = model.Yolov11_Onnx(washer_model_path, label_list=["Washer"])


def handle_client(c: socket.socket, addr):
    print(f"Handling connection from {addr}")
    
    result_path = None
    label_path = None
    count = 0
    err = None
    
    try:
        with c:
            data = c.recv(4096).decode()
            if not data:
                raise ValueError("No data received")
            
            request = json.loads(data)
            image_path = request["imagePath"]
            object_type = request["type"]
            
            if not image_path or not object_type:
                raise ValueError("Missing 'imagePath' or 'type' in request")
            
            if object_type == "washer":
                result_path, label_path, count = model.detect_and_save(washer_model, image_path, object_type)
            else:
                err = "No model available for specified type"
                
            response = {
                "imagePath": result_path,
                "labelPath": label_path,
                "ndet": count,
                "error": err
            }
            print(f"Processing from {addr} done")
            c.sendall(json.dumps(response).encode())
    except Exception as e:
        error_response = {"error": str(e)}
        try:
            c.sendall(json.dumps(error_response).encode())
        except Exception:
            pass
        print(f"Error handling client {addr}: {e}")

def main():
    
    HOST = "127.0.0.1"
    PORT = 8888
    
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((HOST, PORT))
        
        s.listen(1)
        print("Server listening on port", PORT)
        
        try:
            while True:
                c, addr = s.accept()
                print("Connect from ", str(addr))
                threading.Thread(target=handle_client, args=(c, addr), daemon=True).start()
        except KeyboardInterrupt:
            print("Server shutting down....")
        except Exception as e:
            print(f"Server error: {e}")
    
if __name__ == "__main__":
    main()