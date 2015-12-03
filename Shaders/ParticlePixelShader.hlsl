struct GeometryToPixel
{
	float4 position			: SV_POSITION;
	float4 startColor		: COLOR0;
	float2 texcoord			: TEXCOORD;
};

Texture2D res : register(t0);
SamplerState state : register(s0);

float4 main(GeometryToPixel input ) : SV_TARGET
{
	// Set up output struct
	float4 surface = res.Sample(state, input.texcoord);

	//return float4(input.startColor.rgb * surface.rgb, surface.a);
	return input.startColor * surface;
}