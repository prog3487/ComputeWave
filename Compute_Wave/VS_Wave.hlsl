
#include "Structures.hlsli"

#define DISPLACEMENT_MAP

cbuffer Parameters : register(b0)
{
	float4x4 gWorld;		// TODO ; BIND!
	float4x4 gViewProj;		// TODO ; BIND!
	float4x4 gTexTransform;	// TODO ; BIND!
	float4x4 gMatTransform;	// TODO ; BIND!		// What is this??
}

cbuffer Parameters1 : register(b1)
{
	float2 gDisplacementMapTexelSize;	// packoffset(c0)	// TODO ; BIND!
	float gGridSpatialStep;	// TODO ; BIND!
}

Texture2D<float> gDisplacementMap : register(t0);	// TODO ; BIND!

sampler gSamLinearWrap : register(s0);	// TODO ; BIND!
sampler gSamPointClamp : register(s1);	// TODO ; BIND!

VertexOut main(VertexIn vin)
{
	VertexOut vout;

#ifdef DISPLACEMENT_MAP
	// get height from texture
	vin.PosL.y += gDisplacementMap.SampleLevel(gSamLinearWrap, vin.TexC, 1.0f).r;

	// compute normal
	float du = gDisplacementMapTexelSize.x;
	float dv = gDisplacementMapTexelSize.y;
	float l = gDisplacementMap.SampleLevel(gSamPointClamp, vin.TexC - float2(du, 0.0f), 0.0f).r;
	float r = gDisplacementMap.SampleLevel(gSamPointClamp, vin.TexC + float2(du, 0.0f), 0.0f).r;
	float t = gDisplacementMap.SampleLevel(gSamPointClamp, vin.TexC - float2(0.0f, dv), 0.0f).r;
	float b = gDisplacementMap.SampleLevel(gSamPointClamp, vin.TexC + float2(0.0f, dv), 0.0f).r;

	vin.NormalL = normalize(float3( -r + l, 2.0f * gGridSpatialStep, b - t));
#endif

	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
	vout.PosW = posW.xyz;

	// skip inv-transpose
	vout.NormalW = mul(vin.NormalL, (float3x3)gWorld);

	// homogeneous coordinate
	vout.PosH = mul(posW, gViewProj);

	// tex-c
	float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
	vout.TexC = mul(texC, gMatTransform).xy;

	return vout;
}