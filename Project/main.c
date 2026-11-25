
/*
 *   Source [main.c]
 */

#include "rose/rose.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static void push_face(ROSE_Vertex3D* out, ROSE_Vec3 a, ROSE_Vec3 b, ROSE_Vec3 c, ROSE_Vec3 d, ROSE_Color color)
{
	ROSE_Vec3 corners[6] = { a, b, c, a, c, d };

	for (uInt32 i = 0; i < 6; i++)
		out[i] = (ROSE_Vertex3D){ corners[i].x, corners[i].y, corners[i].z, color };
}

int main(void)
{
	ROSE_Init(L"My Window", 1200, 700);
	ROSE_WindowVSync(false);

	/*
	 *   Cube
	 */

	const float h = 0.5f;
	ROSE_Vertex3D cube[36];

	push_face(cube,      (ROSE_Vec3) { -h,  h, -h }, (ROSE_Vec3) {  h,  h, -h }, (ROSE_Vec3) {  h, -h, -h }, (ROSE_Vec3) { -h, -h, -h }, ROSE_COLOR_RED);    // -Z
	push_face(cube +  6, (ROSE_Vec3) {  h,  h,  h }, (ROSE_Vec3) { -h,  h,  h }, (ROSE_Vec3) { -h, -h,  h }, (ROSE_Vec3) {  h, -h,  h }, ROSE_COLOR_CYAN);   // +Z
	push_face(cube + 12, (ROSE_Vec3) {  h,  h, -h }, (ROSE_Vec3) {  h,  h,  h }, (ROSE_Vec3) {  h, -h,  h }, (ROSE_Vec3) {  h, -h, -h }, ROSE_COLOR_GREEN);  // +X
	push_face(cube + 18, (ROSE_Vec3) { -h,  h,  h }, (ROSE_Vec3) { -h,  h, -h }, (ROSE_Vec3) { -h, -h, -h }, (ROSE_Vec3) { -h, -h,  h }, ROSE_COLOR_PURPLE); // -X
	push_face(cube + 24, (ROSE_Vec3) { -h,  h,  h }, (ROSE_Vec3) {  h,  h,  h }, (ROSE_Vec3) {  h,  h, -h }, (ROSE_Vec3) { -h,  h, -h }, ROSE_COLOR_BLUE);   // +Y
	push_face(cube + 30, (ROSE_Vec3) { -h, -h, -h }, (ROSE_Vec3) {  h, -h, -h }, (ROSE_Vec3) {  h, -h,  h }, (ROSE_Vec3) { -h, -h,  h }, ROSE_COLOR_YELLOW); // -Y

	ROSE_Mesh3D* cube_mesh = ROSE_Mesh3DCreate(cube, 36);
	ROSE_Quat spin = ROSE_QUAT_IDENTITY;

	ROSE_Camera camera = ROSE_CAMERA_DEFAULT;
	camera.position = (ROSE_Vec3) { 0.0f, 0.5f, -6.0f };

	/*
	 *   Text
	 */

	ROSE_Text* stats = ROSE_TextCreate("FPS: --");

	while (true) {
		if (ROSE_WindowClosed())
			break;

		if (ROSE_WindowMinimized())
			continue;

		float dt = ROSE_FrameDelta();

		if (ROSE_MousePressing(ROSE_MOUSE_LEFT)) {
			float dx, dy;
			ROSE_MouseVelocity(&dx, &dy);
			ROSE_Quat quat = camera.orientation;
			ROSE_Vec3 right = ROSE_QuatRight(quat);
			quat = ROSE_QuatRotate(quat, (ROSE_Vec3) { 0.0f, 1.0f, 0.0f }, dx * 0.2f);
			quat = ROSE_QuatRotate(quat, right, dy * 0.2f);
			camera.orientation = quat;
		}

		float speed = 7.0f * dt;
		ROSE_Vec3 world_up = { 0.0f, 1.0f, 0.0f };

		if (ROSE_KeyboardPressing(ROSE_SCANCODE_W)) {
			ROSE_Quat quat = camera.orientation;
			ROSE_Vec3 forward = ROSE_QuatForward(quat);
			camera.position.x += forward.x * speed;
			camera.position.y += forward.y * speed;
			camera.position.z += forward.z * speed;
		}

		if (ROSE_KeyboardPressing(ROSE_SCANCODE_S)) {
			ROSE_Quat quat = camera.orientation;
			ROSE_Vec3 forward = ROSE_QuatForward(quat);
			camera.position.x -= forward.x * speed;
			camera.position.y -= forward.y * speed;
			camera.position.z -= forward.z * speed;
		}

		if (ROSE_KeyboardPressing(ROSE_SCANCODE_D)) {
			ROSE_Quat quat = camera.orientation;
			ROSE_Vec3 right = ROSE_QuatRight(quat);
			camera.position.x += right.x * speed;
			camera.position.y += right.y * speed;
			camera.position.z += right.z * speed;
		}

		if (ROSE_KeyboardPressing(ROSE_SCANCODE_A)) {
			ROSE_Quat quat = camera.orientation;
			ROSE_Vec3 right = ROSE_QuatRight(quat);
			camera.position.x -= right.x * speed;
			camera.position.y -= right.y * speed;
			camera.position.z -= right.z * speed;
		}

		if (ROSE_KeyboardPressing(ROSE_SCANCODE_SPACE)) {

		}

		if (ROSE_KeyboardPressing(ROSE_SCANCODE_LEFT_SHIFT)) {

		}

		spin = ROSE_QuatRotate(spin, (ROSE_Vec3) { 0.0f, 1.0f, 0.0f }, 60.0f * dt);
		spin = ROSE_QuatRotate(spin, (ROSE_Vec3) { 1.0f, 0.0f, 0.0f }, 25.0f * dt);

		ROSE_FrameClear(ROSE_COLOR_RGBA(30, 15, 70, 255));

		ROSE_Mesh3DDraw(cube_mesh, camera, (ROSE_Vec3) { 0.0f, 0.0f, 0.0f }, spin, 1.0f, ROSE_COLOR_WHITE);

		for (int i = 0; i < 8; i++) {
			ROSE_Quat facing = ROSE_QuatRotate(ROSE_QUAT_IDENTITY, world_up, (float)i * 45.0f);
			ROSE_Vec3 out = ROSE_QuatForward(facing);
			ROSE_Vec3 pos = { out.x * 3.0f, -1.0f, out.z * 3.0f };
			ROSE_Mesh3DDraw(cube_mesh, camera, pos, facing, 0.4f, ROSE_COLOR_WHITE);
		}

		ROSE_Vec3 cam_pos = camera.position;
		ROSE_TextUpdate(stats, "FPS: %.0f\nPOS: %.1f %.1f %.1f", ROSE_WindowFPS(), cam_pos.x, cam_pos.y, cam_pos.z);
		ROSE_TextDraw(stats, 8.0f, 8.0f, 2.0f, ROSE_COLOR_YELLOW);

		ROSE_FrameRender();
	}

	ROSE_TextDestroy(stats);

	ROSE_Quit();
}

