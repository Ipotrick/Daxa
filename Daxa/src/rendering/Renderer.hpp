#pragma once

#include <unordered_map>
#include <memory> 

#include "../platform/Window.hpp"

#include "Camera.hpp"
#include "Vulkan.hpp"
#include "ImageManager.hpp"
#include "FrameContext.hpp"
#include "Image.hpp"

namespace daxa {

	struct RenderingRessources {
		GPUContext gpu;
		vkh::DescriptorSetLayoutCache descLayoutCache;
		std::unordered_map<std::string_view, vk::UniqueRenderPass> renderPasses;
		std::unordered_map<std::string_view, vkh::Pipeline> pipelines;
		std::unordered_map<std::string_view, vkh::DescriptorSetAllocator> descSetAllocators;
		std::unordered_map<std::string_view, vk::UniqueSampler> samplers;
		std::unordered_map<std::string_view, Image> images;
		std::unordered_map<std::string_view, SimpleMesh> meshes;
	};

	inline Image createDepthImage(GPUContext& gpu, vk::Extent2D size) {
		auto DEPTH_IMAGE_FORMAT = vk::Format::eD32Sfloat;

		return createImage2d(gpu, daxa::Image2dCreateInfo{
			.format = DEPTH_IMAGE_FORMAT,
			.size = size,
			.usage = vk::ImageUsageFlagBits::eDepthStencilAttachment,
			.viewImageAspectFlags = vk::ImageAspectFlagBits::eDepth,
		});
	}

	inline std::optional<vkh::Pipeline> createMeshPipeline(GPUContext& gpu, vk::RenderPass pass, vkh::DescriptorSetLayoutCache& descLayoutCache) {
		auto f = loadGLSLShaderToSpv("Daxa/shaders/mesh.frag").value();
		auto v = loadGLSLShaderToSpv("Daxa/shaders/mesh.vert").value();

		vkh::GraphicsPipelineBuilder pipelineBuilder(
			gpu.device,
			pass);

		pipelineBuilder
			.setVertexInput(Vertex::INFO.makePipelineVertexInputStateCreateInfo())
			.addShaderStage(&v, vk::ShaderStageFlagBits::eVertex)
			.addShaderStage(&f, vk::ShaderStageFlagBits::eFragment)
			.setDepthStencil(vk::PipelineDepthStencilStateCreateInfo{
				.depthTestEnable = VK_TRUE,
				.depthWriteEnable = VK_TRUE,
				.depthCompareOp = vk::CompareOp::eLess,
				.depthBoundsTestEnable = VK_FALSE,
				.stencilTestEnable = VK_FALSE,
				.minDepthBounds = 0.0f,
				.maxDepthBounds = 1.0f,
				})
				.setRasterization(vk::PipelineRasterizationStateCreateInfo{
					.cullMode = vk::CullModeFlagBits::eBack,
					.frontFace = vk::FrontFace::eCounterClockwise,
					.lineWidth = 1.0f,
					})
					.reflectSPVForDescriptors(descLayoutCache)
			.reflectSPVForPushConstants();

		return std::move(pipelineBuilder.build());
	}

	inline std::optional<vk::UniqueRenderPass> createMeshRenderpass(GPUContext& gpu, vk::Format surfaceFormat, vk::Format depthFormat) {
		return vkh::RenderPassBuilder{ gpu.device }
			.addAttachment(vk::AttachmentDescription{
				.format = surfaceFormat,
				.loadOp = vk::AttachmentLoadOp::eClear,
				.storeOp = vk::AttachmentStoreOp::eStore,
				.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
				.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
				.initialLayout = vk::ImageLayout::eUndefined,
				.finalLayout = vk::ImageLayout::ePresentSrcKHR,
				})
			.addAttachment(vk::AttachmentDescription{
				.format = depthFormat,
				.loadOp = vk::AttachmentLoadOp::eClear,
				.storeOp = vk::AttachmentStoreOp::eStore,
				.stencilLoadOp = vk::AttachmentLoadOp::eClear,
				.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
				.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal,
				})
			.build();
	}

	inline vk::UniqueFramebuffer createMeshFramebuffer(vk::RenderPass renderpass, vk::Extent2D size, vk::Device device, vk::ImageView surface, vk::ImageView depthImg) {
		vk::FramebufferCreateInfo framebufferCI{
			.renderPass = renderpass,
			.width = size.width,
			.height = size.height,
			.layers = 1,
		};
	
		std::array<vk::ImageView, 2> attachments{ surface, depthImg };
		framebufferCI.attachmentCount = (u32)attachments.size();
		framebufferCI.pAttachments = attachments.data();
		return device.createFramebufferUnique({ .renderPass = renderpass, .width = size.width, .height = size.height, .layers = 1, });
	}

	class Renderer {
	public:
		Renderer(std::shared_ptr<Window> win) 
			: window{ std::move(win) }
			, ressources{ .gpu = VulkanGlobals::getGlobalContext(), .descLayoutCache = {VulkanGlobals::getGlobalContext().device} }
		{
			this->framesInFlight = window->imagesInFlight;

			this->ressources.images["depth"] = createDepthImage(this->ressources.gpu, vk::Extent2D{
				.width	= window->getSize()[0],
				.height = window->getSize()[1]
			});

			this->ressources.renderPasses["main_pass"] = createMeshRenderpass(this->ressources.gpu, window->swapchainImageFormat, this->ressources.images["depth"].info.format).value();
			this->ressources.pipelines["main_pipeline"] = createMeshPipeline(this->ressources.gpu, *this->ressources.renderPasses["main_pass"], this->ressources.descLayoutCache).value();
			this->ressources.samplers["default"] = this->ressources.gpu.device.createSamplerUnique({
				.addressModeU = vk::SamplerAddressMode::eClampToEdge,
				.addressModeV = vk::SamplerAddressMode::eClampToEdge,
				.borderColor = vk::BorderColor::eFloatOpaqueWhite,
			});

			for (int i = 0; i < this->framesInFlight; i++) {
				this->frames.push_back({this->ressources.gpu});
			}
		}

		void draw() {
			auto& gpu = this->ressources.gpu;

			auto& frame = frames[frameIndex++ % frames.size()];
			frame.waitOnCompletion(gpu);
			frame.reset();

			auto [image, view, semaphore] = window->getNextImage();

			vk::CommandBuffer cmd = frame.commandBufferPool.getCommandBuffer();
			cmd.begin({});

			frame.temporaryFramebuffers.push_back(createMeshFramebuffer(
				*ressources.renderPasses["main_pass"],
				vk::Extent2D{ .width = window->getSize()[0], .height = window->getSize()[1] },
				gpu.device,
				view,
				*ressources.images["depth_img"].view
			));
			auto& framebuffer = *frame.temporaryFramebuffers.back();


		}

		FPSCamera camera;
		ImageManager imgMtx;
	private:
		std::shared_ptr<Window> window{ nullptr };

		RenderingRessources ressources;
		std::vector<FrameContext> frames;

		u32 framesInFlight{ 2 };
		u32 frameIndex{ 0 };
	};
}
