
/*
 *   Header [rose_math.h]
 */

#ifndef ROSE_MATH_HEADER_GUARD
#define ROSE_MATH_HEADER_GUARD

typedef struct ROSE_Vec2 {
	float x, y;
} ROSE_Vec2;

typedef struct ROSE_Vec3 {
	float x, y, z;
} ROSE_Vec3;

typedef struct ROSE_Quat {
	float w, x, y, z;
} ROSE_Quat;

typedef struct ROSE_Matrix {
	float m[16];
} ROSE_Matrix;

#define ROSE_VEC2_ZERO ((ROSE_Vec2) {  0.0f,  0.0f })
#define ROSE_VEC2_XPOS ((ROSE_Vec2) {  1.0f,  0.0f })
#define ROSE_VEC2_XNEG ((ROSE_Vec2) { -1.0f,  0.0f })
#define ROSE_VEC2_YPOS ((ROSE_Vec2) {  0.0f,  1.0f })
#define ROSE_VEC2_YNEG ((ROSE_Vec2) {  0.0f, -1.0f })

#define ROSE_VEC3_ZERO ((ROSE_Vec3) {  0.0f,  0.0f,  0.0f })
#define ROSE_VEC3_XPOS ((ROSE_Vec3) {  1.0f,  0.0f,  0.0f })
#define ROSE_VEC3_XNEG ((ROSE_Vec3) { -1.0f,  0.0f,  0.0f })
#define ROSE_VEC3_YPOS ((ROSE_Vec3) {  0.0f,  1.0f,  0.0f })
#define ROSE_VEC3_YNEG ((ROSE_Vec3) {  0.0f, -1.0f,  0.0f })
#define ROSE_VEC3_ZPOS ((ROSE_Vec3) {  0.0f,  0.0f,  1.0f })
#define ROSE_VEC3_ZNEG ((ROSE_Vec3) {  0.0f,  0.0f, -1.0f })

#define ROSE_QUAT_IDENTITY \
((ROSE_Quat) { 1.0f, 0.0f, 0.0f, 0.0f })

#define ROSE_MATRIX_IDENTITY \
((ROSE_Matrix) {{            \
	1.0f, 0.0f, 0.0f, 0.0f,  \
	0.0f, 1.0f, 0.0f, 0.0f,  \
	0.0f, 0.0f, 1.0f, 0.0f,  \
	0.0f, 0.0f, 0.0f, 1.0f   \
}})

float ROSE_Clamp(float value, float min, float max);
float ROSE_Lerp(float begin, float end, float t);

ROSE_Vec2 ROSE_Vec2Add(ROSE_Vec2 vec_a, ROSE_Vec2 vec_b);
ROSE_Vec2 ROSE_Vec2Sub(ROSE_Vec2 vec_a, ROSE_Vec2 vec_b);
ROSE_Vec2 ROSE_Vec2Scale(ROSE_Vec2 vec, float scale);
ROSE_Vec2 ROSE_Vec2Lerp(ROSE_Vec2 begin, ROSE_Vec2 end, float t);
float     ROSE_Vec2Dot(ROSE_Vec2 vec_a, ROSE_Vec2 vec_b);
float     ROSE_Vec2Cross(ROSE_Vec2 vec_a, ROSE_Vec2 vec_b);
ROSE_Vec2 ROSE_Vec2Normalize(ROSE_Vec2 vec);
float     ROSE_Vec2LengthSquared(ROSE_Vec2 vec);
float     ROSE_Vec2Length(ROSE_Vec2 vec);
float     ROSE_Vec2Distance(ROSE_Vec2 vec_a, ROSE_Vec2 vec_b);

ROSE_Vec3 ROSE_Vec3Add(ROSE_Vec3 vec_a, ROSE_Vec3 vec_b);
ROSE_Vec3 ROSE_Vec3Sub(ROSE_Vec3 vec_a, ROSE_Vec3 vec_b);
ROSE_Vec3 ROSE_Vec3Scale(ROSE_Vec3 vec, float scale);
ROSE_Vec3 ROSE_Vec3Lerp(ROSE_Vec3 begin, ROSE_Vec3 end, float t);
float     ROSE_Vec3Dot(ROSE_Vec3 vec_a, ROSE_Vec3 vec_b);
ROSE_Vec3 ROSE_Vec3Cross(ROSE_Vec3 vec_a, ROSE_Vec3 vec_b);
ROSE_Vec3 ROSE_Vec3Normalize(ROSE_Vec3 vec);
float     ROSE_Vec3LengthSquared(ROSE_Vec3 vec);
float     ROSE_Vec3Length(ROSE_Vec3 vec);
float     ROSE_Vec3Distance(ROSE_Vec3 vec_a, ROSE_Vec3 vec_b);

float     ROSE_QuatDot(ROSE_Quat quat_a, ROSE_Quat quat_b);
ROSE_Quat ROSE_QuatNegate(ROSE_Quat quat);
ROSE_Quat ROSE_QuatConjugate(ROSE_Quat quat);
ROSE_Quat ROSE_QuatMult(ROSE_Quat quat_a, ROSE_Quat quat_b);
ROSE_Quat ROSE_QuatSLerp(ROSE_Quat begin, ROSE_Quat end, float t);
ROSE_Quat ROSE_QuatNormalize(ROSE_Quat quat);
ROSE_Vec3 ROSE_QuatRotateVector(ROSE_Quat quat, ROSE_Vec3 vec);
ROSE_Quat ROSE_QuatRotate(ROSE_Quat quat, ROSE_Vec3 axis, float degrees);
ROSE_Vec3 ROSE_QuatForward(ROSE_Quat quat);
ROSE_Vec3 ROSE_QuatRight(ROSE_Quat quat);
ROSE_Vec3 ROSE_QuatUp(ROSE_Quat quat);

ROSE_Matrix ROSE_MatrixMult(ROSE_Matrix a, ROSE_Matrix b);
ROSE_Matrix ROSE_MatrixModel(ROSE_Vec3 position, ROSE_Quat orientation, float scale);
ROSE_Matrix ROSE_MatrixView(ROSE_Vec3 position, ROSE_Quat orientation);
ROSE_Matrix ROSE_MatrixProject(float fov_degrees, float aspect, float near_plane, float far_plane);
ROSE_Matrix ROSE_MatrixMVP(ROSE_Matrix model, ROSE_Matrix view, ROSE_Matrix project);

#endif /* ROSE_MATH_HEADER_GUARD */

