struct VertexShaderInput
{ 

	float3 position		: POSITION;
	float2 texcoord		: TEXCOORD;
	float3 normal		: NORMAL;
};

struct VertexToGeometry
{

};

VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToGeometry output;
	

	return output;
}