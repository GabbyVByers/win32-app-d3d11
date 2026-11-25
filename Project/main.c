
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

	ROSE_Vertex3D vertices[] = {
		{ 0.0f, 1.0f, 0.0f, ROSE_COLOR_RED    }, { 1.0f, 1.0f, 0.0f, ROSE_COLOR_RED    }, { 1.0f, 0.0f, 0.0f, ROSE_COLOR_RED    },
		{ 0.0f, 1.0f, 0.0f, ROSE_COLOR_RED    }, { 1.0f, 0.0f, 0.0f, ROSE_COLOR_RED    }, { 0.0f, 0.0f, 0.0f, ROSE_COLOR_RED    },
		{ 1.0f, 1.0f, 1.0f, ROSE_COLOR_GREEN  }, { 0.0f, 1.0f, 1.0f, ROSE_COLOR_GREEN  }, { 0.0f, 0.0f, 1.0f, ROSE_COLOR_GREEN  },
		{ 1.0f, 1.0f, 1.0f, ROSE_COLOR_GREEN  }, { 0.0f, 0.0f, 1.0f, ROSE_COLOR_GREEN  }, { 1.0f, 0.0f, 1.0f, ROSE_COLOR_GREEN  },
		{ 1.0f, 1.0f, 0.0f, ROSE_COLOR_BLUE   }, { 1.0f, 1.0f, 1.0f, ROSE_COLOR_BLUE   }, { 1.0f, 0.0f, 1.0f, ROSE_COLOR_BLUE   },
		{ 1.0f, 1.0f, 0.0f, ROSE_COLOR_BLUE   }, { 1.0f, 0.0f, 1.0f, ROSE_COLOR_BLUE   }, { 1.0f, 0.0f, 0.0f, ROSE_COLOR_BLUE   },
		{ 0.0f, 1.0f, 1.0f, ROSE_COLOR_PURPLE }, { 0.0f, 1.0f, 0.0f, ROSE_COLOR_PURPLE }, { 0.0f, 0.0f, 0.0f, ROSE_COLOR_PURPLE },
		{ 0.0f, 1.0f, 1.0f, ROSE_COLOR_PURPLE }, { 0.0f, 0.0f, 0.0f, ROSE_COLOR_PURPLE }, { 0.0f, 0.0f, 1.0f, ROSE_COLOR_PURPLE },
		{ 0.0f, 1.0f, 1.0f, ROSE_COLOR_CYAN   }, { 1.0f, 1.0f, 1.0f, ROSE_COLOR_CYAN   }, { 1.0f, 1.0f, 0.0f, ROSE_COLOR_CYAN   },
		{ 0.0f, 1.0f, 1.0f, ROSE_COLOR_CYAN   }, { 1.0f, 1.0f, 0.0f, ROSE_COLOR_CYAN   }, { 0.0f, 1.0f, 0.0f, ROSE_COLOR_CYAN   },
		{ 0.0f, 0.0f, 0.0f, ROSE_COLOR_YELLOW }, { 1.0f, 0.0f, 0.0f, ROSE_COLOR_YELLOW }, { 1.0f, 0.0f, 1.0f, ROSE_COLOR_YELLOW },
		{ 0.0f, 0.0f, 0.0f, ROSE_COLOR_YELLOW }, { 1.0f, 0.0f, 1.0f, ROSE_COLOR_YELLOW }, { 0.0f, 0.0f, 1.0f, ROSE_COLOR_YELLOW },
	};

	uInt32 count = sizeof(vertices) / sizeof(vertices[0]);
	ROSE_Mesh3D* mesh = ROSE_Mesh3DCreate(vertices, count);

	ROSE_Camera camera = ROSE_CAMERA_DEFAULT;
	camera.position = (ROSE_Vec3) { 0.0f, 0.0f, -3.0f };

	while (true) {
		if (ROSE_WindowClosed())
			break;

		if (ROSE_WindowMinimized())
			continue;

		float dt = ROSE_FrameDelta();
		camera_control(&camera, dt);

		ROSE_FrameClear(ROSE_COLOR_RGBA(30, 15, 70, 255));
		ROSE_Mesh3DDraw(mesh, camera, ROSE_VEC3_ZERO, ROSE_QUAT_IDENTITY, 1.0f, ROSE_COLOR_WHITE);
		ROSE_FrameRender();
	}

	ROSE_Mesh3DDestroy(mesh);
	ROSE_Quit();
}

static void camera_control(ROSE_Camera* camera, float dt)
{
	if (ROSE_MousePressing(ROSE_MOUSE_LEFT)) {
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

