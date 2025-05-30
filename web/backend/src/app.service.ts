import { Injectable } from '@nestjs/common';
import * as fs from 'fs';
import * as net from 'net'
import * as path from 'path'

@Injectable()
export class AppService {
  getHello(): string {
    return 'Hello World!';
  }

  async runDetection(filePath: string, type: string){
    // const absPath =  `D:\\code\\projectTest\\web\\test-express\\${filePath.replace(/\//g, '\\')}`;
    return new Promise((resolve, reject) => {
      const client = new net.Socket();

      client.connect(8888, "127.0.0.1", () => {
        const payload = JSON.stringify({imagePath: filePath, type});
        client.write(payload);
      });

      client.on('data', (data) => {
        try{
          const result = JSON.parse(data.toString());
          if(result.error){
            reject(result.error);
            return;
          }

          fs.unlink(filePath, err => {
            if (err) console.error(`Failed to delete temp file: ${err}`);
          });

          const imagePath = path.join(path.resolve(__dirname, '..'), result.imagePath);
          const buffer = fs.readFileSync(imagePath);
          const base64 = buffer.toString('base64');
          const ext = path.extname(imagePath).toLowerCase();
          const mime = ext === '.png' ? 'image/png' : 'image/jpeg';

          resolve({
            ndet: result.ndet,
            image: `data:${mime};base64,${base64}`,
            labelUrl: result.labelPath,
          });

        }catch(err){
          reject(err);
        }finally {
          client.destroy();
        }
      });

      client.on('error', (err) => {
        reject(`Socket error: ${err}`);
      });
    });
  }
}
