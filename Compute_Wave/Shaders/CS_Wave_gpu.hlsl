

cbuffer cbUpdateSettings
{
	float gWaveConstant0;
	float gWaveConstant1;
	float gWaveConstant2;
};

RWTexture2D<float> gPrevSolInput : register(u0);
RWTexture2D<float> gCurrSolInput : register(u1);
RWTexture2D<float> gOutput : register(u2);

[numthreads(16, 16, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	int x = DTid.x;
	int y = DTid.y;

	gOutput[int2(x, y)] =
		gWaveConstant0 * gPrevSolInput[int2(x, y)].r +
		gWaveConstant1 * gCurrSolInput[int2(x, y)].r +
		gWaveConstant2 * (
			gCurrSolInput[int2(x, y + 1)].r +
			gCurrSolInput[int2(x, y - 1)].r +
			gCurrSolInput[int2(x + 1, y)].r +
			gCurrSolInput[int2(x - 1, y)].r);
}