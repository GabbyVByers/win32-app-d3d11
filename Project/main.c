
/*
 *   Source [main.c]
 */

#include "rose/rose.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static void camera_control(ROSE_Camera* camera, float dt);

int main(void)
{
	ROSE_Init(L"My Window", 1200, 700);
	ROSE_WindowVSync(false);

	ROSE_VertexMesh vertices[] = {
		{ { -.5,  .5, -.5 }, { 0, 0 }, ROSE_COLOR_RED    }, { {  .5,  .5, -.5 }, { 1, 0 }, ROSE_COLOR_RED    }, { {  .5, -.5, -.5 }, { 1, 1 }, ROSE_COLOR_RED    },
		{ { -.5,  .5, -.5 }, { 0, 0 }, ROSE_COLOR_RED    }, { {  .5, -.5, -.5 }, { 1, 1 }, ROSE_COLOR_RED    }, { { -.5, -.5, -.5 }, { 0, 1 }, ROSE_COLOR_RED    },
		{ {  .5,  .5,  .5 }, { 0, 0 }, ROSE_COLOR_CYAN   }, { { -.5,  .5,  .5 }, { 1, 0 }, ROSE_COLOR_CYAN   }, { { -.5, -.5,  .5 }, { 1, 1 }, ROSE_COLOR_CYAN   },
		{ {  .5,  .5,  .5 }, { 0, 0 }, ROSE_COLOR_CYAN   }, { { -.5, -.5,  .5 }, { 1, 1 }, ROSE_COLOR_CYAN   }, { {  .5, -.5,  .5 }, { 0, 1 }, ROSE_COLOR_CYAN   },
		{ {  .5,  .5, -.5 }, { 0, 0 }, ROSE_COLOR_BLUE   }, { {  .5,  .5,  .5 }, { 1, 0 }, ROSE_COLOR_BLUE   }, { {  .5, -.5,  .5 }, { 1, 1 }, ROSE_COLOR_BLUE   },
		{ {  .5,  .5, -.5 }, { 0, 0 }, ROSE_COLOR_BLUE   }, { {  .5, -.5,  .5 }, { 1, 1 }, ROSE_COLOR_BLUE   }, { {  .5, -.5, -.5 }, { 0, 1 }, ROSE_COLOR_BLUE   },
		{ { -.5,  .5,  .5 }, { 0, 0 }, ROSE_COLOR_PURPLE }, { { -.5,  .5, -.5 }, { 1, 0 }, ROSE_COLOR_PURPLE }, { { -.5, -.5, -.5 }, { 1, 1 }, ROSE_COLOR_PURPLE },
		{ { -.5,  .5,  .5 }, { 0, 0 }, ROSE_COLOR_PURPLE }, { { -.5, -.5, -.5 }, { 1, 1 }, ROSE_COLOR_PURPLE }, { { -.5, -.5,  .5 }, { 0, 1 }, ROSE_COLOR_PURPLE },
		{ { -.5,  .5,  .5 }, { 0, 0 }, ROSE_COLOR_GREEN  }, { {  .5,  .5,  .5 }, { 1, 0 }, ROSE_COLOR_GREEN  }, { {  .5,  .5, -.5 }, { 1, 1 }, ROSE_COLOR_GREEN  },
		{ { -.5,  .5,  .5 }, { 0, 0 }, ROSE_COLOR_GREEN  }, { {  .5,  .5, -.5 }, { 1, 1 }, ROSE_COLOR_GREEN  }, { { -.5,  .5, -.5 }, { 0, 1 }, ROSE_COLOR_GREEN  },
		{ { -.5, -.5, -.5 }, { 0, 0 }, ROSE_COLOR_YELLOW }, { {  .5, -.5, -.5 }, { 1, 0 }, ROSE_COLOR_YELLOW }, { {  .5, -.5,  .5 }, { 1, 1 }, ROSE_COLOR_YELLOW },
		{ { -.5, -.5, -.5 }, { 0, 0 }, ROSE_COLOR_YELLOW }, { {  .5, -.5,  .5 }, { 1, 1 }, ROSE_COLOR_YELLOW }, { { -.5, -.5,  .5 }, { 0, 1 }, ROSE_COLOR_YELLOW },
	};

	uInt32 count = sizeof(vertices) / sizeof(vertices[0]);
	ROSE_Mesh* mesh = ROSE_MeshCreate(vertices, count);

	ROSE_Camera camera = ROSE_CAMERA_DEFAULT;
	camera.position = (ROSE_Vec3){ 0.0f, 0.0f, -3.0f };

	float scale = 1.0f;
	ROSE_Slider* slider = ROSE_SliderCreate((ROSE_Vec2) { 300.0f, 16.0f }, 0.25f, 3.0f, &scale);

	while (true) {
		if (ROSE_WindowClosed())
			break;

		if (ROSE_WindowMinimized())
			continue;

		float dt = ROSE_FrameDelta();
		camera_control(&camera, dt);

		ROSE_MeshDrawInfo info = ROSE_MESH_DRAW_DEFAULT;
		info.scale = scale;

		ROSE_DrawInfo ui = ROSE_DRAW_DEFAULT;
		ui.position = (ROSE_Vec2) { 40.0f, 40.0f };

		ROSE_FrameClear(ROSE_COLOR_RGBA(30, 15, 70, 255));
		ROSE_MeshDraw(mesh, info, camera);
		ROSE_SliderDraw(slider, ui);
		ROSE_FrameRender();
	}

	ROSE_SliderDestroy(slider);
	ROSE_MeshDestroy(mesh);
	ROSE_Quit();
}

static void camera_control(ROSE_Camera* camera, float dt)
{
	if (ROSE_MousePressed(ROSE_MOUSE_RIGHT))
		ROSE_MouseCursorHide();

	if (ROSE_MouseReleased(ROSE_MOUSE_RIGHT))
		ROSE_MouseCursorReveal();

	if (ROSE_MousePressing(ROSE_MOUSE_RIGHT)) {
		ROSE_Vec2 mouse_vel = ROSE_MouseVelocity();
		ROSE_Quat quat = camera->orientation;
		quat = ROSE_QuatRotate(quat, ROSE_VEC3_YPOS, mouse_vel.x * 0.2f);
		quat = ROSE_QuatRotate(quat, ROSE_QuatRight(quat), mouse_vel.y * 0.2f);
		camera->orientation = quat;
	}

	float speed = 7.0f * dt;
	ROSE_Vec3 forward = ROSE_QuatForward(camera->orientation);
	forward.y = 0.0f;
	forward = ROSE_Vec3Normalize(forward);
	forward = ROSE_Vec3Scale(forward, speed);

	ROSE_Vec3 right = ROSE_Vec3Scale(ROSE_QuatRight(camera->orientation), speed);
	ROSE_Vec3 up = ROSE_Vec3Scale(ROSE_VEC3_YPOS, speed);

	if (ROSE_KeyboardPressing(ROSE_SCANCODE_W)) camera->position = ROSE_Vec3Add(camera->position, forward);
	if (ROSE_KeyboardPressing(ROSE_SCANCODE_S)) camera->position = ROSE_Vec3Sub(camera->position, forward);
	if (ROSE_KeyboardPressing(ROSE_SCANCODE_D)) camera->position = ROSE_Vec3Add(camera->position, right);
	if (ROSE_KeyboardPressing(ROSE_SCANCODE_A)) camera->position = ROSE_Vec3Sub(camera->position, right);

	if (ROSE_KeyboardPressing(ROSE_SCANCODE_SPACE))
		camera->position = ROSE_Vec3Add(camera->position, up);

	if (ROSE_KeyboardPressing(ROSE_SCANCODE_LEFT_SHIFT))
		camera->position = ROSE_Vec3Sub(camera->position, up);
}

