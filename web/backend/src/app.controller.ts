import { Controller, Get, Post, UploadedFile, UseInterceptors, Body, Res } from '@nestjs/common';
import { Response } from 'express';
import { AppService } from './app.service';
import { FileInterceptor } from '@nestjs/platform-express';
import { diskStorage } from 'multer';
import { exec } from 'child_process';
import { promisify } from 'util';
import { extname } from 'path';
import * as fs from 'fs';
import * as path from 'path';

@Controller()
export class AppController {
  constructor(private readonly appService: AppService) {}

  @Get()
  getHello(): string {
    return this.appService.getHello();
  }

  @Post('/detect')
  @UseInterceptors(
    FileInterceptor('image', {
      storage: diskStorage({
        destination: './uploads/results',
        filename: (req, file, callback) =>{
          const uniqueSuffix = Date.now() + '-' + Math.round(Math.random() * 1e9);
          const ext = extname(file.originalname);
          callback(null, `${file.fieldname}-${uniqueSuffix}${ext}`);
        },
      }),
    }),
  )
  async handleUpload(
    @UploadedFile() file: Express.Multer.File,
    @Body('type') type: string,
  ){
    if (!file || !type) {
      return { success: false, error: 'Missing image or type' };
    }
    console.log(`handle request! ${type}`);
    return this.appService.runDetection(file.path, type);
  }

  @Post('/esp32/detect')
  @UseInterceptors(
    FileInterceptor('image', {
      storage: diskStorage({
        destination: './uploads/results',
        filename: (req, file, callback) =>{
          const uniqueSuffix = Date.now() + '-' + Math.round(Math.random() * 1e9);
          const ext = extname(file.originalname);
          callback(null, `${file.fieldname}-${uniqueSuffix}${ext}`);
        },
      }),
    }),
  )
  async handleUploadESP32(
    @UploadedFile() file: Express.Multer.File,
    @Body('type') type: string,
    @Res() res: Response
  ){
    if (!file || !type) {
      return { success: false, error: 'Missing image or type' };
    }
    console.log(`handle request! ${type}`);

    const result = await this.appService.runDetection(file.path, type);
    const labelPath = result?.labelUrl?.trim();;
    console.log(labelPath)

    const out_path = labelPath ? path.join(__dirname, labelPath) : '';

    console.log(out_path);

    if(!labelPath){
      return res.status(404).json({ success: false, error: 'Output file not found'});
    }
    
    const content = fs.readFileSync(labelPath);
    res.setHeader('Content-Type', 'text/plain');
    res.setHeader('Content-Disposition', `inline; filename="${path.basename(labelPath)}"`);
    return res.send(content);
  }

}
