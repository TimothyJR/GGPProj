cbuffer externalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
	float t;
};

struct VertexToGeometry
{
	float3 position			: POSITION;
	float3 velocity			: TEXCOORD0;
	float4 startColor		: COLOR0;
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
	float3 actualPos = input[0].position + t * input[0].velocity;
	
	
	output.position = float4(actualPos, 1);
	output.position.x -= 0.5;
	output.position.y -= 0.5;
	output.position = mul(output.position, mul(mul(world, view), projection));
	output.startColor = input[0].startColor;
	spriteStream.Append(output);
	
	output.position = float4(actualPos, 1);
	output.position.x -= 0.5;
	output.position.y += 0.5;
	output.position = mul(output.position, (mul(mul(world, view), projection)));
	output.startColor = input[0].startColor;
	spriteStream.Append(output);

	output.position = float4(actualPos, 1);
	output.position.x += 0.5;
	output.position.y -= 0.5;
	output.position = mul(output.position, (mul(mul(world, view), projection)));
	output.startColor = input[0].startColor;
	spriteStream.Append(output);

	
	output.position = float4(actualPos, 1);
	output.position.x += 0.5;
	output.position.y += 0.5;
	output.position = mul(output.position, (mul(mul(world, view), projection)));
	output.startColor = input[0].startColor;
	spriteStream.Append(output);

}