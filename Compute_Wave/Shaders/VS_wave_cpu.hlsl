
#include "Structures.hlsli"

cbuffer constants
{
	float4x4 gWorldViewProj;
};

VertexOut_Vc main(VertexIn_Vc vin)
{
	VertexOut_Vc vout;
	vout.PosH = mul(float4(vin.Pos, 1.0f), gWorldViewProj);
	vout.Color = vin.Color;
	return vout;
}