#pragma once

#include "SimpleMath.h"

struct VertexWave
{
	DirectX::SimpleMath::Vector3 Pos;
	DirectX::SimpleMath::Vector4 Color;
};

struct VertexWave_GPU
{
	DirectX::SimpleMath::Vector3 Pos;
	DirectX::SimpleMath::Vector4 Color;
	DirectX::SimpleMath::Vector2 Tex;
};

struct ConstantBuffer_WaveCPU
{
	DirectX::SimpleMath::Matrix WorldViewProj;
};

struct CBuffer_WaveGPU_Frame
{
	DirectX::SimpleMath::Matrix World;
	DirectX::SimpleMath::Matrix ViewProj;
//	DirectX::SimpleMath::Matrix TexTransform;
//	DirectX::SimpleMath::Matrix MatTransform;
};


struct CBuffer_WaveConstants
{
	float wc0, wc1, wc2;
	float pad;
};

struct CBuffer_Disturb
{
	float DisturbMag;
	int32_t DisturbX, DisturbY;
	float pad;
};