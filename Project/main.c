
/*
 *   Source [main.c]
 */

#include "app.h"

#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))

int main(void)
{
	app_init(L"My Window", 500, 500);

	vertex_t vertices[] = {
		{ .xpos = 0, .ypos = 0, .color = COLOR_RED   },
		{ .xpos = 0, .ypos = 0, .color = COLOR_GREEN },
		{ .xpos = 0, .ypos = 0, .color = COLOR_BLUE  },
	};

	while (true) {
		if (app_closed())
			break;

		if (app_minimized())
			continue;

		app_clear(COLOR_PURPLE);
		app_draw(vertices, ARRAY_SIZE(vertices));
		app_render();
	}

	app_quit();
}

