
/*
 *   HLSL Shader (2D)
 */

cbuffer constants : register(b0)
{
	float2 screen_size;
	float2 offset;
	float4 tint;
	float  scale;
};

Texture2D    tex  : register(t0);
SamplerState samp : register(s0);

struct vs_input
{
	float2 position : POSITION;
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
	float2 pixel = input.position * scale + offset;
	float2 ndc = pixel / screen_size * 2.0 - 1.0;
	ndc.y = -ndc.y;

	ps_input output;
	output.position = float4(ndc, 0.0, 1.0);
	output.uv = input.uv;
	output.color = input.color * tint;
	return output;
}

float4 ps_main(ps_input input) : SV_TARGET
{
	return tex.Sample(samp, input.uv) * input.color;
}

