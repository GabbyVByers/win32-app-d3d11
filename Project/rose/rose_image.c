
/*
 *   Source [rose_image.c]
 */

#define ROSE_EXTERN
#include "rose_internal.h"

ROSE_Image* ROSE_ImageCreate(uInt32 width, uInt32 height)
{
	ROSE_Image* image = calloc(1, sizeof(ROSE_Image));

	uInt32 count = width * height;
	uInt32* pixels = malloc(count * sizeof(uInt32));
	memset(pixels, (uInt8)255, count * sizeof(uInt32));

	image->width  = width;
	image->height = height;
	image->pixels = pixels;

	return image;
}

void ROSE_ImageDestroy(ROSE_Image* image)
{
	if (!image)
		return;

	if (image->pixels)
		free(image->pixels);

	free(image);
}

ROSE_Image* ROSE_ImageLoad(const char* path)
{
	ROSE_Image* image = calloc(1, sizeof(ROSE_Image));

	uInt32 width   = 0u;
	uInt32 height  = 0u;
	uInt32* pixels = NULL;
	
	int retcode = png_decode(path, &pixels, &width, &height);
	if (retcode != 0) {
		fprintf(stderr, "png_decode() failed! (error: %s)", png_error(retcode));
		exit(EXIT_FAILURE);
	}

	image->width  = width;
	image->height = height;
	image->pixels = pixels;

	return image;
}

void ROSE_ImageSave(ROSE_Image* image, const char* path)
{
	uInt32 width   = image->width;
	uInt32 height  = image->height;
	uInt32* pixels = image->pixels;

	int retcode = png_encode(path, pixels, width, height);
	if (retcode != 0) {
		fprintf(stderr, "png_encode() failed! (error: %s)", png_error(retcode));
		exit(EXIT_FAILURE);
	}
}

void ROSE_ImageSize(ROSE_Image* image, uInt32* width, uInt32* height)
{
	*width  = image->width;
	*height = image->height;
}

uInt32* ROSE_ImageCopyPixels(ROSE_Image* image)
{
	uInt32 width   = image->width;
	uInt32 height  = image->height;
	uInt32* pixels = image->pixels;

	if (!width || !height)
		return NULL;

	uInt32 count = width * height;
	uInt32* copy = malloc(count * sizeof(uInt32));
	memcpy(copy, pixels, count * sizeof(uInt32));

	return copy;
}

