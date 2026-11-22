
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
	if (image == NULL)
		return;

	if (image->pixels != NULL)
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
	assert(retcode == 0 && "png_decode() failed!");

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
	assert(retcode == 0 && "png_encode() failed!");
}

void ROSE_ImageResize(ROSE_Image* image, uInt32 width, uInt32 height)
{
	uInt32 count = width * height;
	uInt32* pixels = NULL;
	
	if (count != 0) {
		pixels = malloc(count * sizeof(uInt32));
		memset(pixels, 255, count * sizeof(uInt32));
	}

	bool copy =
		(pixels != NULL) &&
		(image->pixels != NULL);

	if (copy) {
		uInt32 copy_width = (width < image->width) ? width : image->width;
		uInt32 copy_height = (height < image->height) ? height : image->height;

		for (uInt32 y = 0; y < copy_height; y++) {
			uInt32 src_index = y * image->width;
			uInt32 dst_index = y * width;
			uInt32 copy_size = copy_width * sizeof(uInt32);
			memcpy(pixels + dst_index, image->pixels + src_index, copy_size);
		}
	}

	if (image->pixels != NULL) {
		free(image->pixels);
	}

	image->width  = width;
	image->height = height;
	image->pixels = pixels;
}

void ROSE_ImageFillColor(ROSE_Image* image, ROSE_Color color)
{
	uInt8 r = (uInt8)(color.r * 255.5f);
	uInt8 g = (uInt8)(color.g * 255.5f);
	uInt8 b = (uInt8)(color.b * 255.5f);
	uInt8 a = (uInt8)(color.a * 255.5f);

	uInt32 pixel =
		((uInt32)r)       |
		((uInt32)g <<  8) |
		((uInt32)b << 16) |
		((uInt32)a << 24);

	for (uInt32 y = 0; y < image->height; y++) {
		for (uInt32 x = 0; x < image->width; x++) {
			uInt32 i = (y * image->width) + x;
			image->pixels[i] = pixel;
		}
	}
}

ROSE_Color ROSE_ImageGetPixel(ROSE_Image* image, uInt32 xpos, uInt32 ypos)
{
	assert(xpos < image->width  && "bad bounds!");
	assert(ypos < image->height && "bad bounds!");

	uInt32 index = (ypos * image->width) + xpos;
	uInt32 pixel = image->pixels[index];
	
	float r = ((float)((uInt8)(pixel)))       / 255.0f;
	float g = ((float)((uInt8)(pixel >>  8))) / 255.0f;
	float b = ((float)((uInt8)(pixel >> 16))) / 255.0f;
	float a = ((float)((uInt8)(pixel >> 24))) / 255.0f;
	
	return (ROSE_Color) { r, g, b, a };
}

void ROSE_ImageSetPixel(ROSE_Image* image, uInt32 xpos, uInt32 ypos, ROSE_Color color)
{
	assert(xpos < image->width  && "bad bounds!");
	assert(ypos < image->height && "bad bounds!");

	uInt8 r = (uInt8)(color.r * 255.5f);
	uInt8 g = (uInt8)(color.g * 255.5f);
	uInt8 b = (uInt8)(color.b * 255.5f);
	uInt8 a = (uInt8)(color.a * 255.5f);

	uInt32 pixel =
		((uInt32)r)       |
		((uInt32)g <<  8) |
		((uInt32)b << 16) |
		((uInt32)a << 24);

	uInt32 index = (ypos * image->width) + xpos;
	image->pixels[index] = pixel;
}

void ROSE_ImageSize(ROSE_Image* image, uInt32* width, uInt32* height)
{
	*width  = image->width;
	*height = image->height;
}

uInt32* ROSE_ImageCopyPixels(ROSE_Image* image)
{
	if (image->pixels == NULL)
		return NULL;

	uInt32 count = image->width * image->height;
	uInt32* pixels = image->pixels;
	uInt32* copy = malloc(count * sizeof(uInt32));
	memcpy(copy, pixels, count * sizeof(uInt32));

	return copy;
}

