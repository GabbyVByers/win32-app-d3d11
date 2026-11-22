
/*
 *   Source [main.c]
 */

#include "rose/rose.h"

int main(void)
{
	ROSE_Init(L"My Window", 500, 500);

	ROSE_Vertex2D vertices[] = {
		{ .xpos = 250.0f, .ypos = 100.0f, .color = ROSE_COLOR_RED   },
		{ .xpos = 430.0f, .ypos = 400.0f, .color = ROSE_COLOR_GREEN },
		{ .xpos =  70.0f, .ypos = 400.0f, .color = ROSE_COLOR_BLUE  },
	};

	size_t count =
		sizeof(vertices) /
		sizeof(vertices[0]);

	ROSE_Mesh* mesh = ROSE_MeshCreate(vertices, count);

	while (true) {
		if (ROSE_WindowClosed())
			break;
		
		if (ROSE_WindowMinimized())
			continue;

		ROSE_FrameClear(ROSE_COLOR_RGBA(30, 15, 70, 255));
		ROSE_MeshDraw(mesh);
		ROSE_FrameRender();
	}

	ROSE_MeshDestroy(mesh);
	ROSE_Quit();
}

