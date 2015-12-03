struct VertexShaderInput
{ 
	float3 startPosition	: POSITION;
	float3 velocity			: TEXCOORD0;
	float4 startColor		: COLOR0;
	float4 endColor			: COLOR1;
	float startRotation		: TEXCOORD1;
};

struct VertexToGeometry
{
	float3 position			: POSITION;
	float3 velocity			: TEXCOORD0;
	float4 startColor		: COLOR0;
	float4 endColor			: COLOR1;
	float startRotation		: TEXCOORD1;
};

VertexToGeometry main( VertexShaderInput input )
{
	// Set up output struct
	VertexToGeometry output;
	output.position = input.startPosition;
	output.velocity = input.velocity;
	output.startColor = input.startColor;
	output.endColor = input.endColor;
	output.startRotation = input.startRotation;
	return output;
}