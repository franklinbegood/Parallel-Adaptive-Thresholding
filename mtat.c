#include "bmp.h"
#include "mtat.h"
#include <math.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

char* _strdup(const char* src) {
	return strcpy(malloc((strlen(src) + 1) * sizeof(*src)), src);
}

void* worker_binarize(void* arg) {
	ImgPartition* inp = (ImgPartition*)arg;
	int bytesPP = inp->orig_data->header.bits_per_pixel / 8;
	unsigned int cnt = inp->start;
	int h = inp->orig_data->header.height_px;
	int w = inp->orig_data->header.width_px;
	int r = inp->radius;
	int padding = 4 - (inp->orig_data->header.width_px * inp->orig_data->header.bits_per_pixel / 8) % 4;
	int tmp = 0;
	int ext = 0;				// Neighborhood pixels
	int cor = 0;				// Center pixel
	int cmp_cnt = 0;
	unsigned char* data = inp->orig_data->data;
	unsigned char* newData = inp->new_data->data;
	//..
	if(padding == 4) {
		padding = 0;			// Adjust padding
	}

	while(cnt <= inp->end) {
		ext = 0;
		cmp_cnt = 0;
		for(int y = -r; y <= r; y++) {
			for(int x = -r; x <= r; x++) {
				if(y + (cnt / w) < h && y + (cnt / w) >= 0) {				// Check for out of bounds in y
					if(x + (cnt % w) < w && x + (cnt % w) >= 0) {			// Check for out of bounds in x
						tmp = 0;
						for(int i = 0; i < bytesPP; i++){
							tmp += data[i + (cnt + y * w + x) * bytesPP + padding * ((cnt + y * w + x) / w)];	// Read byte data into a temp variable
						}
						ext += tmp;
						cmp_cnt++;
					}
				}
			}
		}
		cor = 0;
		for(int i = 0; i < bytesPP; i++) {
			cor += data[i + cnt * bytesPP + padding * (cnt / w)];
		}
		
		if(cor * cmp_cnt <= ext) {
			for(int i = 0; i < bytesPP; i++) {
				newData[i + cnt * bytesPP + padding * (cnt / w)] = 0;
			}
		} else {
			for(int i = 0; i < bytesPP; i++) {
				newData[i + cnt * bytesPP + padding * (cnt / w)] = 255;
			}
		}

		if((cnt + 1) % w == 0) {											
			for(int i = 0; i < padding; i++) {								
				newData[i + (cnt + 1) * bytesPP + padding * (cnt / w)] = 0; 
			}
		}
		cnt++;
	}
	return NULL;
}

BMPImage* binarize(BMPImage* image, int radius, int num_threads, char** error) {
	int imgSize = image->header.image_size_bytes;
	int bytesPP = image->header.bits_per_pixel / 8;
	int paddingTotal = (4 - (image->header.width_px * image->header.bits_per_pixel / 8) % 4);

	if (paddingTotal == 4) {
		paddingTotal = 0;
	} else {
		paddingTotal *= image->header.height_px;
	}

	// All errors 2XX relate to binarize
	if(radius < 0) {
		if(*error == NULL) {
			*error = _strdup("Error 201: Radius can not be negative.");
		}
		return NULL;
	}

	if(image == NULL) {
		if(*error == NULL) {
			*error = _strdup("Error 202: Image can not be NULL.");
		}
		return NULL;
	}
	
	// Allocate memory for output image
	BMPImage* newImg = malloc(sizeof(*newImg));
	newImg->header = image->header;
	newImg->data = malloc(sizeof(*(newImg->data)) *
					imgSize);

	if(newImg == NULL || newImg->data == NULL) {
		if(*error == NULL) {
			*error = _strdup("Error 203: Image memory allocation failed.");
		}
		return NULL;
	}	

	// Partition image
	ImgPartition* imgPartition = malloc(sizeof(*imgPartition) * num_threads);
	
	for(int p = 0; p < num_threads; p++) {
		(imgPartition[p]).start = p * (imgSize - paddingTotal) / bytesPP / num_threads;
		(imgPartition[p]).end = ((p + 1) * (imgSize - paddingTotal) / bytesPP / num_threads) - 1;	
		(imgPartition[p]).radius = radius;
		(imgPartition[p]).orig_data = image;
		(imgPartition[p]).new_data = newImg;
	}
	
	// Begin threading
	pthread_t* thread = malloc(sizeof(*thread) * num_threads);

	if(thread == NULL) {
		if(*error == NULL) {
			*error = _strdup("Error 204: Thread memory allocation failed.");
		}
		return NULL;
	}

	for(int i = 0; i < num_threads; i++) {
		pthread_create(&(thread[i]), NULL, worker_binarize, 
		&(imgPartition[i]));
	}

	for(int o = 0; o < num_threads; o++) {
		pthread_join(thread[o], NULL);
	}

	free(thread);
	free(imgPartition);
	
	return newImg;
}

int int_cmp(const void *p1, const void *p2) {
	return (*((const int *)p1)) - (*((const int *)p2));
}

void* worker_median(void* arg) {
	ImgPartition* inp = (ImgPartition*)arg;
	int bytesPP = inp->orig_data->header.bits_per_pixel / 8;
	int cnt = inp->start;
	int h = inp->orig_data->header.height_px;
	int w = inp->orig_data->header.width_px;
	int r = inp->radius;
	int padding = 4 - (inp->orig_data->header.width_px * inp->orig_data->header.bits_per_pixel / 8) % 4;
	int tmp = 0;
	int cor = 0;				// Center pixel
	int med = 0;				// median
	int cmp_cnt = 0;
	int cmp_idx = 0;
	unsigned char* data = inp->orig_data->data;
	unsigned char* newData = inp->new_data->data;
	int* filt = NULL; 
	//..
	if(padding == 4) {
		padding = 0;			// Adjust padding
	}
	while(cnt <= inp->end) {
		cmp_cnt = 0;
		for(int y = -r; y <= r; y++) {
			for(int x = -r; x <= r; x++) {
				if(y + (cnt / w) < h && y + (cnt / w) >= 0) {				// Check for out of bounds in y
					if(x + (cnt % w) < w && x + (cnt % w) >= 0) {			// Check for out of bounds in x
						cmp_cnt++;
					}
				}
			}
		}
		filt = malloc(sizeof(*filt) * cmp_cnt);
		//Copy loop here
		cmp_idx = 0;
		for(int y = -r; y <= r; y++) {
			for(int x = -r; x <= r; x++) {
				if(y + (cnt / w) < h && y + (cnt / w) >= 0) {				// Check for out of bounds in y
					if(x + (cnt % w) < w && x + (cnt % w) >= 0) {			// Check for out of bounds in x
						tmp = 0;
						for(int i = 0; i < bytesPP; i++){
							tmp += data[i + (cnt + y * w + x) * bytesPP + padding * ((cnt + y * w + x) / w)];	// Read byte data into a temp variable
						}
						filt[cmp_idx++] = tmp;
					}
				}
			}
		}

		qsort(filt, cmp_cnt, sizeof(*filt), int_cmp);

		cor = 0;
		for(int i = 0; i < bytesPP; i++) {
			cor += data[i + cnt * bytesPP + padding * (cnt / w)];
		}
		med = 0;
		if (cmp_cnt % 2 == 1) {
			med = filt[cmp_cnt / 2] * 2;
		} else {
			med += filt[cmp_cnt / 2];
			med += filt[cmp_cnt / 2 - 1];
		}
		
		if(cor * 2 <= med) {
			for(int i = 0; i < bytesPP; i++) {
				newData[i + cnt * bytesPP + padding * (cnt / w)] = 0;
			}
		} else {
			for(int i = 0; i < bytesPP; i++) {
				newData[i + cnt * bytesPP + padding * (cnt / w)] = 255;
			}
		}

		if((cnt + 1) % w == 0) {
			for(int i = 0; i < padding; i++) {
				newData[i + (cnt + 1) * bytesPP + padding * (cnt / w)] = 0;
			}
		}
		free(filt);
		cnt++;
	}
	return NULL;
}


BMPImage* median(BMPImage* image, int radius, int num_threads, char** error) {
	int imgSize = image->header.image_size_bytes;
	int bytesPP = image->header.bits_per_pixel / 8;
	int paddingTotal = (4 - (image->header.width_px * image->header.bits_per_pixel / 8) % 4);

	if (paddingTotal == 4) {
		paddingTotal = 0;
	} else {
		paddingTotal *= image->header.height_px;
	}

	// All errors 4XX relate to median
	if(radius < 0) {
		if(*error == NULL) {
			*error = _strdup("Error 401: Radius can not be negative.");
		}
		return NULL;
	}

	if(image == NULL) {
		if(*error == NULL) {
			*error = _strdup("Error 402: Image can not be NULL.");
		}
		return NULL;
	}
	
	// Allocate memory for output image
	BMPImage* newImg = malloc(sizeof(*newImg));
	newImg->header = image->header;
	newImg->data = malloc(sizeof(*(newImg->data)) *
					imgSize);

	if(newImg == NULL || newImg->data == NULL) {
		if(*error == NULL) {
			*error = _strdup("Error 403: Image memory allocation failed.");
		}
		return NULL;
	}	

	// Partition image
	ImgPartition* imgPartition = malloc(sizeof(*imgPartition) * num_threads);
	
	for(int p = 0; p < num_threads; p++) {
		(imgPartition[p]).start = p * (imgSize - paddingTotal) / bytesPP / num_threads;
		(imgPartition[p]).end = ((p + 1) * (imgSize - paddingTotal) / bytesPP / num_threads) - 1;	
		(imgPartition[p]).radius = radius;
		(imgPartition[p]).orig_data = image;
		(imgPartition[p]).new_data = newImg;
	}
	
	// Begin threading
	pthread_t* thread = malloc(sizeof(*thread) * num_threads);

	if(thread == NULL) {
		if(*error == NULL) {
			*error = _strdup("Error 404: Thread memory allocation failed.");
		}
		return NULL;
	}

	for(int i = 0; i < num_threads; i++) {
		pthread_create(&(thread[i]), NULL, worker_median, 
		&(imgPartition[i]));
	}

	for(int o = 0; o < num_threads; o++) {
		pthread_join(thread[o], NULL);
	}

	free(thread);
	free(imgPartition);
	
	return newImg;


}















