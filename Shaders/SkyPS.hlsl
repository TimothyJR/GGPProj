// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 uvw          : TEXCOORD0;
};

// Textures and such
TextureCube sky			: register(t0);
SamplerState trilinear	: register(s0);

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	return sky.Sample(trilinear, input.uvw);
}