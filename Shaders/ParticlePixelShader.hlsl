struct GeometryToPixel
{
	float4 position			: SV_POSITION;
	float4 startColor		: COLOR0;
};


float4 main(GeometryToPixel input ) : SV_TARGET
{
	// Set up output struct
	

	return input.startColor;
}