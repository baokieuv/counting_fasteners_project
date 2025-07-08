import { Injectable } from '@nestjs/common';
import { exec } from 'child_process';
import * as fs from 'fs';
import { url } from 'inspector';
import { execArgv } from 'process';
import * as util from 'util';
import * as path from 'path'

const execPromise = util.promisify(exec);

@Injectable()
export class AppService {
  
  async runDetection(filePath: string, type: string): Promise<{
      ndet: number;
      image: string;
      labelUrl: string;
    }>{
    // const absPath =  `D:\\code\\projectTest\\web\\test-express\\${filePath.replace(/\//g, '\\')}`;
    console.log(filePath);
    const cmd = `python model/model.py --input "${filePath}" --type ${type}`;
    try{
      const  { stdout } = await execPromise(cmd);
      
      fs.unlink(filePath, err => {
        if (err) console.error(`Failed to delete temp file: ${err}`);
      });

      console.log(filePath);

      const resultLines = stdout.trim().split('\n');
  
      const labelLine = resultLines.find(line => line.startsWith('Label: '));
      const urlLine = resultLines.find(line => line.startsWith('Save: '));
      const numLine = resultLines.find(line => line.startsWith('Num: '));
  
      const labelUrl = labelLine ? labelLine.replace("Label: ", "").replace(/\\/g, "/") : null;
      const imageUrl = urlLine ? urlLine.replace("Save: ", "").replace(/\\/g, "/") : null;
      const ndet = numLine ? parseInt(numLine.replace("Num: ", "")) : null;
      // console.log(`Detect: ${imageUrl}, Num: ${ndet}`)

      const out_path = imageUrl ? path.join(path.resolve(__dirname, '..'), imageUrl?.trim()) : '';

      console.log(out_path);

      if(!out_path || !fs.existsSync(out_path)){
        throw new Error("Ảnh không tồn tại");
      }


      const buffer = fs.readFileSync(out_path);
      const base64 = buffer.toString('base64');
      const ext = path.extname(out_path).toLowerCase();

      const mime = ext === '.png' ? 'image/png' : 'image/jpeg';

      return {
        ndet,
        image: `data:${mime};base64,${base64}`,
        labelUrl,
      };
      
    } catch(error){
      console.error("Lỗi xử lý: ", error);
      return {message: 'Lỗi xử lý ảnh', error};
    }
  }
}
