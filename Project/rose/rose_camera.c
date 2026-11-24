
/*
 *   Source [rose_camera.c]
 */

#include "rose_internal.h"

void ROSE_CameraFOV(float degrees)
{
	rose__camera.fov = degrees;
}

void ROSE_CameraNearPlane(float distance)
{
	rose__camera.near_plane = distance;
}

void ROSE_CameraFarPlane(float distance)
{
	rose__camera.far_plane = distance;
}

void ROSE_CameraPositionMove(ROSE_Vec3 direction, float distance)
{
	ROSE_Vec3 step = rose__vec3_scale(rose__vec3_normalize(direction), distance);
	rose__camera.position = rose__vec3_add(rose__camera.position, step);
}

void ROSE_CameraPositionSet(ROSE_Vec3 position)
{
	rose__camera.position = position;
}

ROSE_Vec3 ROSE_CameraPositionGet(void)
{
	return rose__camera.position;
}

void ROSE_CameraRotate(ROSE_Vec3 axis, float theta)
{
	rose__camera.orientation = ROSE_QuatRotate(rose__camera.orientation, axis, theta);
}

ROSE_Vec3 ROSE_CameraDirectionForward(void)
{
	return ROSE_QuatForward(rose__camera.orientation);
}

ROSE_Vec3 ROSE_CameraDirectionUp(void)
{
	return ROSE_QuatUp(rose__camera.orientation);
}

ROSE_Vec3 ROSE_CameraDirectionRight(void)
{
	return ROSE_QuatRight(rose__camera.orientation);
}

ROSE_Quat ROSE_CameraOrientationGet(void)
{
	return rose__camera.orientation;
}

void ROSE_CameraOrientationSet(ROSE_Quat orientation)
{
	rose__camera.orientation = rose__quat_normalize(orientation);
}

