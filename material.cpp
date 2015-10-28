#include "Material.h"


material::material(vertex_shader& vertex, pixel_shader& pixel, option<texture&> texture_data, option<geometry_shader&> geometry)
: vertex(vertex), pixel(pixel), texture_data(texture_data), geometry(geometry)
{
}


material::~material()
{
}


material make_material(vertex_shader& vertex, pixel_shader& pixel, texture& texture) {
	auto tex = SomeRef(texture);
	return material(vertex, pixel, tex, None<geometry_shader&>());
}
material make_material(vertex_shader& vertex, pixel_shader& pixel, geometry_shader& geometry) {
	return material(vertex, pixel, None<texture&>(), SomeRef(geometry));
}
material make_material(vertex_shader& vertex, pixel_shader& pixel) {
	return material(vertex, pixel, None<texture&>(), None<geometry_shader&>());
}