#include "bmp.h"
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

// Returns NULL and reports general read error
BMPImage* _cleanF(FILE* fp, char** error) {
	if(*error == NULL) {
		char* message = "read_bmp(FILE* fp, char** error) failed to read. Image may be corrupt or file does not exist.";
		*error = malloc((strlen(message) + 1) * sizeof(**error));
		strcpy(*error, message);
	}
	return NULL;
}

BMPImage* read_bmp(FILE* fp, char** error) {
	BMPImage* img = NULL;

	if(fp == NULL || *error != NULL) { 
		return _cleanF(fp, error);
	}

	if(fseek(fp, BMP_HEADER_SIZE, SEEK_SET) < 0) {
		return _cleanF(fp, error);
	}
	if(ftell(fp) != BMP_HEADER_SIZE) {
		return _cleanF(fp, error);
	}
	rewind(fp);

	img = malloc(sizeof(*img));							// Allocate memory for img

	if(fread(&(img->header), sizeof((img->header)), 1, fp)) {
		if(check_bmp_header(&(img->header), fp)) {
			img->data = malloc(sizeof(*(img->data)) * 	// Allocate memory for img's data
			img->header.image_size_bytes);
			if(fread(img->data, sizeof(*(img->data)), 
			img->header.image_size_bytes, fp)) {

/*				char auxbyte;
				if(fread(&auxbyte, sizeof(auxbyte), 1, fp)) {		// Check to find any excess data unread, if they exist the read process has failed
					return _cleanF(fp, error);
				}
*/
				return img;
			} else {
				return _cleanF(fp, error);
			}
	
		} else {
			return _cleanF(fp, error);
		}
	} else {
		return _cleanF(fp, error);
	}
}

// Similar to cleanF, except for write errors, write errors are more prone to different issues so more dynamic error messages
void _write_error(char** error, char* msg) {
	if(*error == NULL) {
		char* message = msg; 
		*error = malloc((strlen(message) + 1) * sizeof(**error));
		strcpy(*error, message);
	}
}

bool write_bmp(FILE* fp, BMPImage* image, char** error) {
	if(fp == NULL || *error != NULL) {
		_write_error(error, "write_bmp(FILE* fp, BMPImage* image, char** error) failed because file could not be written.");
		return false;
	}
	if(image == NULL) {
		_write_error(error, "write_bmp(FILE* fp, BMPImage* image, char** error) failed because image does not exist.");
		return false;
	}
	if(fwrite(&(image->header), sizeof(image->header), 1, fp)) {
		if(fwrite(image->data, sizeof(*(image->data)), 
		image->header.image_size_bytes, fp) == 
		image->header.image_size_bytes) {
			return true;
		} else {
			_write_error(error, "write_bmp(FILE* fp, BMPImage* image, char** error) failed because corrupted image file.");
			return false;
		}
	} else {
		_write_error(error, "write_bmp(FILE* fp, BMPImage* image, char** error) failed because corrupted image file.");
		return false;
	}
}

bool check_bmp_header(BMPHeader* bmp_hdr, FILE* fp) {
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	fseek(fp, bmp_hdr->offset, SEEK_SET);			// Set file read location to offset
	int padding = 4 - (((bmp_hdr->width_px * bmp_hdr->bits_per_pixel) / 8) % 4);
	if(padding == 4) {
		padding = 0;
	}
	if(bmp_hdr == NULL) {
		return false;
	}
	if(bmp_hdr->type != 0x4d42) {
		return false;
	}
	
	if(bmp_hdr->offset != BMP_HEADER_SIZE) {
		return false;
	}

	if(bmp_hdr->dib_header_size != DIB_HEADER_SIZE) {
		return false;
	}

	if(bmp_hdr->num_planes != 1) {
		return false;
	}

	if(bmp_hdr->compression != 0) {
		return false;
	}

	if(bmp_hdr->num_colors != 0 || 
	bmp_hdr->important_colors != 0) {
		return false;
	}

	if(!(bmp_hdr->bits_per_pixel == 16 || 
	bmp_hdr->bits_per_pixel == 24)) {
		return false;
	}
	// Confirm that size of data and entire file is correct
	if(bmp_hdr->image_size_bytes != (bmp_hdr->width_px * bmp_hdr->height_px * bmp_hdr->bits_per_pixel / 8) + padding * bmp_hdr->height_px ||
	bmp_hdr->size != bmp_hdr->offset + bmp_hdr->image_size_bytes) {
		return false;
	}

	if(bmp_hdr->size != size) {
		return false;
	}

	return true;
}

BMPImage* crop_bmp(BMPImage* image, int x, int y, int w, int h, char** error) {
	// Error checking
	if(image == NULL || *error != NULL) {
		_write_error(error, "crop_bmp(.. ) failed because image does not exist.");
		return NULL;
	}
	if(x < 0 || x > image->header.width_px) {
		_write_error(error, "crop_bmp(.. ) failed because x out of bounds.");
		return NULL;
	}
	if(y < 0 || y > image->header.height_px) {
		_write_error(error, "crop_bmp(.. ) failed because y out of bounds.");
		return NULL;
	}
	if(w < 0 || x + w > image->header.width_px) {
		_write_error(error, "crop_bmp(.. ) failed because w out of bounds.");
		return NULL;
	}
	if(h < 0 || y + h > image->header.height_px) {
		_write_error(error, "crop_bmp(.. ) failed because h out of bounds.");
		return NULL;
	}

	// Creates temporary image to be allocated by user
	BMPImage* newImg = malloc(sizeof(*newImg));
	newImg->header = image->header;
	newImg->header.width_px = w;
	newImg->header.height_px = h;
	int padding = 4 - ((newImg->header.width_px * newImg->header.height_px / 8) % 4);
	if(padding == 4) {
		padding = 0;
	}
	newImg->header.image_size_bytes = w * h * newImg->header.bits_per_pixel / 8 + padding * newImg->header.height_px;
	newImg->header.size = newImg->header.image_size_bytes + newImg->header.offset;
	newImg->data = malloc(sizeof(*(newImg->data)) * newImg->header.image_size_bytes); 
	int auxC = 0;

	for(int j = y * newImg->header.bits_per_pixel / 8; j < (h + y) * newImg->header.bits_per_pixel / 8; j++) {
		for(int i = x; i < (w + x); i++) {
			newImg->data[auxC] = image->data[i + (image->header.width_px) * j];
			auxC++;
		}
		// Add padding
		for(int i = 0; i < padding; i++) {
			newImg->data[auxC++] = 0;
		}
	}
	
	return newImg;
}

void free_bmp(BMPImage* image) {
	if(image != NULL && image->data != NULL) {
		free(image->data);
	}
	if(image != NULL) {
		free(image);
	}
}

