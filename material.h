#pragma once
#include "shader.h"
#include "texture.h"
class material
{
public:
	~material();

	vertex_shader& vertex;
	pixel_shader& pixel;
	texture& texture_data;

	friend material make_material(vertex_shader& vertex, pixel_shader& pixel, texture& texture);
private:
	material(vertex_shader& vertex, pixel_shader& pixel, texture& texture);
};

