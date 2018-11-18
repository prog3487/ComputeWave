
struct VertexIn_Vc
{
	float3 Pos   : SV_Position;
	float4 Color : COLOR;
};

struct VertexOut_Vc
{
	float4 PosH  : SV_Position;
	float4 Color : COLOR;
};

struct VertexIn
{
	float3 PosL		: SV_Position;
	float4 Color	: COLOR;
	//float3 NormalL	: NORMAL;
	float2 TexC		: TEXCOORD;
};

struct VertexOut
{
	float4 PosH		: SV_Position;
	//float3 PosW		: POSITION;
	//float3 NormalW	: NORMAL;
	//float2 TexC		: TEXTURE;
};