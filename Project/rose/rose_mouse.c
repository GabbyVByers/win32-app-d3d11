
/*
 *   Source [rose_mouse.c]
 */

#include "rose_internal.h"

bool ROSE_MousePressing(ROSE_MouseButton button)
{
	assert(button < ROSE_MOUSE_COUNT);
	return rose__mouse.pressing[button];
}

bool ROSE_MousePressed(ROSE_MouseButton button)
{
	assert(button < ROSE_MOUSE_COUNT);
	return rose__mouse.pressed[button];
}

bool ROSE_MouseReleased(ROSE_MouseButton button)
{
	assert(button < ROSE_MOUSE_COUNT);
	return rose__mouse.released[button];
}

float ROSE_MouseScroll(void)
{
	return rose__mouse.scroll;
}

void ROSE_MousePosition(float* xpos, float* ypos)
{
	assert(xpos != NULL);
	assert(ypos != NULL);
	(*xpos) = rose__mouse.curr_xpos;
	(*ypos) = rose__mouse.curr_ypos;
}

void ROSE_MouseVelocity(float* xvel, float* yvel)
{
	assert(xvel != NULL);
	assert(yvel != NULL);
	(*xvel) = rose__mouse.curr_xpos - rose__mouse.prev_xpos;
	(*yvel) = rose__mouse.curr_ypos - rose__mouse.prev_ypos;
}

void rose__mouse_update(void)
{
	bool any = false;
	for (uInt32 i = 0; i < ROSE_MOUSE_COUNT; i++) {
		if (rose__mouse.pressing[i]) {
			any = true;
			break;
		}
	}

	bool capture = (any)  && (GetCapture() != rose__hwnd);
	bool release = (!any) && (GetCapture() == rose__hwnd);

	if (capture)
		SetCapture(rose__hwnd);

	if (release)
		ReleaseCapture();
}

void rose__mouse_clear(void)
{
	memset(rose__mouse.pressed,  0, sizeof(rose__mouse.pressed));
	memset(rose__mouse.released, 0, sizeof(rose__mouse.released));
	rose__mouse.scroll = 0.0f;
	rose__mouse.prev_xpos = rose__mouse.curr_xpos;
	rose__mouse.prev_ypos = rose__mouse.curr_ypos;
}

void rose__mouse_button(ROSE_MouseButton button, bool pressing)
{
	bool prev = rose__mouse.pressing[button];

	if (pressing && !prev)
		rose__mouse.pressed[button] = true;

	if (!pressing && prev)
		rose__mouse.released[button] = true;

	rose__mouse.pressing[button] = pressing;
	rose__mouse_update();
}

void rose__mouse_release(void)
{
	for (uInt32 i = 0; i < ROSE_MOUSE_COUNT; i++) {
		if (rose__mouse.pressing[i])
			rose__mouse.released[i] = true;
		rose__mouse.pressing[i] = false;
	}

	rose__mouse_update();
}

