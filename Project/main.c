
/*
 *   Source [main.c]
 */

#include "rose/rose.h"

int main(void)
{
	ROSE_Init(L"My Window", 500, 500);

	ROSE_Image* image = ROSE_ImageCreate(8, 8);
	for (uInt32 y = 0; y < 8; y++) {
		for (uInt32 x = 0; x < 8; x++) {
			ROSE_Color color = ((x + y) & 1) ? ROSE_COLOR_WHITE : ROSE_COLOR_PURPLE;
			ROSE_ImageSetPixel(image, x, y, color);
		}
	}

	ROSE_Texture* texture = ROSE_TextureCreate(image);
	ROSE_ImageDestroy(image);

	float x0 = 100.0f, y0 = 100.0f;
	float x1 = 400.0f, y1 = 400.0f;

	ROSE_Vertex2D vertices[] = {
		{ .x = x0, .y = y0, .u = 0.0f, .v = 0.0f, .color = ROSE_COLOR_WHITE },
		{ .x = x1, .y = y0, .u = 1.0f, .v = 0.0f, .color = ROSE_COLOR_WHITE },
		{ .x = x0, .y = y1, .u = 0.0f, .v = 1.0f, .color = ROSE_COLOR_WHITE },
		{ .x = x1, .y = y0, .u = 1.0f, .v = 0.0f, .color = ROSE_COLOR_WHITE },
		{ .x = x1, .y = y1, .u = 1.0f, .v = 1.0f, .color = ROSE_COLOR_WHITE },
		{ .x = x0, .y = y1, .u = 0.0f, .v = 1.0f, .color = ROSE_COLOR_WHITE },
	};

	uInt32 count =
		sizeof(vertices) /
		sizeof(vertices[0]);

	ROSE_Mesh2D* mesh = ROSE_Mesh2DCreate(vertices, count);

	while (true) {
		if (ROSE_WindowClosed())
			break;

		if (ROSE_WindowMinimized())
			continue;

		ROSE_FrameClear(ROSE_COLOR_RGBA(30, 15, 70, 255));
		ROSE_Mesh2DDraw(mesh, texture);
		ROSE_FrameRender();
	}

	ROSE_TextureDestroy(texture);
	ROSE_Mesh2DDestroy(mesh);
	ROSE_Quit();
}

