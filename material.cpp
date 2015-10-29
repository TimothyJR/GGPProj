#include "Material.h"


material::material(vertex_shader& vertex, pixel_shader& pixel, option<geometry_shader&> geometry)
: vertex(vertex), pixel(pixel), geometry(geometry)
{
}


material::~material()
{
}


material make_material(vertex_shader& vertex, pixel_shader& pixel, geometry_shader& geometry) {
	return material(vertex, pixel, SomeRef(geometry));
}
material make_material(vertex_shader& vertex, pixel_shader& pixel) {
	return material(vertex, pixel, None<geometry_shader&>());
}