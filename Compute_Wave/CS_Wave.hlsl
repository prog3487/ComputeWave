
RWTexture2D<float> gPrevSolInput : register(u0);
RWTexture2D<float> gCurrSolInput : register(u1);
RWTexture2D<float> gOutput : register(u2);

// 1. Compute height of wave

// 2. Create new wave

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
}