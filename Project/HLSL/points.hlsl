
/*
 *   HLSL Shader (Points)
 */

cbuffer point_constants : register(b0)
{
	float2 screen_size;
	float  diameter;
};

struct gs_vertex
{
	float4 position : SV_POSITION;
	float2 uv       : TEXCOORD;
	float4 color    : COLOR;
};

[maxvertexcount(4)]
void gs_main(point gs_vertex input[1], inout TriangleStream<gs_vertex> stream)
{
	static const float2 corners[4] = {
		float2(-1.0,  1.0), // top-left
		float2( 1.0,  1.0), // top-right
		float2(-1.0, -1.0), // bottom-left
		float2( 1.0, -1.0), // bottom-right
	};

	gs_vertex v = input[0];
	float4 center = v.position;

	// half the diameter in NDC is (diameter / 2) * (2 / screen_size) = diameter / screen_size
	// multiplied by w so the perspective divide cancels it (constant pixel size in 3D)
	float2 half_size = diameter / screen_size * center.w;

	for (int i = 0; i < 4; i++) {
		v.position = center;
		v.position.xy += corners[i] * half_size;
		stream.Append(v);
	}
}
