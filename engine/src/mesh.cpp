
#include "mesh.h"

#include <tiny_gltf.h>
#include <iostream>

using namespace engine;

Mesh::Mesh(std::string gltf_file)
{
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string error;

	bool success = loader.LoadASCIIFromFile(&model, &error, gltf_file);

	if(!error.empty())
		throw std::runtime_error(error);

	if(!success)
		throw std::runtime_error("Failed to load glTF file.");

	if(model.meshes.empty())
		throw std::runtime_error("No meshes in glTF file.");

	auto mesh = model.meshes[0];
	auto primitive = mesh.primitives[0];

	auto position_accessor = model.accessors[primitive.attributes["POSITION"]];
	auto position_buffer_view = model.bufferViews[position_accessor.bufferView];
	auto position_buffer = model.buffers[position_buffer_view.buffer];

	auto uv_accessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
	auto uv_buffer_view = model.bufferViews[uv_accessor.bufferView];
	auto uv_buffer = model.buffers[uv_buffer_view.buffer];

	vertices.resize(position_accessor.count);

	for(size_t i=0; i<vertices.size(); i++)
	{
		const auto *position_data = position_buffer.data.data() + position_buffer_view.byteOffset
									+ (position_buffer_view.byteStride + sizeof(float) * 3) * i
									+ position_accessor.byteOffset;

		memcpy(&vertices[i].pos, position_data, sizeof(float) * 3);
		vertices[i].pos *= 4.0f;


		const auto *uv_data = uv_buffer.data.data() + uv_buffer_view.byteOffset
							  + (uv_buffer_view.byteStride + sizeof(float) * 2) * i
							  + uv_accessor.byteOffset;

		memcpy(&vertices[i].uv, uv_data, sizeof(float) * 2);
	}


	auto index_accessor = model.accessors[primitive.indices];
	auto index_buffer_view = model.bufferViews[index_accessor.bufferView];
	auto index_buffer = model.buffers[index_buffer_view.buffer];

	indices.resize(index_accessor.count);

	for(size_t i=0; i<indices.size(); i++)
	{
		const auto *data = index_buffer.data.data() + index_buffer_view.byteOffset
						   + (index_buffer_view.byteStride + sizeof(uint16_t)) * i
						   + index_accessor.byteOffset;

		memcpy(&indices[i], data, sizeof(uint16_t));
	}
}

Mesh::~Mesh()
{

}
