#pragma once

#include <unordered_map> 
#include <memory> 

#include "../platform/Window.hpp"

#include "Camera.hpp"
#include "Vulkan.hpp"
#include "ImageManager.hpp"
#include "Image.hpp"

#include "api_abstration/Device.hpp"

namespace daxa {

	struct RenderingRessources {
		vkh::DescriptorSetLayoutCache descLayoutCache;
		std::unordered_map<std::string_view, vk::UniqueRenderPass> renderPasses;
		std::unordered_map<std::string_view, vkh::Pipeline> pipelines;
		std::unordered_map<std::string_view, vkh::DescriptorSetAllocator> descSetAllocators;
		std::unordered_map<std::string_view, vk::UniqueSampler> samplers;
		std::unordered_map<std::string_view, gpu::ImageHandle> images;
		std::unordered_map<std::string_view, SimpleMesh> meshes;
	};

	class Renderer {
	public:
		Renderer(std::shared_ptr<Window> win) 
			: window{ std::move(win) }
			, device{ gpu::Device::createNewDevice() }
			, ressources{ std::make_shared<RenderingRessources>(RenderingRessources{.descLayoutCache = {device.getDevice()}}) }
		{ }

		void draw() {

			device.nextFrameContext();
		}

		FPSCamera camera;
		std::shared_ptr<Window> window{ nullptr };
		gpu::Device device;
		std::shared_ptr<RenderingRessources> ressources;
	private:
	};
}
