
/*
 *   HLSL Shader (3D)
 */

cbuffer constants : register(b0)
{
	row_major float4x4 mvp;
	float4 tint;
};

struct vs_input
{
	float3 position : POSITION;
	float4 color    : COLOR;
};

struct ps_input
{
	float4 position : SV_POSITION;
	float4 color    : COLOR;
};

ps_input vs_main(vs_input input)
{
	ps_input output;
	output.position = mul(float4(input.position, 1.0), mvp);
	output.color = input.color * tint;
	return output;
}

float4 ps_main(ps_input input) : SV_TARGET
{
	return input.color;
}

