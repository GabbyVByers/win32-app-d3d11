
/*
 *   Source [rose_math.c]
 */

#include "rose_internal.h"

#define DEGREE_TO_RADIAN (3.1415926535897f / 180.0f)
#define EPSILON (1e-8f)

// SCALAR

float ROSE_Clamp(float value, float min, float max)
{
	if (value < min) return min;
	if (value > max) return max;
	return value;
}

float ROSE_Lerp(float begin, float end, float t)
{
	return begin + ((end - begin) * t);
}

// VECTOR 2D

ROSE_Vec2 ROSE_Vec2Add(ROSE_Vec2 vec_a, ROSE_Vec2 vec_b)
{
	return (ROSE_Vec2) {
		vec_a.x + vec_b.x,
		vec_a.y + vec_b.y
	};
}

ROSE_Vec2 ROSE_Vec2Sub(ROSE_Vec2 vec_a, ROSE_Vec2 vec_b)
{
	return (ROSE_Vec2) {
		vec_a.x - vec_b.x,
		vec_a.y - vec_b.y
	};
}

ROSE_Vec2 ROSE_Vec2Scale(ROSE_Vec2 vec, float scale)
{
	return (ROSE_Vec2) {
		vec.x * scale,
		vec.y * scale
	};
}

ROSE_Vec2 ROSE_Vec2Lerp(ROSE_Vec2 begin, ROSE_Vec2 end, float t)
{
	return ROSE_Vec2Add(begin, ROSE_Vec2Scale(ROSE_Vec2Sub(end, begin), t));
}

float ROSE_Vec2Dot(ROSE_Vec2 vec_a, ROSE_Vec2 vec_b)
{
	return
		(vec_a.x * vec_b.x) +
		(vec_a.y * vec_b.y);
}

float ROSE_Vec2Cross(ROSE_Vec2 vec_a, ROSE_Vec2 vec_b)
{
	return
		(vec_a.x * vec_b.y) -
		(vec_a.y * vec_b.x);
}

ROSE_Vec2 ROSE_Vec2Normalize(ROSE_Vec2 vec)
{
	float length = ROSE_Vec2Length(vec);

	if (length < EPSILON)
		return ROSE_VEC2_ZERO;

	return ROSE_Vec2Scale(vec, 1.0f / length);
}

float ROSE_Vec2LengthSquared(ROSE_Vec2 vec)
{
	return ROSE_Vec2Dot(vec, vec);
}

float ROSE_Vec2Length(ROSE_Vec2 vec)
{
	return sqrtf(ROSE_Vec2LengthSquared(vec));
}

float ROSE_Vec2Distance(ROSE_Vec2 vec_a, ROSE_Vec2 vec_b)
{
	return ROSE_Vec2Length(ROSE_Vec2Sub(vec_a, vec_b));
}

// VECTOR 3D

ROSE_Vec3 ROSE_Vec3Add(ROSE_Vec3 vec_a, ROSE_Vec3 vec_b)
{
	return (ROSE_Vec3) {
		vec_a.x + vec_b.x,
		vec_a.y + vec_b.y,
		vec_a.z + vec_b.z
	};
}

ROSE_Vec3 ROSE_Vec3Sub(ROSE_Vec3 vec_a, ROSE_Vec3 vec_b)
{
	return (ROSE_Vec3) {
		vec_a.x - vec_b.x,
		vec_a.y - vec_b.y,
		vec_a.z - vec_b.z
	};
}

ROSE_Vec3 ROSE_Vec3Scale(ROSE_Vec3 vec, float scale)
{
	return (ROSE_Vec3) {
		vec.x * scale,
		vec.y * scale,
		vec.z * scale
	};
}

ROSE_Vec3 ROSE_Vec3Lerp(ROSE_Vec3 begin, ROSE_Vec3 end, float t)
{
	return ROSE_Vec3Add(begin, ROSE_Vec3Scale(ROSE_Vec3Sub(end, begin), t));
}

float ROSE_Vec3Dot(ROSE_Vec3 vec_a, ROSE_Vec3 vec_b)
{
	return
		(vec_a.x * vec_b.x) +
		(vec_a.y * vec_b.y) +
		(vec_a.z * vec_b.z);
}

ROSE_Vec3 ROSE_Vec3Cross(ROSE_Vec3 vec_a, ROSE_Vec3 vec_b)
{
	return (ROSE_Vec3) {
		(vec_a.y * vec_b.z) - (vec_a.z * vec_b.y),
		(vec_a.z * vec_b.x) - (vec_a.x * vec_b.z),
		(vec_a.x * vec_b.y) - (vec_a.y * vec_b.x)
	};
}

ROSE_Vec3 ROSE_Vec3Normalize(ROSE_Vec3 vec)
{
	float length = ROSE_Vec3Length(vec);

	if (length < EPSILON)
		return ROSE_VEC3_ZERO;

	return ROSE_Vec3Scale(vec, 1.0f / length);
}

float ROSE_Vec3LengthSquared(ROSE_Vec3 vec)
{
	return ROSE_Vec3Dot(vec, vec);
}

float ROSE_Vec3Length(ROSE_Vec3 vec)
{
	return sqrtf(ROSE_Vec3LengthSquared(vec));
}

float ROSE_Vec3Distance(ROSE_Vec3 vec_a, ROSE_Vec3 vec_b)
{
	return ROSE_Vec3Length(ROSE_Vec3Sub(vec_a, vec_b));
}

// QUATERNION

float ROSE_QuatDot(ROSE_Quat quat_a, ROSE_Quat quat_b)
{
	return
		(quat_a.w * quat_b.w) +
		(quat_a.x * quat_b.x) +
		(quat_a.y * quat_b.y) +
		(quat_a.z * quat_b.z);
}

ROSE_Quat ROSE_QuatNegate(ROSE_Quat quat)
{
	return (ROSE_Quat) {
		-quat.w,
		-quat.x,
		-quat.y,
		-quat.z
	};
}

ROSE_Quat ROSE_QuatConjugate(ROSE_Quat quat)
{
	return (ROSE_Quat) {
		 quat.w,
		-quat.x,
		-quat.y,
		-quat.z
	};
}

ROSE_Quat ROSE_QuatMult(ROSE_Quat quat_a, ROSE_Quat quat_b)
{
	return (ROSE_Quat) {
		.w = (quat_a.w * quat_b.w) - (quat_a.x * quat_b.x) - (quat_a.y * quat_b.y) - (quat_a.z * quat_b.z),
		.x = (quat_a.w * quat_b.x) + (quat_a.x * quat_b.w) + (quat_a.y * quat_b.z) - (quat_a.z * quat_b.y),
		.y = (quat_a.w * quat_b.y) - (quat_a.x * quat_b.z) + (quat_a.y * quat_b.w) + (quat_a.z * quat_b.x),
		.z = (quat_a.w * quat_b.z) + (quat_a.x * quat_b.y) - (quat_a.y * quat_b.x) + (quat_a.z * quat_b.w),
	};
}

ROSE_Quat ROSE_QuatSLerp(ROSE_Quat begin, ROSE_Quat end, float t)
{
	float dot = ROSE_QuatDot(begin, end);

	if (dot < 0.0f) {
		end = ROSE_QuatNegate(end);
		dot = -dot;
	}

	if (dot > 0.9995f) {
		ROSE_Quat result = {
			ROSE_Lerp(begin.w, end.w, t),
			ROSE_Lerp(begin.x, end.x, t),
			ROSE_Lerp(begin.y, end.y, t),
			ROSE_Lerp(begin.z, end.z, t),
		};

		return ROSE_QuatNormalize(result);
	}

	float theta = acosf(dot);
	float sine  = sinf(theta);
	float wa    = sinf((1.0f - t) * theta) / sine;
	float wb    = sinf(t * theta) / sine;

	return (ROSE_Quat) {
		(begin.w * wa) + (end.w * wb),
		(begin.x * wa) + (end.x * wb),
		(begin.y * wa) + (end.y * wb),
		(begin.z * wa) + (end.z * wb),
	};
}

ROSE_Quat ROSE_QuatNormalize(ROSE_Quat quat)
{
	float length = sqrtf(ROSE_QuatDot(quat, quat));

	if (length < EPSILON)
		return ROSE_QUAT_IDENTITY;

	float inverse = 1.0f / length;
	return (ROSE_Quat) {
		quat.w * inverse,
		quat.x * inverse,
		quat.y * inverse,
		quat.z * inverse
	};
}

ROSE_Vec3 ROSE_QuatRotateVector(ROSE_Quat quat, ROSE_Vec3 vec)
{
	ROSE_Vec3 u = { quat.x, quat.y, quat.z };
	ROSE_Vec3 t = ROSE_Vec3Scale(ROSE_Vec3Cross(u, vec), 2.0f);
	return ROSE_Vec3Add(ROSE_Vec3Add(vec, ROSE_Vec3Scale(t, quat.w)), ROSE_Vec3Cross(u, t));
}

ROSE_Quat ROSE_QuatRotate(ROSE_Quat quat, ROSE_Vec3 axis, float degrees)
{
	ROSE_Vec3 a = ROSE_Vec3Normalize(axis);

	if (ROSE_Vec3LengthSquared(a) == 0.0f)
		return quat;

	float half = degrees * DEGREE_TO_RADIAN * 0.5f;
	float sine = sinf(half);

	ROSE_Quat rotation = {
		cosf(half),
		a.x * sine,
		a.y * sine,
		a.z * sine
	};

	return ROSE_QuatNormalize(ROSE_QuatMult(rotation, quat));
}

ROSE_Vec3 ROSE_QuatForward(ROSE_Quat quat)
{
	return ROSE_QuatRotateVector(quat, ROSE_VEC3_ZPOS);
}

ROSE_Vec3 ROSE_QuatRight(ROSE_Quat quat)
{
	return ROSE_QuatRotateVector(quat, ROSE_VEC3_XPOS);
}

ROSE_Vec3 ROSE_QuatUp(ROSE_Quat quat)
{
	return ROSE_QuatRotateVector(quat, ROSE_VEC3_YPOS);
}

// MATRIX

ROSE_Matrix ROSE_MatrixMult(ROSE_Matrix a, ROSE_Matrix b)
{
	ROSE_Matrix result = { 0 };

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

ROSE_Matrix ROSE_MatrixModel(ROSE_Vec3 position, ROSE_Quat orientation, float scale)
{
	ROSE_Vec3 r = ROSE_Vec3Scale(ROSE_QuatRight(orientation), scale);
	ROSE_Vec3 u = ROSE_Vec3Scale(ROSE_QuatUp(orientation), scale);
	ROSE_Vec3 f = ROSE_Vec3Scale(ROSE_QuatForward(orientation), scale);
	ROSE_Vec3 p = position;

	return (ROSE_Matrix) {{
		r.x, r.y, r.z, 0.0f,
		u.x, u.y, u.z, 0.0f,
		f.x, f.y, f.z, 0.0f,
		p.x, p.y, p.z, 1.0f,
	}};
}

ROSE_Matrix ROSE_MatrixView(ROSE_Vec3 position, ROSE_Quat orientation)
{
	ROSE_Vec3 r = ROSE_QuatRight(orientation);
	ROSE_Vec3 u = ROSE_QuatUp(orientation);
	ROSE_Vec3 f = ROSE_QuatForward(orientation);
	ROSE_Vec3 p = position;

	return (ROSE_Matrix) {{
		r.x, u.x, f.x, 0.0f,
		r.y, u.y, f.y, 0.0f,
		r.z, u.z, f.z, 0.0f,
		-ROSE_Vec3Dot(p, r), -ROSE_Vec3Dot(p, u), -ROSE_Vec3Dot(p, f), 1.0f,
	}};
}

ROSE_Matrix ROSE_MatrixProject(float fov_degrees, float aspect, float near_plane, float far_plane)
{
	float ys = 1.0f / tanf(fov_degrees * DEGREE_TO_RADIAN * 0.5f);
	float xs = ys / aspect;
	float zs = far_plane / (far_plane - near_plane);

	return (ROSE_Matrix) {{
		xs,   0.0f, 0.0f,             0.0f,
		0.0f, ys,   0.0f,             0.0f,
		0.0f, 0.0f, zs,               1.0f,
		0.0f, 0.0f, -near_plane * zs, 0.0f,
	}};
}

ROSE_Matrix ROSE_MatrixMVP(ROSE_Matrix model, ROSE_Matrix view, ROSE_Matrix project)
{
	return ROSE_MatrixMult(ROSE_MatrixMult(model, view), project);
}

