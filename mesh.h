#pragma once
#include <vector>
#include <memory>
#include <DirectXMath.h>
#include "shader.h"
#include "vertex.h"
#include "dxi_ptr.h"


template <typename T, int Type>
struct d3d_buffer {
	int len;
	D3D11_SUBRESOURCE_DATA info;
	dxi_ptr<ID3D11Buffer> buffer;
	friend option<d3d_buffer> make_buffer(std::unique_ptr<T[]> data, int len, ID3D11Device& device) {
		d3d_buffer ret;
		ret.len = len;
		D3D11_BUFFER_DESC vbd;
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(T) * len;
		vbd.BindFlags = Type;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;
		vbd.StructureByteStride = 0;

		ret.info.pSysMem = data.release();

		// Actually create the buffer with the initial data
		// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
		auto result = device.CreateBuffer(&vbd, &ret.info, &ret.buffer);
		if (result != S_OK) {
			return None<d3d_buffer>();
		}
		return Some(ret);
	}
private:
	d3d_buffer() {
		
	}

};

class mesh
{
public:
	typedef d3d_buffer<Vertex, D3D11_BIND_VERTEX_BUFFER> vertex_buffer;
	typedef d3d_buffer<UINT, D3D11_BIND_INDEX_BUFFER> index_buffer;
	vertex_buffer vertices;
	index_buffer indices;
	friend option<mesh> load_mesh_from_file(const char * filename, ID3D11Device& device);
private:
	mesh(vertex_buffer, index_buffer);
};


template <class T>
std::unique_ptr<T[]> copy_to_owned_shallow(const std::vector<T>& input) {
	auto ret = std::unique_ptr<T[]>(new T[input.size()]);

	memcpy(ret.get(), input.data(), sizeof(T) * input.size());

	return ret;
}
