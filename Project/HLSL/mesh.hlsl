
/*
 *   HLSL Shader (3D)
 */

cbuffer constants : register(b0)
{
	row_major float4x4 mvp;
	float4 tint;
};

Texture2D    tex  : register(t0);
SamplerState samp : register(s0);

struct vs_input
{
	float3 position : POSITION;
	float2 uv       : TEXCOORD;
	float4 color    : COLOR;
};

struct ps_input
{
	float4 position : SV_POSITION;
	float2 uv       : TEXCOORD;
	float4 color    : COLOR;
};

ps_input vs_main(vs_input input)
{
	ps_input output;
	output.position = mul(float4(input.position, 1.0), mvp);
	output.uv = input.uv;
	output.color = input.color * tint;
	return output;
}

float4 ps_main(ps_input input) : SV_TARGET
{
	return tex.Sample(samp, input.uv) * input.color;
}

