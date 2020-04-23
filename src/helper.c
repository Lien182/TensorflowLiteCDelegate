#include "helper.h"
#include "math.h"
 
//Taken from
//https://elcharolin.wordpress.com/2018/11/28/read-and-write-bmp-files-in-c-c/ 
void ReadImage(const char *fileName,uint8_t **pixels, int32 *width, int32 *height, int32 *bytesPerPixel)
{
    FILE *imageFile = fopen(fileName, "rb");
    int32 dataOffset;
    fseek(imageFile, DATA_OFFSET_OFFSET, SEEK_SET);
    fread(&dataOffset, 4, 1, imageFile);
    fseek(imageFile, WIDTH_OFFSET, SEEK_SET);
    fread(width, 4, 1, imageFile);
    fseek(imageFile, HEIGHT_OFFSET, SEEK_SET);
    fread(height, 4, 1, imageFile);
    int16 bitsPerPixel;
    fseek(imageFile, BITS_PER_PIXEL_OFFSET, SEEK_SET);
    fread(&bitsPerPixel, 2, 1, imageFile);
    *bytesPerPixel = ((int32)bitsPerPixel) / 8;

    int paddedRowSize = (int)(4 * ceil((float)(*width) / 4.0f))*(*bytesPerPixel);
    int unpaddedRowSize = (*width)*(*bytesPerPixel);
    int totalSize = unpaddedRowSize*(*height);
    *pixels = (byte*)malloc(totalSize);
    int i = 0;
    byte *currentRowPointer = *pixels+((*height-1)*unpaddedRowSize);
    for (i = 0; i < *height; i++)
    {
        fseek(imageFile, dataOffset+(i*paddedRowSize), SEEK_SET);
        fread(currentRowPointer, 1, unpaddedRowSize, imageFile);
        currentRowPointer -= unpaddedRowSize;
    }

    fclose(imageFile);
}

//Modified taken from
//https://elcharolin.wordpress.com/2018/11/28/read-and-write-bmp-files-in-c-c/
void ReadImageAlign(const char *fileName,uint8_t **pixels, int32 *width, int32 *height, int32 *bytesPerPixel)
{
    FILE *imageFile = fopen(fileName, "rb");
    int32 dataOffset;
    fseek(imageFile, DATA_OFFSET_OFFSET, SEEK_SET);
    fread(&dataOffset, 4, 1, imageFile);
    fseek(imageFile, WIDTH_OFFSET, SEEK_SET);
    fread(width, 4, 1, imageFile);
    fseek(imageFile, HEIGHT_OFFSET, SEEK_SET);
    fread(height, 4, 1, imageFile);
    int16 bitsPerPixel;
    fseek(imageFile, BITS_PER_PIXEL_OFFSET, SEEK_SET);
    fread(&bitsPerPixel, 2, 1, imageFile);
    *bytesPerPixel = ((int32)bitsPerPixel) / 8;

    *pixels = (byte*)malloc(sizeof(uint32_t) * *width * *height);
    printf("Read Image: width=%d, height=%d \n", *width, *height);

    uint8_t raw[4];
    uint32_t src_pos, dest_pos;



    for (int i = 0; i < *height; i++)
    {
        for(int j = 0; j < *width; j++)
        {
            src_pos = (i * *width + j) *  *bytesPerPixel;
            dest_pos = (i * *width + j) *  4;
            
            //printf("Read Image: src_pos=%d, dest_pos=%d \n", src_pos, dest_pos);

            fseek(imageFile, src_pos, SEEK_SET);
            fread((void*)raw, 1, *bytesPerPixel, imageFile);
            
            if(*bytesPerPixel == 3)
            {
                (*pixels)[dest_pos]     = raw[2];
                (*pixels)[dest_pos + 1] = raw[1];
                (*pixels)[dest_pos + 2] = raw[0];
            }
            else if(*bytesPerPixel == 4)
            {
                (*pixels)[dest_pos]     = raw[2];
                (*pixels)[dest_pos + 1] = raw[1];
                (*pixels)[dest_pos + 2] = raw[0];
                (*pixels)[dest_pos + 3] = raw[3];
            }
        }
    }
    fclose(imageFile);

}
 
//Taken from
//https://elcharolin.wordpress.com/2018/11/28/read-and-write-bmp-files-in-c-c/

void WriteImage(const char *fileName, uint8_t *pixels, int32 width, int32 height,int32 bytesPerPixel)
{
    FILE *outputFile = fopen(fileName, "wb");
    //*****HEADER************//
    const char *BM = "BM";
    fwrite(&BM[0], 1, 1, outputFile);
    fwrite(&BM[1], 1, 1, outputFile);
    int paddedRowSize = (int)(4 * ceil((float)width/4.0f))*bytesPerPixel;
    int32 fileSize = paddedRowSize*height + HEADER_SIZE + INFO_HEADER_SIZE;
    fwrite(&fileSize, 4, 1, outputFile);
    int32 reserved = 0x0000;
    fwrite(&reserved, 4, 1, outputFile);
    int32 dataOffset = HEADER_SIZE+INFO_HEADER_SIZE;
    fwrite(&dataOffset, 4, 1, outputFile);

    //*******INFO*HEADER******//
    int32 infoHeaderSize = INFO_HEADER_SIZE;
    fwrite(&infoHeaderSize, 4, 1, outputFile);
    fwrite(&width, 4, 1, outputFile);
    fwrite(&height, 4, 1, outputFile);
    int16 planes = 1; //always 1
    fwrite(&planes, 2, 1, outputFile);
    int16 bitsPerPixel = bytesPerPixel * 8;
    fwrite(&bitsPerPixel, 2, 1, outputFile);
    //write compression
    int32 compression = NO_COMPRESION;
    fwrite(&compression, 4, 1, outputFile);
    //write image size (in bytes)
    int32 imageSize = width*height*bytesPerPixel;
    fwrite(&imageSize, 4, 1, outputFile);
    int32 resolutionX = 11811; //300 dpi
    int32 resolutionY = 11811; //300 dpi
    fwrite(&resolutionX, 4, 1, outputFile);
    fwrite(&resolutionY, 4, 1, outputFile);
    int32 colorsUsed = MAX_NUMBER_OF_COLORS;
    fwrite(&colorsUsed, 4, 1, outputFile);
    int32 importantColors = ALL_COLORS_REQUIRED;
    fwrite(&importantColors, 4, 1, outputFile);
    int i = 0;
    int unpaddedRowSize = width*bytesPerPixel;
    for ( i = 0; i < height; i++)
    {
            int pixelOffset = ((height - i) - 1)*unpaddedRowSize;
            fwrite(&pixels[pixelOffset], 1, paddedRowSize, outputFile); 
    }
    fclose(outputFile);
}