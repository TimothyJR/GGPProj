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
	float scale = start_size + change_size_over_time;

	float4 change_color_over_time = change_over_time * (input[0].endColor - input[0].startColor);
	
	float3 acceleration = ((normalize(input[0].velocity) * end_speed) - (normalize(input[0].velocity) * start_speed)) / duration;

	float angle = input[0].startRotation + change_over_time * (input[0].startRotation);
	float cs = cos(angle);
	float sn = sin(angle);

	float3 actual_pos = input[0].position + (normalize(input[0].velocity) * start_speed) * t + 0.5f * acceleration * t * t;


	// Billboarding
	float4 proj_pos = mul(float4(actual_pos, 1.0f), mul(mul(world, view), projection));
	output.position = proj_pos;
	float depth_change = output.position.z / output.position.w;

	float particle_default_scale = 0.1f;
	
	float2 offsets[4];
	offsets[0] = float2(-particle_default_scale * cs + particle_default_scale * sn, -particle_default_scale * sn - particle_default_scale * cs);
	offsets[1] = float2(-particle_default_scale * cs - particle_default_scale * sn, -particle_default_scale * sn + particle_default_scale * cs);
	offsets[2] = float2(particle_default_scale * cs + particle_default_scale * sn, particle_default_scale * sn - particle_default_scale * cs);
	offsets[3] = float2(particle_default_scale * cs + particle_default_scale * sn, particle_default_scale * sn + particle_default_scale * cs);

	output.position.xy = proj_pos.xy + offsets[0] * depth_change * (start_size + change_size_over_time);
	output.startColor = input[0].startColor + change_color_over_time;
	output.texcoord = float2(0,0);
	spriteStream.Append(output);

	output.position.xy = proj_pos.xy + offsets[1] * depth_change * (start_size + change_size_over_time);
	output.startColor = input[0].startColor + change_color_over_time;
	output.texcoord = float2(0,1);
	spriteStream.Append(output);

	output.position.xy = proj_pos.xy + offsets[2] * depth_change * (start_size + change_size_over_time);
	output.startColor = input[0].startColor + change_color_over_time;
	output.texcoord = float2(1,0);
	spriteStream.Append(output);

	output.position.xy = proj_pos.xy + offsets[3] * depth_change * (start_size + change_size_over_time);
	output.startColor = input[0].startColor + change_color_over_time;
	output.texcoord = float2(1,1);


	spriteStream.Append(output);

}