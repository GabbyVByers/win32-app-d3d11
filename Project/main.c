
/*
 *   Source [main.c]
 */

#include "rose/rose.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CLEAR_COLOR ROSE_COLOR_RGBA(70, 20, 80, 255)

int main(void)
{
	ROSE_Init(L"My Window", 1200, 700);
	ROSE_WindowVSync(false);

	float value = 0;
	ROSE_Slider* slider = ROSE_SliderCreate((ROSE_Vec2){ 500.0f, 20.0f }, 0.0f, 1.0f, &value);

	ROSE_DrawInfo info = ROSE_DRAW_DEFAULT;
	info.position.x = 50.0f;
	info.position.y = 50.0f;

	while (true) {
		if (ROSE_WindowClosed())
			break;

		if (ROSE_WindowMinimized())
			continue;

		ROSE_FrameClear(CLEAR_COLOR);
		ROSE_SliderDraw(slider, info);
		ROSE_FrameRender();
	}

	ROSE_Quit();
}

//void camera_control(ROSE_Camera* camera, float dt)
//{
//	if (ROSE_MousePressed(ROSE_MOUSE_RIGHT))
//		ROSE_MouseCursorHide();
//
//	if (ROSE_MouseReleased(ROSE_MOUSE_RIGHT))
//		ROSE_MouseCursorReveal();
//
//	if (ROSE_MousePressing(ROSE_MOUSE_RIGHT)) {
//		ROSE_Vec2 mouse_vel = ROSE_MouseVelocity();
//		ROSE_Quat quat = camera->orientation;
//		quat = ROSE_QuatRotate(quat, ROSE_VEC3_YPOS, mouse_vel.x * 0.2f);
//		quat = ROSE_QuatRotate(quat, ROSE_QuatRight(quat), mouse_vel.y * 0.2f);
//		camera->orientation = quat;
//	}
//
//	float speed = 7.0f * dt;
//	ROSE_Vec3 forward = ROSE_QuatForward(camera->orientation);
//	forward.y = 0.0f;
//	forward = ROSE_Vec3Normalize(forward);
//	forward = ROSE_Vec3Scale(forward, speed);
//
//	ROSE_Vec3 right = ROSE_Vec3Scale(ROSE_QuatRight(camera->orientation), speed);
//	ROSE_Vec3 up = ROSE_Vec3Scale(ROSE_VEC3_YPOS, speed);
//
//	if (ROSE_KeyboardPressing(ROSE_SCANCODE_W)) camera->position = ROSE_Vec3Add(camera->position, forward);
//	if (ROSE_KeyboardPressing(ROSE_SCANCODE_S)) camera->position = ROSE_Vec3Sub(camera->position, forward);
//	if (ROSE_KeyboardPressing(ROSE_SCANCODE_D)) camera->position = ROSE_Vec3Add(camera->position, right);
//	if (ROSE_KeyboardPressing(ROSE_SCANCODE_A)) camera->position = ROSE_Vec3Sub(camera->position, right);
//
//	if (ROSE_KeyboardPressing(ROSE_SCANCODE_SPACE))
//		camera->position = ROSE_Vec3Add(camera->position, up);
//
//	if (ROSE_KeyboardPressing(ROSE_SCANCODE_LEFT_SHIFT))
//		camera->position = ROSE_Vec3Sub(camera->position, up);
//}

