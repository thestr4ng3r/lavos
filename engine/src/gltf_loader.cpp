
#include "material.h"
#include "gltf_loader.h"

#include <tiny_gltf.h>

using namespace engine;

GLTF::GLTF(Renderer *renderer, std::string filename)
	: renderer(renderer)
{
	auto slash_pos = filename.find_last_of('/');
	root_path = filename.substr(0, slash_pos);

	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string error;

	bool success = loader.LoadASCIIFromFile(&model, &error, filename);

	if(!error.empty())
		throw std::runtime_error(error);

	if(!success)
		throw std::runtime_error("Failed to load glTF file.");

	LoadGLTF(model);
}

GLTF::~GLTF()
{
	for(auto mesh : meshes)
		delete mesh;
}

void GLTF::LoadGLTF(tinygltf::Model &model)
{
	LoadMeshes(model);
	LoadMaterialInstances(model);
}

bool GetSubParameter(const tinygltf::ParameterMap &params, std::string name, std::string subname, int &dst)
{
	auto it = params.find(name);
	if(it == params.end())
		return false;

	auto &values = it->second.json_double_value;
	auto it2 = values.find(subname);
	if(it2 == values.end())
		return false;

	dst = (int)it2->second;
	return true;
}

void GLTF::LoadMaterialInstances(tinygltf::Model &model)
{
	Material *material = renderer->GetMaterial();

	for(const auto &gltf_material : model.materials)
	{
		std::string tex;

		int index;
		if(GetSubParameter(gltf_material.values, "baseColorTexture", "index", index))
		{
			int image_index = model.textures[index].source;
			tex = root_path + "/" + model.images[image_index].uri;
			// TODO: load from image data, not uri
		}

		auto material_instance = new MaterialInstance(material, renderer->GetDescriptorPool(), tex);
		material_instances.push_back(material_instance);
	}
}

void GLTF::LoadMeshes(tinygltf::Model &model)
{
	for(auto &gltf_mesh : model.meshes)
	{
		for(auto &gltf_primitive : gltf_mesh.primitives)
		{
			auto position_accessor = model.accessors[gltf_primitive.attributes["POSITION"]];
			auto position_buffer_view = model.bufferViews[position_accessor.bufferView];
			auto position_buffer = model.buffers[position_buffer_view.buffer];

			auto uv_accessor = model.accessors[gltf_primitive.attributes["TEXCOORD_0"]];
			auto uv_buffer_view = model.bufferViews[uv_accessor.bufferView];
			auto uv_buffer = model.buffers[uv_buffer_view.buffer];

			auto *mesh = new Mesh();
			meshes.push_back(mesh);
			auto &vertices = mesh->vertices;
			auto &indices = mesh->indices;

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


			auto index_accessor = model.accessors[gltf_primitive.indices];
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
	}
}
