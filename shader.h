#pragma once
#pragma comment(lib, "dxguid.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>

#include "window_class.h"
#include "map.h"
#include "option.h"
#include "dxi_ptr.h"

struct shader_variable
{
	unsigned int offset;
	unsigned int size;
	unsigned int constant_buffer_index;
};

struct constant_buffer
{
	unsigned int bind_index;
	ID3D11Buffer* buffer;
	std::unique_ptr<unsigned char[]> local_data_buffer;
};

typedef std::vector<D3D11_INPUT_ELEMENT_DESC> input_layout_vector;

option<input_layout_vector> generate_input_layout(const D3D11_SHADER_DESC& shader_description, ID3D11ShaderReflection& shader_reflection);

template <typename ShaderType, typename DXShaderType>
class shader {
public:
	shader(ID3D11Device* device, ID3D11DeviceContext* context)
		: device(device), device_context(context) {

	}

	shader(shader&& rhs)
		: device(rhs.device), device_context(rhs.device_context)
	{
		this->constant_buffers = std::move(rhs.constant_buffers);
		this->constant_buffer_count = std::move(rhs.constant_buffer_count);
		this->constant_buffer_table = std::move(rhs.constant_buffer_table);
		this->sampler_id_table = std::move(rhs.sampler_id_table);
		this->texture_id_table = std::move(rhs.texture_id_table);
		this->variable_table = std::move(rhs.variable_table);
		this->input_layout = std::move(rhs.input_layout);
		this->compiled_shader = std::move(rhs.compiled_shader);
	}

	shader& operator =(shader&& rhs)
	{
		this->constant_buffers = std::move(rhs.constant_buffers);
		this->constant_buffer_count = std::move(rhs.constant_buffer_count);
		this->constant_buffer_table = std::move(rhs.constant_buffer_table);
		this->sampler_id_table = std::move(rhs.sampler_id_table);
		this->texture_id_table = std::move(rhs.texture_id_table);
		this->variable_table = std::move(rhs.variable_table);
		this->input_layout = std::move(rhs.input_layout);
		this->compiled_shader = std::move(rhs.compiled_shader);

		this->device = rhs.device;
		this->device_context = rhs.device_context;
		return *this;
	}

	~shader()
	{}

	void activate(bool copy_data = true) 
	{
		if (copy_data) {
			this->copy_all_buffers();
		};

		// calls the appropriate implementation of set shader and constant buffers
		static_cast<ShaderType*>(this)->activate_with_buffers();
	}
	void copy_all_buffers() 
	{
		for (unsigned int i = 0; i < constant_buffer_count; i++)
		{
			this->device_context->UpdateSubresource(
				constant_buffers[i].buffer, 0, 0,
				constant_buffers[i].local_data_buffer.get(), 0, 0);
		}
	}

	template<typename T>
	bool set_data(const char* name, const T& data)
	{
		return this->unsafe_set_data(name, (void*)&data, sizeof(T));
	}

	friend void load_constant_buffers_into(const D3D11_SHADER_DESC& shader_description, ID3D11ShaderReflection& shader_reflection, shader& target) {
		// Handle bound resources (like shaders and samplers)
		unsigned int resource_count = shader_description.BoundResources;
		for (unsigned int r = 0; r < resource_count; r++) {
			// Get this resource's description
			D3D11_SHADER_INPUT_BIND_DESC resource_description;
			shader_reflection.GetResourceBindingDesc(r, &resource_description);

			switch (resource_description.Type) {
			case D3D_SIT_TEXTURE:
				target.texture_id_table.insert(resource_description.Name, resource_description.BindPoint);
				break;
			case D3D_SIT_SAMPLER:
				target.sampler_id_table.insert(resource_description.Name, resource_description.BindPoint);
				break;
			}
		}
	}
	friend void load_bound_resources_into(const D3D11_SHADER_DESC& shader_description, ID3D11ShaderReflection& shader_reflection, shader& target)
	{
		target.constant_buffer_count = shader_description.ConstantBuffers;
		target.constant_buffers = std::unique_ptr<constant_buffer[]>(new constant_buffer[target.constant_buffer_count]);

		for (unsigned int b = 0; b < target.constant_buffer_count; b++) {
			ID3D11ShaderReflectionConstantBuffer* reflected_constant_buffers = shader_reflection.GetConstantBufferByIndex(b);

			// Get the description of this buffer
			D3D11_SHADER_BUFFER_DESC constant_buffer_description;
			reflected_constant_buffers->GetDesc(&constant_buffer_description);

			// Get the description of the resource binding, so
			// we know exactly how it's bound in the shader
			D3D11_SHADER_INPUT_BIND_DESC bind_description;
			shader_reflection.GetResourceBindingDescByName(constant_buffer_description.Name, &bind_description);

			// Set up the buffer and put its pointer in the table
			target.constant_buffers[b].bind_index = bind_description.BindPoint;
			target.constant_buffer_table.insert(constant_buffer_description.Name, &target.constant_buffers[b]);

			// Create this constant buffer
			D3D11_BUFFER_DESC new_constant_buffer_description;
			new_constant_buffer_description.Usage = D3D11_USAGE_DEFAULT;
			new_constant_buffer_description.ByteWidth = constant_buffer_description.Size;
			new_constant_buffer_description.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			new_constant_buffer_description.CPUAccessFlags = 0;
			new_constant_buffer_description.MiscFlags = 0;
			new_constant_buffer_description.StructureByteStride = 0;
			target.device->CreateBuffer(&new_constant_buffer_description, 0, &target.constant_buffers[b].buffer);

			// Set up the data buffer for this constant buffer
			target.constant_buffers[b].local_data_buffer = std::unique_ptr<unsigned char[]>(new unsigned char[constant_buffer_description.Size]);
			ZeroMemory(target.constant_buffers[b].local_data_buffer.get(), constant_buffer_description.Size);

			// Loop through all variables in this buffer
			for (unsigned int v = 0; v < constant_buffer_description.Variables; v++) {
				// Get this variable
				ID3D11ShaderReflectionVariable* reflected_shader_variable = reflected_constant_buffers->GetVariableByIndex(v);

				// Get the description of the variable
				D3D11_SHADER_VARIABLE_DESC shader_variable_description;
				reflected_shader_variable->GetDesc(&shader_variable_description);

				// Create the variable struct
				shader_variable shader_variable_info;
				shader_variable_info.constant_buffer_index = b;
				shader_variable_info.offset = shader_variable_description.StartOffset;
				shader_variable_info.size = shader_variable_description.Size;

				// Add this variable to the table
				target.variable_table.insert(shader_variable_description.Name, shader_variable_info);
			}
		}
	}
	friend bool load_input_layout_into(const D3D11_SHADER_DESC& shader_description, ID3D11ShaderReflection& shader_reflection, ID3DBlob& shader_blob, shader& target)
	{
		// for all of the input descriptions we'll be creating from reflection
		auto input_layout_option = generate_input_layout(shader_description, shader_reflection);
		if (input_layout_option.is_none()) {
			return false;
		}
		auto input_layout_description = input_layout_option.unwrap();
		HRESULT input_layout_result = target.device->CreateInputLayout(
			&input_layout_description[0],
			input_layout_description.size(),
			shader_blob.GetBufferPointer(),
			shader_blob.GetBufferSize(),
			&target.input_layout);
		return true;
	}

protected:
	bool unsafe_set_data(const char* name, const void* data, unsigned int size)
	{
		auto variable_data = this->variable_table.at(name);
		if (variable_data == nullptr) {
			return false;
		}
		memcpy(
			this->constant_buffers[variable_data->constant_buffer_index].local_data_buffer.get() + variable_data->offset,
			data,
			size);

		return true;
	};

	ID3D11Device* device;
	ID3D11DeviceContext* device_context;

	unsigned int constant_buffer_count;

	std::unique_ptr<constant_buffer[]> constant_buffers;
	map<constant_buffer*> constant_buffer_table;
	map<shader_variable> variable_table;
	map<unsigned int> texture_id_table;
	map<unsigned int> sampler_id_table;

	dxi_ptr<ID3D11InputLayout> input_layout;
	dxi_ptr<DXShaderType> compiled_shader;
};

class vertex_shader : public shader<vertex_shader, ID3D11VertexShader>
{
public:
	vertex_shader(dx_info render_target);

	vertex_shader(vertex_shader &&);
	vertex_shader& operator =(vertex_shader &&);

	// returns r-value refernce because the type cannot be copied
	friend option<vertex_shader> load_vertex_shader(const wchar_t* filename, dx_info info);

	bool set_shader_resource_view(const char* name, ID3D11ShaderResourceView* resource_view);
	bool set_sampler_state(const char* name, ID3D11SamplerState* sampler_state);


protected:
	friend class shader<vertex_shader, ID3D11VertexShader>;
	void activate_with_buffers();
};

class pixel_shader : public shader<pixel_shader, ID3D11PixelShader>
{
public:
	pixel_shader(dx_info render_target);

	pixel_shader(pixel_shader &&);
	pixel_shader& operator =(pixel_shader &&);

	// returns r-value refernce because the type cannot be copied
	friend option<pixel_shader> load_pixel_shader(const wchar_t* filename, dx_info info);

	bool set_shader_resource_view(const char* name, ID3D11ShaderResourceView* resource_view);
	bool set_sampler_state(const char* name, ID3D11SamplerState* sampler_state);

protected:
	friend class shader<pixel_shader, ID3D11PixelShader>;

	void activate_with_buffers();

};

class geometry_shader : public shader<geometry_shader, ID3D11GeometryShader>
{
public:
	geometry_shader(dx_info render_target);

	geometry_shader(geometry_shader &&);
	geometry_shader& operator =(geometry_shader &&);

	// returns r-value refernce because the type cannot be copied
	friend option<geometry_shader> load_geometry_shader(const wchar_t* filename, dx_info info);

	bool set_shader_resource_view(const char* name, ID3D11ShaderResourceView* resource_view);
	bool set_sampler_state(const char* name, ID3D11SamplerState* sampler_state);
	static void deactivate(ID3D11DeviceContext& device_context);
protected:
	friend class shader<geometry_shader, ID3D11GeometryShader>;
	void activate_with_buffers();

};

