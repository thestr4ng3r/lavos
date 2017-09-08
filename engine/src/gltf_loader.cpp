
#include "material.h"
#include "gltf_loader.h"

#include <tiny_gltf.h>
#include <iostream>

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
	//bool success = loader.LoadBinaryFromFile(&model, &error, filename);

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
	LoadMaterialInstances(model);
	LoadMeshes(model);
}


static Image LoadImage(GLTF *gltf, tinygltf::Model &model, int index)
{
	auto gltf_image = model.images[index];
	return Image::LoadFromPixelDataRGBA8UI(gltf->GetRenderer()->GetEngine(),
										   static_cast<uint32_t>(gltf_image.width),
										   static_cast<uint32_t>(gltf_image.height),
										   gltf_image.image.data());
}

static Texture LoadTexture(GLTF *gltf, tinygltf::Model &model, int index)
{
	auto gltf_texture = model.textures[index];

	auto device = gltf->GetRenderer()->GetEngine()->GetVkDevice();

	auto image = LoadImage(gltf, model, gltf_texture.source);

	if(image == nullptr)
		return nullptr;


	auto image_view_info = vk::ImageViewCreateInfo()
		.setImage(image.image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(vk::Format::eR8G8B8A8Unorm)
		.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

	auto image_view = device.createImageView(image_view_info);


	//auto gltf_sampler = model.samplers[gltf_texture.sampler];

	auto create_info = vk::SamplerCreateInfo()
		.setMagFilter(vk::Filter::eLinear)
		.setMinFilter(vk::Filter::eLinear)
		.setAddressModeU(vk::SamplerAddressMode::eRepeat)
		.setAddressModeV(vk::SamplerAddressMode::eRepeat)
		.setAddressModeW(vk::SamplerAddressMode::eRepeat)
		.setAnisotropyEnable(VK_TRUE)
		.setMaxAnisotropy(16)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setMipLodBias(0.0f)
		.setMinLod(0.0f)
		.setMaxLod(0.0f);

	auto sampler = device.createSampler(create_info);

	return Texture(image, image_view, sampler);
}


static bool GetSubParameter(const tinygltf::ParameterMap &params, std::string name, std::string subname, int &dst)
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

static Texture LoadSubParameterTexture(GLTF *gltf, tinygltf::Model &model, const tinygltf::ParameterMap &params, std::string name, std::string subname)
{
	int index;
	if(!GetSubParameter(params, name, subname, index))
		return nullptr;

	return LoadTexture(gltf, model, index);
}

void GLTF::LoadMaterialInstances(tinygltf::Model &model)
{
	Material *material = renderer->GetMaterialPipeline(0).material;

	for(const auto &gltf_material : model.materials)
	{
		auto material_instance = new MaterialInstance(material, renderer->GetDescriptorPool());
		material_instance->SetTexture(LoadSubParameterTexture(this, model, gltf_material.values, "baseColorTexture", "index"));
		material_instance->WriteDescriptorSet();
		material_instances.push_back(material_instance);
	}
}


void GLTF::LoadMeshes(tinygltf::Model &model)
{
	for(auto &gltf_mesh : model.meshes)
	{
		auto *mesh = new Mesh(renderer->GetEngine());
		meshes.push_back(mesh);
		auto &vertices = mesh->vertices;
		auto &indices = mesh->indices;

		for(auto &gltf_primitive : gltf_mesh.primitives)
		{
			auto &position_accessor = model.accessors[gltf_primitive.attributes["POSITION"]];
			auto &position_buffer_view = model.bufferViews[position_accessor.bufferView];
			auto &position_buffer = model.buffers[position_buffer_view.buffer];

			auto &uv_accessor = model.accessors[gltf_primitive.attributes["TEXCOORD_0"]];
			auto &uv_buffer_view = model.bufferViews[uv_accessor.bufferView];
			auto &uv_buffer = model.buffers[uv_buffer_view.buffer];

			size_t vertices_base = vertices.size();
			vertices.resize(vertices_base + position_accessor.count);
			for(size_t i=0; i<position_accessor.count; i++)
			{
				const auto *position_data = position_buffer.data.data() + position_buffer_view.byteOffset
											+ (position_buffer_view.byteStride + sizeof(float) * 3) * i
											+ position_accessor.byteOffset;

				memcpy(&vertices[vertices_base + i].pos, position_data, sizeof(float) * 3);
				//vertices[vertices_base + i].pos *= 4.0f;


				const auto *uv_data = uv_buffer.data.data() + uv_buffer_view.byteOffset
									  + (uv_buffer_view.byteStride + sizeof(float) * 2) * i
									  + uv_accessor.byteOffset;

				memcpy(&vertices[vertices_base + i].uv, uv_data, sizeof(float) * 2);
			}


			auto &index_accessor = model.accessors[gltf_primitive.indices];
			auto &index_buffer_view = model.bufferViews[index_accessor.bufferView];
			auto &index_buffer = model.buffers[index_buffer_view.buffer];

			size_t indices_base = indices.size();
			indices.resize(indices_base + index_accessor.count);

			if(index_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
			{
				for(size_t i=0; i<index_accessor.count; i++)
				{
					const auto *data = index_buffer.data.data() + index_buffer_view.byteOffset
									   + (index_buffer_view.byteStride + 1) * i
									   + index_accessor.byteOffset;

					indices[indices_base + i] = *data;
					indices[indices_base + i] += vertices_base;
				}
			}
			else if(index_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			{
				for(size_t i=0; i<index_accessor.count; i++)
				{
					const auto *data = index_buffer.data.data() + index_buffer_view.byteOffset
									   + (index_buffer_view.byteStride + sizeof(uint16_t)) * i
									   + index_accessor.byteOffset;

					memcpy(&indices[indices_base + i], data, sizeof(uint16_t));
					indices[indices_base + i] += vertices_base;
				}
			}



			Mesh::Primitive primitive;
			primitive.material_instance = material_instances[gltf_primitive.material]; // TODO: gltf_primitive could have no material
			primitive.indices_offset = static_cast<uint32_t>(indices_base);
			primitive.indices_count = static_cast<uint32_t>(index_accessor.count);
			mesh->primitives.push_back(primitive);
		}

		mesh->CreateBuffers();
	}
}