#pragma once
#include "shader.h"
#include "texture.h"
#include "option.h"
class material
{
public:
	~material();

	vertex_shader& vertex;
	pixel_shader& pixel;
	option<geometry_shader&> geometry;

	friend material make_material(vertex_shader& vertex, pixel_shader& pixel, geometry_shader& geometry);
	friend material make_material(vertex_shader& vertex, pixel_shader& pixel);
private:
	material(vertex_shader& vertex, pixel_shader& pixel, option<geometry_shader&> geometry);
};

