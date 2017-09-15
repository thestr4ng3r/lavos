
#include "engine.h"
#include "material.h"
#include "asset_container.h"
#include "component/mesh_component.h"
#include "component/camera_component.h"

#include <tiny_gltf.h>
#include <iostream>

#include <glm/gtx/quaternion.hpp>

using namespace engine;

AssetContainer::AssetContainer(Engine *engine)
	: engine(engine)
{
}

AssetContainer::~AssetContainer()
{
	for(auto mesh : meshes)
		delete mesh;

	for(auto material_instance : material_instances)
		delete material_instance;

	if(descriptor_pool)
		engine->GetVkDevice().destroyDescriptorPool(descriptor_pool);
}



// ---------------------------------------
// glTF
// ---------------------------------------


static Image LoadImage(AssetContainer &container, tinygltf::Model &model, int index)
{
	auto gltf_image = model.images[index];

	vk::Format format;
	switch(gltf_image.component)
	{
		case 1:
			format = vk::Format::eR8Unorm;
			break;
		case 2:
			format = vk::Format::eR8G8Unorm;
			break;
		case 3:
			format = vk::Format::eR8G8B8Unorm;
			break;
		case 4:
			format = vk::Format::eR8G8B8A8Unorm;
			break;
		default:
			throw std::runtime_error("invalid component count for image.");
	}

	return Image::LoadFromPixelData(container.engine, format,
										   static_cast<uint32_t>(gltf_image.width),
										   static_cast<uint32_t>(gltf_image.height),
										   gltf_image.image.data());
}

static Texture LoadTexture(AssetContainer &container, tinygltf::Model &model, int index)
{
	auto gltf_texture = model.textures[index];

	auto device = container.engine->GetVkDevice();

	auto image = LoadImage(container, model, gltf_texture.source);

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

static Texture LoadSubParameterTexture(AssetContainer &container, tinygltf::Model &model, const tinygltf::ParameterMap &params, std::string name, std::string subname)
{
	int index;
	if(!GetSubParameter(params, name, subname, index))
		return nullptr;

	return LoadTexture(container, model, index);
}

static void LoadMaterialInstances(AssetContainer &container, Material *material, tinygltf::Model &model)
{
	for(const auto &gltf_material : model.materials)
	{
		auto material_instance = new MaterialInstance(material, container.descriptor_pool);
		material_instance->SetTexture(LoadSubParameterTexture(container, model, gltf_material.values, "baseColorTexture", "index"));
		material_instance->WriteDescriptorSet();
		container.material_instances.push_back(material_instance);
	}
}


static void LoadMeshes(AssetContainer &container, tinygltf::Model &model)
{
	for(auto &gltf_mesh : model.meshes)
	{
		auto *mesh = new Mesh(container.engine);
		container.meshes.push_back(mesh);
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
			primitive.material_instance = container.material_instances[gltf_primitive.material]; // TODO: gltf_primitive could have no material
			primitive.indices_offset = static_cast<uint32_t>(indices_base);
			primitive.indices_count = static_cast<uint32_t>(index_accessor.count);
			mesh->primitives.push_back(primitive);
		}

		mesh->CreateBuffers();
	}
}

#include "glm_stream.h"

static void LoadNode(AssetContainer &container, tinygltf::Model &model, Node *parent_node, int gltf_node_index)
{
	auto &gltf_node = model.nodes[gltf_node_index];
	auto current_node = new Node();
	parent_node->AddChild(current_node);

	for(int gltf_child_node_index : gltf_node.children)
	{
		LoadNode(container, model, current_node, gltf_child_node_index);
	}

	auto transform_component = new TransformComponent();

	if(gltf_node.translation.size() >= 3)
	{
		transform_component->translation = glm::vec3(gltf_node.translation[0],
													 gltf_node.translation[1],
													 gltf_node.translation[2]);
	}

	if(gltf_node.rotation.size() >= 4)
	{
		float x = static_cast<float>(gltf_node.rotation[0]);
		float y = static_cast<float>(gltf_node.rotation[1]);
		float z = static_cast<float>(gltf_node.rotation[2]);
		float w = static_cast<float>(gltf_node.rotation[3]);

		transform_component->rotation = glm::quat(w, x, y, z);

		glm::vec3 euler = glm::eulerAngles(transform_component->rotation);
	}

	if(gltf_node.scale.size() >= 3)
	{
		transform_component->scale = glm::vec3(gltf_node.scale[0],
											   gltf_node.scale[1],
											   gltf_node.scale[2]);
	}

	current_node->AddComponent(transform_component);


	if(gltf_node.mesh >= 0)
	{
		auto mesh = container.meshes[gltf_node.mesh];
		current_node->AddComponent(new MeshComponent(mesh));
	}

	if(gltf_node.camera >= 0)
	{
		auto gltf_camera = model.cameras[gltf_node.camera];
		auto camera = new CameraComponent();

		if(gltf_camera.type == "orthographic")
		{
			// TODO
		}
		else
		{
			camera->SetType(CameraComponent::Type::PERSPECTIVE);

			// TODO: parameters
		}

		current_node->AddComponent(camera);
	}

	// TODO: lights, ...
}

static void LoadScenes(AssetContainer &container, tinygltf::Model &model)
{
	container.scenes.resize(model.scenes.size());

	for(int i=0; i<model.scenes.size(); i++)
	{
		auto &gltf_scene = model.scenes[i];
		auto scene = new Scene();

		for(int gltf_node_index : gltf_scene.nodes)
			LoadNode(container, model, scene->GetRootNode(), gltf_node_index);

		container.scenes[i] = scene;
	}
}


static vk::DescriptorPool CreateDescriptorPoolForGLTF(Engine *engine, Material *material, tinygltf::Model &model)
{
	auto material_instances_count = model.materials.size();
	auto sizes = material->GetDescriptorPoolSizes();

	for(auto &size : sizes)
		size.descriptorCount *= material_instances_count;

	auto create_info = vk::DescriptorPoolCreateInfo()
		.setPoolSizeCount(static_cast<uint32_t>(sizes.size()))
		.setPPoolSizes(sizes.data())
		.setMaxSets(static_cast<uint32_t>(material_instances_count));

	return engine->GetVkDevice().createDescriptorPool(create_info);
}


static AssetContainer *LoadGLTF(Engine *engine, Material *material, tinygltf::Model &model)
{
	AssetContainer *container = new AssetContainer(engine);
	container->descriptor_pool = CreateDescriptorPoolForGLTF(engine, material, model);

	try
	{
		LoadMaterialInstances(*container, material, model);
		LoadMeshes(*container, model);
		LoadScenes(*container, model);
	}
	catch(std::exception e)
	{
		delete container;
		throw e;
	}

	return container;
}


AssetContainer *AssetContainer::LoadFromGLTF(Engine *engine, Material *material, std::string filename)
{
	tinygltf::TinyGLTF loader;
	tinygltf::Model model;
	std::string error;

	bool success = loader.LoadASCIIFromFile(&model, &error, filename);
	//bool success = loader.LoadBinaryFromFile(&model, &error, filename);

	if(!error.empty())
		throw std::runtime_error(error);

	if(!success)
		throw std::runtime_error("Failed to load glTF file.");

	return LoadGLTF(engine, material, model);
}
