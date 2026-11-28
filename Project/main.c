
/*
 *   Source [main.c]
 */

#include "rose/rose.h"
#include "provinces.h"
#include "econ.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define COLOR_OCEAN   ROSE_COLOR_RGBA(0, 0, 100, 255)
#define COLOR_TERRAIN ROSE_COLOR_RGBA(0, 100, 0, 255)

typedef struct Territory {
	Province* province;
	ECON_Market market;
} Territory;

typedef struct World {
	Provinces provinces;
	Territory* territories;
	uInt32 territory_count;
} World;

int main(void)
{
	ROSE_Init(L"My Window", 1200, 700);
	ROSE_WindowVSync(false);

	World world = { 0 };

	{
		ROSE_Image* image = ROSE_ImageLoad("textures/map.png");
		world.provinces = GenerateWorldProvinces(image);
		ROSE_ImageDestroy(image);

		world.territories = calloc(world.provinces.count, sizeof(Territory));
		world.territory_count = world.provinces.count;

		for (uInt32 i = 0; i < world.territory_count; i++) {
			Territory* territory = &world.territories[i];
			territory->province = &world.provinces.array[i];
			territory->market = ECON_MarketInit();
		}
	}

	ROSE_ShapeDrawInfo draw_border = ROSE_SHAPE_DRAW_DEFAULT;
	draw_border.primitive = ROSE_LINES;
	draw_border.color = ROSE_COLOR_BLACK;

	ROSE_ShapeDrawInfo draw_provinces = ROSE_SHAPE_DRAW_DEFAULT;
	draw_provinces.color = COLOR_TERRAIN;

	while (true) {
		if (ROSE_WindowClosed())
			break;

		if (ROSE_WindowMinimized())
			continue;

		ROSE_FrameClear(COLOR_OCEAN);

		for (uInt32 i = 0; i < world.territory_count; i++) {
			Province* province = world.territories[i].province;
			ROSE_ShapeDraw(province->triangles, draw_provinces);
		}

		for (uInt32 i = 0; i < world.territory_count; i++) {
			Province* province = world.territories[i].province;
			ROSE_ShapeDraw(province->border, draw_border);
		}

		ROSE_FrameRender();
	}

	free(world.territories);
	DestroyWorldProvinces(&world.provinces);
	ROSE_Quit();
}

