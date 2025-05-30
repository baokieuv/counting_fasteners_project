import { Controller, Get, Post, UploadedFile, UseInterceptors, Body, Res, HttpStatus } from '@nestjs/common';
import { Response } from 'express';
import { AppService } from './app.service';
import { FileInterceptor } from '@nestjs/platform-express';
import { diskStorage } from 'multer';
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
    @Res() res: Response,
  ){
    if (!file || !type) {
      return res.status(HttpStatus.BAD_REQUEST).json({
        success: false,
        error: "Missing image or type",
      });
    }

    try{
      const imagePath = path.join(path.resolve(__dirname, '..'), file.path);
      const result = await this.appService.runDetection(imagePath, type);
      return res.status(HttpStatus.OK).json({
        success: true,
        ...(typeof result === 'object' && result !== null ? result : { result }),
      })
    }catch(error){
      console.error('Detection failed:', error);

      fs.unlink(file.path, () => {});
      
      return res.status(HttpStatus.INTERNAL_SERVER_ERROR).json({
        success: false,
        error: error instanceof Error ? error.message : error,
      });
    }
  }
}
