
/*
 *   HLSL Shader
 */

cbuffer constants : register(b0)
{
    float2 screen_size;
};

struct vs_input
{
    float2 position : POSITION;
    float4 color    : COLOR;
};

struct ps_input
{
    float4 position : SV_POSITION;
    float4 color    : COLOR;
};

ps_input vs_main(vs_input input)
{
    float2 ndc = input.position / screen_size * 2.0 - 1.0;
    ndc.y = -ndc.y;
    ps_input output;
    output.position = float4(ndc, 0.0, 1.0);
    output.color = input.color;
    return output;
}

float4 ps_main(ps_input input) : SV_TARGET
{
    return input.color;
}

