#include "stdio.h"
#include "image_filtering_new.h"
#include "bmpfuncs.h"

//image-1(240x135).bmp   240x135
//image-1(320x180).bmp   320x180
//image-1(800x450).bmp   800x450
//image-1(1024x576).bmp   1024x576
//image-1(1920x1080).bmp   1920x1080

//#define __240x135__
//#define __1024x576__
#define __1920x1080__

#ifdef __240x135__
#define INPUT_FILE_NAME   "../../../image-1(240x135).bmp"
#define OUTPUT_FILE_NAME  "../../../outputImage-1(240x135).bmp"
#define IMAGE_WIDTH   240
#define IMAGE_HEIGHT  135
#endif

#ifdef __1024x576__
#define INPUT_FILE_NAME   "../../../image-1(1024x576).bmp"
#define OUTPUT_FILE_NAME  "../../../outputImage-1(1024x576).bmp"
#define IMAGE_WIDTH   1024
#define IMAGE_HEIGHT  576
#endif


#ifdef __1920x1080__
#define INPUT_FILE_NAME   "../../../image-1(1920x1080).bmp"
#define OUTPUT_FILE_NAME  "../../../outputImage-1(1920x1080).bmp"
#define IMAGE_WIDTH   1920
#define IMAGE_HEIGHT  1080
#endif


int main() {


	int imageWidth = IMAGE_WIDTH;
	int imageHeight = IMAGE_HEIGHT;
	const char *inputFileName = INPUT_FILE_NAME;
	const char *outputFileName = OUTPUT_FILE_NAME;


	float* image = (float *)malloc(sizeof(float)*IMAGE_HEIGHT*IMAGE_WIDTH);
	float* outImage = (float *)malloc(sizeof(float)*IMAGE_HEIGHT*IMAGE_WIDTH);

	readImage(inputFileName, &imageWidth, &imageHeight, image);


	float* image_padding = (float *)malloc(2*sizeof(float)*(IMAGE_HEIGHT+MASK_HEIGHT)*(IMAGE_WIDTH+MASK_WIDTH));
	float* inputImage  = image_padding;
	float* outputImage = image_padding + (IMAGE_HEIGHT+MASK_HEIGHT)*(IMAGE_WIDTH+MASK_WIDTH);

	for (int i = 0; i < IMAGE_HEIGHT+MASK_HEIGHT; i++)
		for (int j = 0; j < IMAGE_WIDTH+MASK_WIDTH; j++) {
			int row = i-MASK_HEIGHT/2;
			int col = j-MASK_WIDTH/2;

			if (row < 0) row = 0;	if (row > IMAGE_HEIGHT-1) row = IMAGE_HEIGHT-1;
			if (col < 0) col = 0;   if (col > IMAGE_WIDTH-1) col = IMAGE_WIDTH-1;
			image_padding[i*(IMAGE_WIDTH+MASK_WIDTH)+j] = image[row*IMAGE_WIDTH+col];
		}






	//image tiling

	unsigned int inputOffset = 0;
	unsigned int outputOffset = 0;
	for (int row = 0; row < IMAGE_HEIGHT+2*(MASK_HEIGHT/2); row++) {

		image_filtering_new(inputImage, (u32)inputOffset, (u32)(IMAGE_HEIGHT+MASK_HEIGHT)*(IMAGE_WIDTH+MASK_WIDTH)+outputOffset,IMAGE_WIDTH+MASK_WIDTH, IMAGE_HEIGHT+MASK_HEIGHT);

		inputOffset  +=IMAGE_WIDTH+MASK_WIDTH;
		outputOffset +=IMAGE_WIDTH+MASK_WIDTH;
	}



	for (int i = 0; i < IMAGE_HEIGHT; i++)
		for (int j = 0; j < IMAGE_WIDTH; j++) {
			outImage[i*(IMAGE_WIDTH)+j] = outputImage[(i+MASK_HEIGHT/2)*(IMAGE_WIDTH+MASK_WIDTH)+(j+MASK_WIDTH/2)];
		}

	storeImage(outImage, outputFileName, IMAGE_HEIGHT, IMAGE_WIDTH, inputFileName);

	return 0;

}
