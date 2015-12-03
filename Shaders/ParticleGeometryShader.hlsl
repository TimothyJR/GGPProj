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
	float rotation_change;
};

struct VertexToGeometry
{
	float3 position			: POSITION;
	float3 velocity			: TEXCOORD0;
	float4 startColor		: COLOR0;
	float4 endColor			: COLOR1;
	float startRotation		: TEXCOORD1;
};

struct GeometryToPixel
{
	float4 position			: SV_POSITION;
	float4 startColor		: COLOR0;
	float2 texcoord			: TEXCOORD;
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

	float angle = input[0].startRotation + change_over_time * (input[0].startRotation);
	float cs = cos(angle);
	float sn = sin(angle);

	float3 actualPos = input[0].position + (normalize(input[0].velocity) * start_speed) * t + 0.5 * acceleration * t * t;
	
	output.position = float4(actualPos, 1);
	output.position.x += -(start_size + change_size_over_time) * cs + (start_size + change_size_over_time) * sn;;
	output.position.y += -(start_size + change_size_over_time) * sn - (start_size + change_size_over_time) * cs;;
	output.position = mul(output.position, mul(mul(world, view), projection));
	output.startColor = input[0].startColor + change_color_over_time;
	output.texcoord = float2(0,0);
	spriteStream.Append(output);
	
	output.position = float4(actualPos, 1);
	output.position.x += -(start_size + change_size_over_time) * cs - (start_size + change_size_over_time) * sn;
	output.position.y += -(start_size + change_size_over_time) * sn + (start_size + change_size_over_time) * cs;
	output.position = mul(output.position, (mul(mul(world, view), projection)));
	output.startColor = input[0].startColor + change_color_over_time;
	output.texcoord = float2(0,1);
	spriteStream.Append(output);

	output.position = float4(actualPos, 1);
	output.position.x += (start_size + change_size_over_time) * cs + (start_size + change_size_over_time) * sn;
	output.position.y += (start_size + change_size_over_time) * sn - (start_size + change_size_over_time) * cs;
	output.position = mul(output.position, (mul(mul(world, view), projection)));
	output.startColor = input[0].startColor + change_color_over_time;
	output.texcoord = float2(1,0);
	spriteStream.Append(output);

	
	output.position = float4(actualPos, 1);
	output.position.x += (start_size + change_size_over_time) * cs - (start_size + change_size_over_time) * sn;
	output.position.y += (start_size + change_size_over_time) * sn + (start_size + change_size_over_time) * cs;
	output.position = mul(output.position, (mul(mul(world, view), projection)));
	output.startColor = input[0].startColor + change_color_over_time;
	output.texcoord = float2(1,1);
	spriteStream.Append(output);

}