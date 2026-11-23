
/*
 *   Source [main.c]
 */

#include "rose/rose.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(void)
{
	ROSE_Init(L"My Window", 500, 500);
	//ROSE_WindowVSync(false);

	ROSE_Image* image = ROSE_ImageCreate(8, 8);
	for (uInt32 y = 0; y < 8; y++) {
		for (uInt32 x = 0; x < 8; x++) {
			ROSE_Color color = ((x + y) & 1) ? ROSE_COLOR_WHITE : ROSE_COLOR_PURPLE;
			ROSE_ImageSetPixel(image, x, y, color);
		}
	}

	ROSE_Texture* texture = ROSE_TextureCreate(image);
	ROSE_ImageDestroy(image);

	ROSE_Sprite* checker = ROSE_SpriteCreate(texture);
	ROSE_Sprite* dot     = ROSE_SpriteCreate(NULL);

	float time = 0.0f;
	while (true) {
		if (ROSE_WindowClosed())
			break;

		if (ROSE_WindowMinimized())
			continue;

		ROSE_FrameClear(ROSE_COLOR_RGBA(30, 15, 70, 255));

		for (uInt32 gy = 0; gy < 30; gy++) {
			for (uInt32 gx = 0; gx < 30; gx++) {
				float wobble = sinf(time * 3.0f + (float)(gx + gy) * 0.4f) * 4.0f;
				float x = 10.0f + (float)gx * 16.0f;
				float y = 10.0f + (float)gy * 16.0f + wobble;
				ROSE_SpriteDraw(checker, x, y, 1.5f, ROSE_COLOR_WHITE);
			}
		}

		float dot_x = 250.0f + sinf(time) * 200.0f;
		ROSE_SpriteDraw(dot, dot_x, 240.0f, 20.0f, ROSE_COLOR_RED);

		ROSE_FrameRender();
		time += ROSE_FrameDelta();

		fprintf(stdout, "FPS: %d\n", (int)ROSE_WindowFPS());
	}

	ROSE_TextureDestroy(texture);
	ROSE_SpriteDestroy(checker);
	ROSE_SpriteDestroy(dot);
	ROSE_Quit();
}

