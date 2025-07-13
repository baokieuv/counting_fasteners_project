import { Controller, Get, Post, UploadedFile, UseInterceptors, Body, Res, HttpStatus } from '@nestjs/common';
import { Response } from 'express';
import { AppService } from './app.service';
import { FileInterceptor } from '@nestjs/platform-express';
import { diskStorage } from 'multer';
import * as path from 'path';
import * as fs from 'fs';


const storage = diskStorage({
  destination: './uploads/results',
  filename: (req, file, cb) => {
    const uniqueSuffix = Date.now() + '-' + Math.round(Math.random() * 1e9);
    const ext = path.extname(file.originalname);
    cb(null, `${file.fieldname}-${uniqueSuffix}${ext}`);
  },
});

@Controller()
export class AppController {
  constructor(private readonly appService: AppService) {}

  @Post('/detect')
  @UseInterceptors(FileInterceptor('image', { storage }))
  async handleUpload(
    @UploadedFile() file: Express.Multer.File,
    @Body('type') type: string,
    @Res() res: Response
  ){
    return this.handleUploadCommon(file, type, res, false);
  }

  @Post('/esp32/detect')
  @UseInterceptors(FileInterceptor('image', { storage }))
  async handleUploadESP32(
    @UploadedFile() file: Express.Multer.File,
    @Body('type') type: string,
    @Res() res: Response
  ){
    return this.handleUploadCommon(file, type, res, true);
  }

  private async handleUploadCommon(
    file: Express.Multer.File,
    type: string,
    res: Response,
    isESP32: boolean
  ){
    if(!file || !type){
      return res.status(HttpStatus.BAD_REQUEST).json({
        success: false,
        error: 'Missing image or type',
      });
    }

    try{
      const imagePath = path.join(path.resolve(__dirname, '..'), file.path);
      const result = await this.appService.runDetection(imagePath, type);

      const labelPath = result?.labelUrl?.trim();
      if (!labelPath || !fs.existsSync(labelPath)) {
        return res.status(HttpStatus.NOT_FOUND).json({
          success: false,
          error: 'Label file not found',
        });
      }

      if (isESP32) {
        const content = fs.readFileSync(labelPath, 'utf-8');
        res.setHeader('Content-Type', 'text/plain');
        res.setHeader('Content-Disposition', `inline; filename="${path.basename(labelPath)}"`);
        return res.status(HttpStatus.OK).send(content);
      } else {
        return res.status(HttpStatus.OK).json({
          success: true,
          ...result,
        });
      }

    }catch(error){
      console.error('Detection error:', error);
      fs.unlink(file.path, () => {});
      return res.status(HttpStatus.INTERNAL_SERVER_ERROR).json({
        success: false,
        error: error instanceof Error ? error.message : error,
      });
    }
  }
}
