
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float2 texcoord		: TEXCOORD;
	float3 normal		: NORMAL;
	float4 posForShadow : TEXCOORD2;
};

struct DirectionLight
{
	float4 ambient;
	float4 diffuse;
	float3 normal;
};

// Constant Buffer
// - Allows us to define a buffer of individual variables 
//    which will (eventually) hold data from our C++ code
// - All non-pipeline variables that get their values from 
//    our C++ code must be defined inside a Constant Buffer
// - The name of the cbuffer itself is unimportant
cbuffer externalData : register(b0)
{
	DirectionLight light;
	DirectionLight other_light;
};

Texture2D res : register(t0);
Texture2D shadowMap	: register(t1);

SamplerState state : register(s0);
SamplerComparisonState shadowSampler : register(s1);

float4 calculate_light(float3 normal, DirectionLight light)
{
	float3 light_dir = normalize(-light.normal);
	float dir = saturate(dot(light_dir, normal));
	return light.ambient + mul(dir, light.diffuse);
}
// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	float4 surface = res.Sample(state, input.texcoord);

	// Shadows -----------------------------------------------------

	// Calculate this pixel's UV on the shadow map
	// This is where we'll sample to compare depths
	float2 shadowUV = input.posForShadow.xy / input.posForShadow.w * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;

	// Calculate this pixel's depth from the light
	float depthFromLight = input.posForShadow.z / input.posForShadow.w;

	// Sample the shadow map itself
	float shadowAdj = shadowMap.SampleCmpLevelZero(shadowSampler, shadowUV, depthFromLight);

	// how does surface light work?
	float4 surfaceLight = surface * calculate_light(input.normal, light) + surface * calculate_light(input.normal, other_light);
	return surfaceLight * shadowAdj; 
}
