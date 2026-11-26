
/*
 *   Source [rose_peripherals.c]
 */

#include "rose_internal.h"

static void mouse_update(void);
static void keyboard_scancode(ROSE_Scancode, bool);

// MOUSE

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

ROSE_Vec2 ROSE_MousePosition(void)
{
	return rose__mouse.curr_pos;
}

ROSE_Vec2 ROSE_MouseVelocity(void)
{
	return ROSE_Vec2Sub(rose__mouse.curr_pos, rose__mouse.prev_pos);
}

// KEYBOARD

bool ROSE_KeyboardPressing(ROSE_Scancode scancode)
{
	assert(scancode < ROSE_SCANCODE_COUNT);
	return rose__keyboard.pressing[scancode];
}

bool ROSE_KeyboardPressed(ROSE_Scancode scancode)
{
	assert(scancode < ROSE_SCANCODE_COUNT);
	return rose__keyboard.pressed[scancode];
}

bool ROSE_KeyboardReleased(ROSE_Scancode scancode)
{
	assert(scancode < ROSE_SCANCODE_COUNT);
	return rose__keyboard.released[scancode];
}

// INTERNAL MOUSE

void ROSE__MouseClear(void)
{
	memset(rose__mouse.pressed,  0, sizeof(rose__mouse.pressed));
	memset(rose__mouse.released, 0, sizeof(rose__mouse.released));
	rose__mouse.scroll = 0.0f;
	rose__mouse.prev_pos.x = rose__mouse.curr_pos.x;
	rose__mouse.prev_pos.y = rose__mouse.curr_pos.y;
}

void ROSE__MouseButton(ROSE_MouseButton button, bool pressing)
{
	bool prev = rose__mouse.pressing[button];

	if (pressing && !prev)
		rose__mouse.pressed[button] = true;

	if (!pressing && prev)
		rose__mouse.released[button] = true;

	rose__mouse.pressing[button] = pressing;
	mouse_update();
}

void ROSE__MouseRelease(void)
{
	for (uInt32 i = 0; i < ROSE_MOUSE_COUNT; i++) {
		if (rose__mouse.pressing[i])
			rose__mouse.released[i] = true;
		rose__mouse.pressing[i] = false;
	}

	mouse_update();
}

static void mouse_update(void)
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

// INTERNAL KEYBOARD

static const uInt8 scancodes[512] = {
	[0x001] = ROSE_SCANCODE_ESCAPE,
	[0x002] = ROSE_SCANCODE_1,
	[0x003] = ROSE_SCANCODE_2,
	[0x004] = ROSE_SCANCODE_3,
	[0x005] = ROSE_SCANCODE_4,
	[0x006] = ROSE_SCANCODE_5,
	[0x007] = ROSE_SCANCODE_6,
	[0x008] = ROSE_SCANCODE_7,
	[0x009] = ROSE_SCANCODE_8,
	[0x00A] = ROSE_SCANCODE_9,
	[0x00B] = ROSE_SCANCODE_0,
	
	[0x00C] = ROSE_SCANCODE_MINUS,
	[0x00D] = ROSE_SCANCODE_EQUALS,
	[0x00E] = ROSE_SCANCODE_BACKSPACE,
	[0x00F] = ROSE_SCANCODE_TAB,

	[0x010] = ROSE_SCANCODE_Q,
	[0x011] = ROSE_SCANCODE_W,
	[0x012] = ROSE_SCANCODE_E,
	[0x013] = ROSE_SCANCODE_R,
	[0x014] = ROSE_SCANCODE_T,
	[0x015] = ROSE_SCANCODE_Y,
	[0x016] = ROSE_SCANCODE_U,
	[0x017] = ROSE_SCANCODE_I,
	[0x018] = ROSE_SCANCODE_O,
	[0x019] = ROSE_SCANCODE_P,

	[0x01A] = ROSE_SCANCODE_LEFT_BRACKET,
	[0x01B] = ROSE_SCANCODE_RIGHT_BRACKET,
	[0x01C] = ROSE_SCANCODE_ENTER,
	[0x01D] = ROSE_SCANCODE_LEFT_CTRL,

	[0x01E] = ROSE_SCANCODE_A,
	[0x01F] = ROSE_SCANCODE_S,
	[0x020] = ROSE_SCANCODE_D,
	[0x021] = ROSE_SCANCODE_F,
	[0x022] = ROSE_SCANCODE_G,
	[0x023] = ROSE_SCANCODE_H,
	[0x024] = ROSE_SCANCODE_J,
	[0x025] = ROSE_SCANCODE_K,
	[0x026] = ROSE_SCANCODE_L,

	[0x027] = ROSE_SCANCODE_SEMICOLON,
	[0x028] = ROSE_SCANCODE_APOSTROPHE,
	[0x029] = ROSE_SCANCODE_GRAVE,
	[0x02A] = ROSE_SCANCODE_LEFT_SHIFT,
	[0x02B] = ROSE_SCANCODE_BACKSLASH,

	[0x02C] = ROSE_SCANCODE_Z,
	[0x02D] = ROSE_SCANCODE_X,
	[0x02E] = ROSE_SCANCODE_C,
	[0x02F] = ROSE_SCANCODE_V,
	[0x030] = ROSE_SCANCODE_B,
	[0x031] = ROSE_SCANCODE_N,
	[0x032] = ROSE_SCANCODE_M,

	[0x033] = ROSE_SCANCODE_COMMA,
	[0x034] = ROSE_SCANCODE_PERIOD,
	[0x035] = ROSE_SCANCODE_SLASH,
	[0x036] = ROSE_SCANCODE_RIGHT_SHIFT,
	[0x037] = ROSE_SCANCODE_KP_MULTIPLY,
	[0x038] = ROSE_SCANCODE_LEFT_ALT,
	[0x039] = ROSE_SCANCODE_SPACE,
	[0x03A] = ROSE_SCANCODE_CAPS_LOCK,

	[0x03B] = ROSE_SCANCODE_F1,
	[0x03C] = ROSE_SCANCODE_F2,
	[0x03D] = ROSE_SCANCODE_F3,
	[0x03E] = ROSE_SCANCODE_F4,
	[0x03F] = ROSE_SCANCODE_F5,
	[0x040] = ROSE_SCANCODE_F6,
	[0x041] = ROSE_SCANCODE_F7,
	[0x042] = ROSE_SCANCODE_F8,
	[0x043] = ROSE_SCANCODE_F9,
	[0x044] = ROSE_SCANCODE_F10,
	
	[0x045] = ROSE_SCANCODE_PAUSE,
	[0x046] = ROSE_SCANCODE_SCROLL_LOCK,

	[0x047] = ROSE_SCANCODE_KP_7,
	[0x048] = ROSE_SCANCODE_KP_8,
	[0x049] = ROSE_SCANCODE_KP_9,
	[0x04A] = ROSE_SCANCODE_KP_SUBTRACT,
	[0x04B] = ROSE_SCANCODE_KP_4,
	[0x04C] = ROSE_SCANCODE_KP_5,
	[0x04D] = ROSE_SCANCODE_KP_6,
	[0x04E] = ROSE_SCANCODE_KP_ADD,
	[0x04F] = ROSE_SCANCODE_KP_1,
	[0x050] = ROSE_SCANCODE_KP_2,
	[0x051] = ROSE_SCANCODE_KP_3,
	[0x052] = ROSE_SCANCODE_KP_0,
	[0x053] = ROSE_SCANCODE_KP_DECIMAL,

	[0x057] = ROSE_SCANCODE_F11,
	[0x058] = ROSE_SCANCODE_F12,

	[0x11C] = ROSE_SCANCODE_KP_ENTER,
	[0x11D] = ROSE_SCANCODE_RIGHT_CTRL,
	[0x135] = ROSE_SCANCODE_KP_DIVIDE,
	[0x137] = ROSE_SCANCODE_PRINT_SCREEN,
	[0x138] = ROSE_SCANCODE_RIGHT_ALT,
	[0x145] = ROSE_SCANCODE_NUM_LOCK,
	[0x147] = ROSE_SCANCODE_HOME,
	[0x148] = ROSE_SCANCODE_UP,
	[0x149] = ROSE_SCANCODE_PAGE_UP,
	[0x14B] = ROSE_SCANCODE_LEFT,
	[0x14D] = ROSE_SCANCODE_RIGHT,
	[0x14F] = ROSE_SCANCODE_END,
	[0x150] = ROSE_SCANCODE_DOWN,
	[0x151] = ROSE_SCANCODE_PAGE_DOWN,
	[0x152] = ROSE_SCANCODE_INSERT,
	[0x153] = ROSE_SCANCODE_DELETE,
	[0x15B] = ROSE_SCANCODE_LEFT_SUPER,
	[0x15C] = ROSE_SCANCODE_RIGHT_SUPER,
	[0x15D] = ROSE_SCANCODE_MENU,
};

static void keyboard_scancode(ROSE_Scancode scancode, bool pressing)
{
	bool prev = rose__keyboard.pressing[scancode];

	if (pressing && !prev)
		rose__keyboard.pressed[scancode] = true;

	if (!pressing && prev)
		rose__keyboard.released[scancode] = true;

	rose__keyboard.pressing[scancode] = pressing;
}

void ROSE__KeyboardMessage(UINT msg, WPARAM wparam, LPARAM lparam)
{
	UINT code = HIWORD(lparam) & (KF_EXTENDED | 0xFF);

	if (code == 0)
		code = MapVirtualKeyW((UINT)wparam, MAPVK_VK_TO_VSC);

	if (code == 0x054) code = 0x137;
	if (code == 0x146) code = 0x045;
	if (code == 0x136) code = 0x036;

	ROSE_Scancode scancode = (ROSE_Scancode)scancodes[code & 0x1FF];
	if (scancode == ROSE_SCANCODE_UNKNOWN)
		return;

	bool pressing =
		(msg == WM_KEYDOWN) ||
		(msg == WM_SYSKEYDOWN);

	keyboard_scancode(scancode, pressing);
}

void ROSE__KeyboardClear(void)
{
	memset(rose__keyboard.pressed,  0, sizeof(rose__keyboard.pressed));
	memset(rose__keyboard.released, 0, sizeof(rose__keyboard.released));
}

void ROSE__KeyboardRelease(void)
{
	for (uInt32 i = 0; i < ROSE_SCANCODE_COUNT; i++) {
		if (rose__keyboard.pressing[i])
			rose__keyboard.released[i] = true;
		rose__keyboard.pressing[i] = false;
	}
}

