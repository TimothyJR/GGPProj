#include "shader.h"


// uses reflection code from:
// https://takinginitiative.wordpress.com/2011/12/11/directx-1011-basic-shader-reflection-automatic-input-layout-creation/
option<input_layout_vector> generate_input_layout(const D3D11_SHADER_DESC& shader_description, ID3D11ShaderReflection& shader_reflection)
{
	input_layout_vector input_layout_description;
	input_layout_description.reserve(shader_description.InputParameters);
	for (unsigned int i = 0; i< shader_description.InputParameters; i++) {
		D3D11_SIGNATURE_PARAMETER_DESC parameter_description;
		shader_reflection.GetInputParameterDesc(i, &parameter_description);

		D3D11_INPUT_ELEMENT_DESC element_description;
		element_description.SemanticName = parameter_description.SemanticName;
		element_description.SemanticIndex = parameter_description.SemanticIndex;
		element_description.InputSlot = 0;
		element_description.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		element_description.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		element_description.InstanceDataStepRate = 0;

		if (parameter_description.Mask == 1) {
			switch (parameter_description.ComponentType) {
			case D3D_REGISTER_COMPONENT_UINT32:
				element_description.Format = DXGI_FORMAT_R32_UINT;
				break;
			case D3D_REGISTER_COMPONENT_SINT32:
				element_description.Format = DXGI_FORMAT_R32_SINT;
				break;
			case D3D_REGISTER_COMPONENT_FLOAT32:
				element_description.Format = DXGI_FORMAT_R32_FLOAT;
				break;
			default:
				return None<input_layout_vector>();
				break;
			}
		}
		else if (parameter_description.Mask <= 3) {
			switch (parameter_description.ComponentType) {
			case D3D_REGISTER_COMPONENT_UINT32:
				element_description.Format = DXGI_FORMAT_R32G32_UINT;
				break;
			case D3D_REGISTER_COMPONENT_SINT32:
				element_description.Format = DXGI_FORMAT_R32G32_SINT;
				break;
			case D3D_REGISTER_COMPONENT_FLOAT32:
				element_description.Format = DXGI_FORMAT_R32G32_FLOAT;
				break;
			default:
				return None<input_layout_vector>();
				break;
			}
		}
		else if (parameter_description.Mask <= 7) {
			switch (parameter_description.ComponentType) {
			case D3D_REGISTER_COMPONENT_UINT32:
				element_description.Format = DXGI_FORMAT_R32G32B32_UINT;
				break;
			case D3D_REGISTER_COMPONENT_SINT32:
				element_description.Format = DXGI_FORMAT_R32G32B32_SINT;
				break;
			case D3D_REGISTER_COMPONENT_FLOAT32:
				element_description.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				break;
			default:
				return None<input_layout_vector>();
				break;
			}
		}
		else if (parameter_description.Mask <= 15) {
			switch (parameter_description.ComponentType) {
			case D3D_REGISTER_COMPONENT_UINT32:
				element_description.Format = DXGI_FORMAT_R32G32B32A32_UINT;
				break;
			case D3D_REGISTER_COMPONENT_SINT32:
				element_description.Format = DXGI_FORMAT_R32G32B32A32_SINT;
				break;
			case D3D_REGISTER_COMPONENT_FLOAT32:
				element_description.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				break;
			default:
				return None<input_layout_vector>();
				break;
			}
		}

		input_layout_description.push_back(element_description);
	}
	return Some(std::move(input_layout_description));
}

vertex_shader::vertex_shader(dx_info render_target)
	: shader<vertex_shader, ID3D11VertexShader>(render_target.device, render_target.device_context)
{
}

vertex_shader::vertex_shader(vertex_shader&& rhs)
	: shader<vertex_shader, ID3D11VertexShader>(std::move(rhs))
{
}

vertex_shader& vertex_shader::operator=(vertex_shader&& rhs)
{
	shader<vertex_shader, ID3D11VertexShader>::operator=(std::move(rhs));
	return *this;
}

bool vertex_shader::set_shader_resource_view(const char* name, ID3D11ShaderResourceView* view)
{
	auto texture_bind_id = this->texture_id_table.at(name);;
	if (texture_bind_id == nullptr)
		return false;
	this->device_context->VSSetShaderResources(*texture_bind_id, 1, &view);
	return true;
}

bool vertex_shader::set_sampler_state(const char* name, ID3D11SamplerState* state)
{
	auto sampler_bind_id = this->sampler_id_table.at(name);;
	if (sampler_bind_id == nullptr)
		return false;
	this->device_context->VSSetSamplers(*sampler_bind_id, 1, &state);
	return true;
}

void vertex_shader::activate_with_buffers()
{
	// Set the shader and input layout
	this->device_context->IASetInputLayout(this->input_layout);
	this->device_context->VSSetShader(this->compiled_shader, 0, 0);

	// Set the constant buffers
	for (unsigned int i = 0; i < this->constant_buffer_count; i++)
	{
		this->device_context->VSSetConstantBuffers(
			this->constant_buffers[i].bind_index,
			1,
			&this->constant_buffers[i].buffer);
	}
}


option<vertex_shader> load_vertex_shader(const wchar_t * filename, dx_info info)
{
	vertex_shader ret(info);

	dxi_ptr<ID3DBlob> shader_blob;
	HRESULT hr = D3DReadFileToBlob(filename, &shader_blob);
	if (hr != S_OK)
	{
		return None<vertex_shader>();
	}
	// Create the shader from the blob
	HRESULT result = info.device->CreateVertexShader(
		shader_blob->GetBufferPointer(),
		shader_blob->GetBufferSize(),
		0,
		&ret.compiled_shader);

	if (result != S_OK) {
		return None<vertex_shader>();
	}

	// Reflect shader info
	dxi_ptr<ID3D11ShaderReflection> shader_reflection;
	D3DReflect(
		shader_blob->GetBufferPointer(),
		shader_blob->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		(void**)&shader_reflection);

	// grab the shader info from the compiled shader
	// so we can fillout the input layout using reflection
	D3D11_SHADER_DESC shader_description;
	shader_reflection->GetDesc(&shader_description);

	load_input_layout_into(shader_description, *shader_reflection, *shader_blob, ret);
	load_constant_buffers_into(shader_description, *shader_reflection, ret);
	load_bound_resources_into(shader_description, *shader_reflection, ret);

	return Some(std::move(ret));
}




pixel_shader::pixel_shader(dx_info render_target)
	: shader<pixel_shader, ID3D11PixelShader>(render_target.device, render_target.device_context)
{
}

pixel_shader::pixel_shader(pixel_shader && rhs)
	: shader<pixel_shader, ID3D11PixelShader>(std::move(rhs))
{
}

pixel_shader& pixel_shader::operator=(pixel_shader && rhs)
{
	shader<pixel_shader, ID3D11PixelShader>::operator=(std::move(rhs));
	return *this;
}


bool pixel_shader::set_shader_resource_view(const char* name, ID3D11ShaderResourceView* view)
{
	auto texture_bind_id = this->texture_id_table.at(name);;
	if (texture_bind_id == nullptr)
		return false;
	this->device_context->PSSetShaderResources(*texture_bind_id, 1, &view);
	return true;
}

bool pixel_shader::set_sampler_state(const char* name, ID3D11SamplerState* state)
{
	auto sampler_bind_id = this->sampler_id_table.at(name);;
	if (sampler_bind_id == nullptr)
		return false;
	this->device_context->PSSetSamplers(*sampler_bind_id, 1, &state);
	return true;
}

void pixel_shader::activate_with_buffers()
{
	// Set the shader and input layout
	this->device_context->PSSetShader(this->compiled_shader, 0, 0);

	// Set the constant buffers
	for (unsigned int i = 0; i < this->constant_buffer_count; i++)
	{
		this->device_context->PSSetConstantBuffers(
			this->constant_buffers[i].bind_index,
			1,
			&this->constant_buffers[i].buffer);
	}
}
option<pixel_shader> load_pixel_shader(const wchar_t * filename, dx_info info)
{
	pixel_shader ret(info);

	dxi_ptr<ID3DBlob> shader_blob;
	HRESULT hr = D3DReadFileToBlob(filename, &shader_blob);
	if (hr != S_OK)
	{
		return None<pixel_shader>();
	}
	// Create the shader from the blob
	HRESULT result = info.device->CreatePixelShader(
		shader_blob->GetBufferPointer(),
		shader_blob->GetBufferSize(),
		0,
		&ret.compiled_shader);

	if (result != S_OK) {
		return None<pixel_shader>();
	}

	// Reflect shader info
	dxi_ptr<ID3D11ShaderReflection> shader_reflection;
	D3DReflect(
		shader_blob->GetBufferPointer(),
		shader_blob->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		(void**)&shader_reflection);

	// grab the shader info from the compiled shader
	// so we can fillout the input layout using reflection
	D3D11_SHADER_DESC shader_description;
	shader_reflection->GetDesc(&shader_description);

	load_input_layout_into(shader_description, *shader_reflection, *shader_blob, ret);
	load_constant_buffers_into(shader_description, *shader_reflection, ret);
	load_bound_resources_into(shader_description, *shader_reflection, ret);

	return Some(std::move(ret));
}


geometry_shader::geometry_shader(dx_info render_target)
	: shader<geometry_shader, ID3D11GeometryShader>(render_target.device, render_target.device_context)
{
}

geometry_shader::geometry_shader(geometry_shader && rhs)
	: shader<geometry_shader, ID3D11GeometryShader>(std::move(rhs))
{
}

geometry_shader& geometry_shader::operator=(geometry_shader&& rhs)
{
	shader<geometry_shader, ID3D11GeometryShader>::operator=(std::move(rhs));
	return *this;
}

bool geometry_shader::set_shader_resource_view(const char* name, ID3D11ShaderResourceView* view)
{
	auto texture_bind_id = this->texture_id_table.at(name);;
	if (texture_bind_id == nullptr)
		return false;
	this->device_context->GSSetShaderResources(*texture_bind_id, 1, &view);
	return true;
}

bool geometry_shader::set_sampler_state(const char* name, ID3D11SamplerState* state)
{
	auto sampler_bind_id = this->sampler_id_table.at(name);;
	if (sampler_bind_id == nullptr)
		return false;
	this->device_context->GSSetSamplers(*sampler_bind_id, 1, &state);
	return true;
}

void geometry_shader::deactivate(ID3D11DeviceContext& device_context)
{
	device_context.GSSetShader(nullptr, 0, 0);
}

void geometry_shader::activate_with_buffers()
{
	// Set the shader and input layout
	this->device_context->GSSetShader(this->compiled_shader, 0, 0);

	// Set the constant buffers
	for (unsigned int i = 0; i < this->constant_buffer_count; i++)
	{
		this->device_context->GSSetConstantBuffers(
			this->constant_buffers[i].bind_index,
			1,
			&this->constant_buffers[i].buffer);
	}
}
option<geometry_shader> load_geometry_shader(const wchar_t * filename, dx_info info)
{
	geometry_shader ret(info);

	dxi_ptr<ID3DBlob> shader_blob;
	HRESULT hr = D3DReadFileToBlob(filename, &shader_blob);
	if (hr != S_OK)
	{
		return None<geometry_shader>();
	}
	// Create the shader from the blob
	HRESULT result = info.device->CreateGeometryShader(
		shader_blob->GetBufferPointer(),
		shader_blob->GetBufferSize(),
		0,
		&ret.compiled_shader);

	if (result != S_OK) {
		return None<geometry_shader>();
	}

	// Reflect shader info
	dxi_ptr<ID3D11ShaderReflection> shader_reflection;
	D3DReflect(
		shader_blob->GetBufferPointer(),
		shader_blob->GetBufferSize(),
		IID_ID3D11ShaderReflection,
		(void**)&shader_reflection);

	// grab the shader info from the compiled shader
	// so we can fillout the input layout using reflection
	D3D11_SHADER_DESC shader_description;
	shader_reflection->GetDesc(&shader_description);

	load_input_layout_into(shader_description, *shader_reflection, *shader_blob, ret);
	load_bound_resources_into(shader_description, *shader_reflection, ret);
	load_constant_buffers_into(shader_description, *shader_reflection, ret);

	return Some(std::move(ret));
}
