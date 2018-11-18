
#include "Structures.hlsli"

cbuffer Parameters : register(b0)
{
	float4x4 gWorld;
	float4x4 gViewProj;
}

Texture2D<float> gDisplacementMap : register(t0);

sampler gSamLinearWrap : register(s0);

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	// get height from texture
	vin.PosL.y += gDisplacementMap.SampleLevel(gSamLinearWrap, vin.TexC, 0.0f).r;

	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);

	// homogeneous coordinate
	vout.PosH = mul(posW, gViewProj);

	return vout;
}