
cbuffer cbDisturb
{
	float	gDisturbMag;
	int2	gDisturbIndex;
};

RWTexture2D<float> gOutput : register(u0);

[numthreads(1, 1, 1)]
void main( uint3 groupThreadID : SV_GroupThreadID, uint3 DTid : SV_DispatchThreadID )
{
	int x = gDisturbIndex.x;
	int y = gDisturbIndex.y;

	float halfMag = 0.5f*gDisturbMag;

	gOutput[int2(x, y)] += gDisturbMag;
	gOutput[int2(x + 1, y)] += halfMag;
	gOutput[int2(x - 1, y)] += halfMag;
	gOutput[int2(x, y + 1)] += halfMag;
	gOutput[int2(x, y - 1)] += halfMag;
}