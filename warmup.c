#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

char* _strdup(const char* src) {
	return strcpy(malloc((strlen(src) + 1) * sizeof(*src)), src);
}

void* worker_zero_array(void* arg) {
	int *inp = (int*)arg;
	while(*inp != 0) {
		*inp = 0;
		inp++;
	}
	return NULL;
}

bool zero_array(int* array, int size, int num_threads, char** a_error) {
	if(size <= 0) {
		if(*a_error == NULL) {
			*a_error = _strdup("Error 01: Size must be greater than zero.\n");
		}
		return false;
	}

	if(array == NULL) {
		if(*a_error == NULL) {
			*a_error = _strdup("Error 02: Array can not be NULL.\n");
		}
		return false;
	}
	if(num_threads > size) {
		num_threads = size;
	}

	pthread_t* thread = malloc(sizeof(*thread) * num_threads);

	if(thread == NULL) {
		if(*a_error == NULL) {
			*a_error = _strdup("Error 03: Thread could not be created.\n");
		}
		return false;
	}
	array[size - 1] = 0;	// Set last element to zero to prevent corruption
	for(int i = 0; i < num_threads; i++) {
		pthread_create(&(thread[i]), NULL, worker_zero_array, &(array[i * size / num_threads]));
	}

	for(int i = 0; i < num_threads; i++) {
		pthread_join(thread[i], NULL);
	}
	
	free(thread);

	return true; /* TODO: finish this function and remove this stub */
}

int main(int argc, char *argv[]) {
	int array[16] = {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};
	int size = sizeof(array)/sizeof(*array);
	for(int i=0; i<size; i++) {
		assert(array[i] == 2);
	}
	char* error = NULL;
	if( zero_array(array, sizeof(array)/sizeof(*array), 4, &error) ) {
		for(int i=0; i<size; i++) {
			assert(array[i] == 0);
		}
		return EXIT_SUCCESS;
	}
	else {
		fprintf(stderr, "%s\n", error);
		free(error);
		return EXIT_FAILURE;
	}
}

