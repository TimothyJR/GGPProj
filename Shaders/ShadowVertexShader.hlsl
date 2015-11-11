// Constant Buffer
cbuffer externalData : register(b0)
{
	matrix world; // from current entity
	matrix view; // from light
	matrix projection; // from light
};

struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 texcoord		: TEXCOORD;
	float3 normal		: NORMAL;
};


// The entry point (main method) for our vertex shader
float4 main(VertexShaderInput input) : SV_POSITION
{
	matrix worldViewProj = mul(mul(world, view), projection);
	return mul(float4(input.position, 1.0f), worldViewProj);
}