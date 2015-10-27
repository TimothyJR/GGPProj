#include "Material.h"


material::material(vertex_shader& vertex, pixel_shader& pixel, texture& texture_data)
: vertex(vertex), pixel(pixel), texture_data(texture_data)
{
}


material::~material()
{
}


material make_material(vertex_shader& vertex, pixel_shader& pixel, texture& texture) {
	return material(vertex, pixel, texture);
}