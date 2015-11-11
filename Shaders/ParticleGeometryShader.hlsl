cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	float t;
	float duration;
	float start_speed;
	float end_speed;
	float start_size;
	float end_size;
};

struct VertexToGeometry
{
	float3 position			: POSITION;
	float3 velocity			: TEXCOORD0;
	float4 startColor		: COLOR0;
	float4 endColor			: COLOR1;
};

struct GeometryToPixel
{
	float4 position			: SV_POSITION;
	float4 startColor		: COLOR0;
};

[maxvertexcount(4)]
void main(point VertexToGeometry input[1], inout TriangleStream<GeometryToPixel> spriteStream)
{
	GeometryToPixel output;

	// Change over time
	float change_over_time = t / duration;


	float change_size_over_time =  change_over_time * (end_size - start_size);
	float4 change_color_over_time = change_over_time * (input[0].endColor - input[0].startColor);


	float3 acceleration = ((normalize(input[0].velocity) * end_speed) - (normalize(input[0].velocity) * start_speed)) / duration;

	float3 actualPos = input[0].position + (normalize(input[0].velocity) * start_speed) * t + 0.5 * acceleration * t * t;
	
	output.position = float4(actualPos, 1);
	output.position.x -= start_size + change_size_over_time;
	output.position.y -= start_size + change_size_over_time;
	output.position = mul(output.position, mul(mul(world, view), projection));
	output.startColor = input[0].startColor + change_color_over_time;
	spriteStream.Append(output);
	
	output.position = float4(actualPos, 1);
	output.position.x -= start_size + change_size_over_time;
	output.position.y += start_size + change_size_over_time;
	output.position = mul(output.position, (mul(mul(world, view), projection)));
	output.startColor = input[0].startColor + change_color_over_time;
	spriteStream.Append(output);

	output.position = float4(actualPos, 1);
	output.position.x += start_size + change_size_over_time;
	output.position.y -= start_size + change_size_over_time;
	output.position = mul(output.position, (mul(mul(world, view), projection)));
	output.startColor = input[0].startColor + change_color_over_time;
	spriteStream.Append(output);

	
	output.position = float4(actualPos, 1);
	output.position.x += start_size + change_size_over_time;
	output.position.y += start_size + change_size_over_time;
	output.position = mul(output.position, (mul(mul(world, view), projection)));
	output.startColor = input[0].startColor + change_color_over_time;
	spriteStream.Append(output);

}