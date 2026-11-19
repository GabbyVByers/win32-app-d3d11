
/*
 *   Source [main.c]
 */

#include "app.h"

int main(void)
{
	app_init(L"My Window", 500, 500);

	vertex_t vertices[] = {
		{ .xpos = 250.0f, .ypos = 100.0f, .color = COLOR_RED   },
		{ .xpos = 450.0f, .ypos = 400.0f, .color = COLOR_GREEN },
		{ .xpos =  50.0f, .ypos = 400.0f, .color = COLOR_BLUE  },
	};

	uInt32 count =
		sizeof(vertices) /
		sizeof(vertices[0]);

	mesh_t* mesh = mesh_create(vertices, count);

	while (true) {
		if (app_closed())
			break;
		
		if (app_minimized())
			continue;

		app_clear(COLOR_RGBA(30, 15, 70, 255));
		app_draw(mesh);
		app_render();
	}

	mesh_destroy(mesh);
	app_quit();
}

