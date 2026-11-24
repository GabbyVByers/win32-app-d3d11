
/*
 *   Source [rose_math.c]
 */

#include "rose_internal.h"

#define DEGREE_TO_RADIAN (3.1415926535897f / 180.0f)
#define EPSILON (1e-8f)

// VECTOR

ROSE_Vec3 rose__vec3_add(ROSE_Vec3 vec_a, ROSE_Vec3 vec_b)
{
	return (ROSE_Vec3) {
		vec_a.x + vec_b.x,
		vec_a.y + vec_b.y,
		vec_a.z + vec_b.z
	};
}

ROSE_Vec3 rose__vec3_scale(ROSE_Vec3 vec, float scale)
{
	return (ROSE_Vec3) {
		vec.x * scale,
		vec.y * scale,
		vec.z * scale
	};
}

float rose__vec3_dot(ROSE_Vec3 vec_a, ROSE_Vec3 vec_b)
{
	return
		(vec_a.x * vec_b.x) +
		(vec_a.y * vec_b.y) +
		(vec_a.z * vec_b.z);
}

static ROSE_Vec3 vec3_cross(ROSE_Vec3 vec_a, ROSE_Vec3 vec_b)
{
	return (ROSE_Vec3) {
		(vec_a.y * vec_b.z) - (vec_a.z * vec_b.y),
		(vec_a.z * vec_b.x) - (vec_a.x * vec_b.z),
		(vec_a.x * vec_b.y) - (vec_a.y * vec_b.x),
	};
}

ROSE_Vec3 rose__vec3_normalize(ROSE_Vec3 vec)
{
	float dot = rose__vec3_dot(vec, vec);
	float length = sqrtf(dot);

	if (length < EPSILON)
		return (ROSE_Vec3) { 0.0f, 0.0f, 0.0f };

	float scale = (1.0f / length);
	return rose__vec3_scale(vec, scale);
}

// QUATERNION

static ROSE_Quat quat_multiply(ROSE_Quat a, ROSE_Quat b)
{
	return (ROSE_Quat) {
		.w = (a.w * b.w) - (a.x * b.x) - (a.y * b.y) - (a.z * b.z),
		.x = (a.w * b.x) + (a.x * b.w) + (a.y * b.z) - (a.z * b.y),
		.y = (a.w * b.y) - (a.x * b.z) + (a.y * b.w) + (a.z * b.x),
		.z = (a.w * b.z) + (a.x * b.y) - (a.y * b.x) + (a.z * b.w),
	};
}

ROSE_Quat rose__quat_normalize(ROSE_Quat quat)
{
	float length_squared =
		(quat.w * quat.w) +
		(quat.x * quat.x) +
		(quat.y * quat.y) +
		(quat.z * quat.z);

	float length = sqrtf(length_squared);

	if (length < EPSILON)
		return ROSE_QUAT_IDENTITY;

	float inverse = 1.0f / length;
	return (ROSE_Quat) {
		quat.w* inverse,
		quat.x* inverse,
		quat.y* inverse,
		quat.z* inverse
	};
}

static ROSE_Vec3 quat_rotate_vector(ROSE_Quat quat, ROSE_Vec3 vec)
{
	ROSE_Vec3 u = { quat.x, quat.y, quat.z };
	ROSE_Vec3 t = rose__vec3_scale(vec3_cross(u, vec), 2.0f);
	return rose__vec3_add(rose__vec3_add(vec, rose__vec3_scale(t, quat.w)), vec3_cross(u, t));
}

ROSE_Quat ROSE_QuatRotate(ROSE_Quat quat, ROSE_Vec3 axis, float degrees)
{
	ROSE_Vec3 a = rose__vec3_normalize(axis);

	if (rose__vec3_dot(a, a) == 0.0f)
		return quat;

	float half = degrees * DEGREE_TO_RADIAN * 0.5f;
	float sine = sinf(half);
	ROSE_Quat result = { cosf(half), a.x * sine, a.y * sine, a.z * sine };

	return rose__quat_normalize(quat_multiply(result, quat));
}

ROSE_Vec3 ROSE_QuatForward(ROSE_Quat quat)
{
	ROSE_Vec3 forward_basis = { 0.0f, 0.0f, 1.0f };
	return quat_rotate_vector(quat, forward_basis);
}

ROSE_Vec3 ROSE_QuatUp(ROSE_Quat quat)
{
	ROSE_Vec3 up_basis = { 0.0f, 1.0f, 0.0f };
	return quat_rotate_vector(quat, up_basis);
}

ROSE_Vec3 ROSE_QuatRight(ROSE_Quat quat)
{
	ROSE_Vec3 right_basis = { 1.0f, 0.0f, 0.0f };
	return quat_rotate_vector(quat, right_basis);
}

// MATRIX

rose__matrix_t rose__matrix_multiply(rose__matrix_t a, rose__matrix_t b)
{
	rose__matrix_t result = { 0 };

	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			float sum = 0.0f;
			for (int k = 0; k < 4; k++)
				sum += a.m[row * 4 + k] * b.m[k * 4 + col];
			result.m[row * 4 + col] = sum;
		}
	}

	return result;
}

rose__matrix_t rose__matrix_model(ROSE_Vec3 position, ROSE_Quat orientation, float scale)
{
	ROSE_Vec3 r = rose__vec3_scale(ROSE_QuatRight(orientation), scale);
	ROSE_Vec3 u = rose__vec3_scale(ROSE_QuatUp(orientation), scale);
	ROSE_Vec3 f = rose__vec3_scale(ROSE_QuatForward(orientation), scale);
	ROSE_Vec3 p = position;

	return (rose__matrix_t) {{
		r.x, r.y, r.z, 0.0f,
		u.x, u.y, u.z, 0.0f,
		f.x, f.y, f.z, 0.0f,
		p.x, p.y, p.z, 1.0f,
	}};
}

rose__matrix_t rose__matrix_view(ROSE_Vec3 position, ROSE_Quat orientation)
{
	ROSE_Vec3 r = ROSE_QuatRight(orientation);
	ROSE_Vec3 u = ROSE_QuatUp(orientation);
	ROSE_Vec3 f = ROSE_QuatForward(orientation);
	ROSE_Vec3 p = position;

	return (rose__matrix_t) {{
		r.x, u.x, f.x, 0.0f,
		r.y, u.y, f.y, 0.0f,
		r.z, u.z, f.z, 0.0f,
		-rose__vec3_dot(p, r), -rose__vec3_dot(p, u), -rose__vec3_dot(p, f), 1.0f,
	}};
}

rose__matrix_t rose__matrix_project(float fov_degrees, float aspect, float near_plane, float far_plane)
{
	float ys = 1.0f / tanf(fov_degrees * DEGREE_TO_RADIAN * 0.5f);
	float xs = ys / aspect;
	float zs = far_plane / (far_plane - near_plane);

	return (rose__matrix_t) {{
		xs, 0.0f, 0.0f, 0.0f,
		0.0f, ys, 0.0f, 0.0f,
		0.0f, 0.0f, zs, 1.0f,
		0.0f, 0.0f, -near_plane * zs, 0.0f,
	}};
}

