#define HW14_BONUS_1
#ifndef _MTAT_H_
#define _MTAT_H_
#include "bmp.h"

// Do not modify the declaration of binarize.
BMPImage* binarize(BMPImage* image, int radius, int num_threads, char** a_error);

// OK to add your own declarations BELOW here
typedef struct
{
	unsigned int start;
	unsigned int end;
	int radius;
	BMPImage* orig_data;
	BMPImage* new_data;
} ImgPartition;

void* worker_binarize(void* arg);

char* _strdup(const char* src);

BMPImage* median(BMPImage* image, int radius, int num_threads, char** a_error);


#endif /* mtat.h */
