#include "mtat.h"
#include "bmp.h"
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
	// Read file
	char* error = NULL;
	FILE* fp = fopen("img1_6x6_color.bmp", "r");
	BMPImage* img = read_bmp(fp, &error); 
	fclose(fp);
	
	// Write a copy of file
	fp = fopen("test_img_clean.bmp", "w");
	if(write_bmp(fp, img, &error)) {
		fclose(fp);
		printf("Successfully wrote test_img_clean.bmp.\n");
	} else {
		printf("%s", error);
	}

	BMPImage* binImg = binarize(img, 5, 3, &error);
	fp = fopen("test_img_bin.bmp", "w");
	if(write_bmp(fp, binImg, &error)) {
		fclose(fp);
		printf("Successfully wrote test_img_bin.bmp.\n");
	} else {
		printf("%s", error);
	}
	
	free_bmp(img);
	free_bmp(binImg);

	// Test actual page
	fp = fopen("img2_384x510_gray.bmp", "r");
	img = read_bmp(fp, &error);
	fclose(fp);
	
	binImg = binarize(img, 2, 1, &error);
	fp = fopen("aa_Bau.bmp", "w");
	if(write_bmp(fp, binImg, &error)) {
		fclose(fp);
		printf("Successfully wrote aa_Bau.bmp.\n");
	} else {
		printf("%s", error);
	}

	free_bmp(img);
	free_bmp(binImg);

	// Bonus
	fp = fopen("img2_384x510_gray.bmp", "r");
	img = read_bmp(fp, &error);
	fclose(fp);
	
	binImg = median(img, 2, 3, &error);
	fp = fopen("aa_Mau.bmp", "w");
	if(write_bmp(fp, binImg, &error)) {
		fclose(fp);
		printf("Successfully wrote aa_Mau.bmp.\n");
	} else {
		printf("%s", error);
	}

	free_bmp(img);
	free_bmp(binImg);
	

	return EXIT_SUCCESS;
}
