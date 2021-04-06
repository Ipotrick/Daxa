#include "DaxaApplication.hpp"

#include <VkBootstrap.hpp>
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#include <iostream>

namespace daxa {
	Application::Application(std::string name, u32 width, u32 height)
	{
		windowMutex = std::make_unique<OwningMutex<Window>>(
			name, 
			std::array<u32, 2>{ width, height }, 
			vkh::device,
			vkh::mainPhysicalDevice
		);

		presentSem = vkh::device.createSemaphoreUnique(vkh::makeDefaultSemaphoreCI());

		init_default_renderpass();

		init_framebuffers();

		init_pipelines();

		loadMeshes();
	}

	Application::~Application()
	{
		Application::cleanup();
	}

	void Application::draw()
	{
		semaPool.flush();
		cmdPool.flush();

		vkh::device.resetFences(*renderFence);

		auto window = windowMutex->lock();

		u32 swapchainImageIndex;
		VK_CHECK(vkAcquireNextImageKHR(vkh::device, window->swapchain, 1000000000, *presentSem, nullptr, &swapchainImageIndex));

		vk::CommandBuffer cmd = cmdPool.getBuffer();

		auto cmdBeginInfo = vk::CommandBufferBeginInfo{};
		cmd.begin(cmdBeginInfo);

		std::array<vk::ClearValue, 2> clearValues{
			vk::ClearColorValue{ std::array{ 0.0f, 0.0f, 0.8f, 1.0f} },
			vk::ClearDepthStencilValue{ 1.0f }
		};

		vk::RenderPassBeginInfo rpInfo{
			.renderPass = *mainRenderpass,
			.framebuffer = *framebuffers[swapchainImageIndex],
			.renderArea = vk::Rect2D{.extent = window->getExtent()},
			.clearValueCount = (u32)clearValues.size(),
			.pClearValues = clearValues.data(),
		};
		cmd.beginRenderPass(rpInfo, vk::SubpassContents::eInline);

		cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, meshPipeline.pipeline.get());

		vk::Viewport viewport{
			.width = window->getSizeVec().x,
			.height = window->getSizeVec().y,
			.maxDepth = 1.0f,
		};
		vkCmdSetViewport(cmd, 0, 1, (VkViewport*)&viewport);
		vk::Rect2D scissor{
			.extent = window->getExtent()
		};
		cmd.setScissor(0, scissor);

		daxa::Vec3 camPos = { 0.f, 0.0f, -3.0f };

		daxa::Mat4x4 view = daxa::translate(daxa::Mat4x4(1.f), camPos);
		daxa::Mat4x4 projection = daxa::makeProjection<4,f32>(daxa::radians(70.f), 1700.f / 900.f, 0.1f, 200.0f);
		projection[1][1] *= -1;
		daxa::Mat4x4 model = daxa::rotate(daxa::Mat4x4{1.0f}, 0.0f, daxa::radians(_frameNumber * 0.04f), 0.0f);
		daxa::Mat4x4 mesh_matrix = projection * view * model;
		MeshPushConstants constants;
		constants.renderMatrix = daxa::transpose(mesh_matrix);
		std::array< MeshPushConstants, 1> c{ constants };

		//upload the matrix to the GPU via pushconstants
		 
		vkCmdPushConstants(cmd, meshPipeline.layout.get(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

		vk::DeviceSize offset = 0;
		vkCmdBindVertexBuffers(cmd, 0, 1, (VkBuffer*)&monkeyMesh.vertexBuffer, &offset);
		//cmd.bindVertexBuffers(0, monkeyMesh.vertexBuffer.buffer, 0ull);
		cmd.draw(monkeyMesh.vertices.size(), 1, 0, 0);

		cmd.endRenderPass();
		cmd.end();

		vk::SubmitInfo submit = {};
		vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		submit.pWaitDstStageMask = &waitStage;

		vk::Semaphore renderSem = semaPool.get();
		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = &*presentSem;
		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &renderSem;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmd;
		vkQueueSubmit(vkh::mainGraphicsQueue, 1, (VkSubmitInfo*)&submit, *renderFence);

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.pSwapchains = &window->swapchain;
		presentInfo.swapchainCount = 1;
		presentInfo.pWaitSemaphores = &renderSem;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pImageIndices = &swapchainImageIndex;
		VK_CHECK(vkQueuePresentKHR(vkh::mainGraphicsQueue, (VkPresentInfoKHR*)&presentInfo));

		VK_CHECK(vkWaitForFences(vkh::device, 1, (VkFence*)&*renderFence, true, 1000000000));
		_frameNumber++;
	}

	void Application::init_pipelines()
	{
		init_mesh_pipeline();
	}

	void Application::init_mesh_pipeline()
	{
		vkh::PipelineBuilder pipelineBuilder;

		pipelineBuilder.vertexInput = Vertex::INFO.makePipelineVertexInpitSCI();

		pipelineBuilder.pushConstants.push_back(vk::PushConstantRange{
			.stageFlags = vk::ShaderStageFlagBits::eVertex,
			.offset = 0,
			.size = sizeof(MeshPushConstants),
		});

		auto fragShaderOpt = vkh::loadShaderModule( "shaders/colortri.frag.spv");
		auto vertShaderOpt = vkh::loadShaderModule("shaders/tri_mesh.vert.spv" );

		if (!fragShaderOpt) std::cout << "Error when building the mesh fragment shader module" << std::endl;
		else std::cout << "Red mesh fragment shader succesfully loaded" << std::endl;
		if (!vertShaderOpt) std::cout << "Error when building the mesh vertex shader module" << std::endl;
		else std::cout << "Red mesh vertex shader succesfully loaded" << std::endl;

		pipelineBuilder.shaderStages.push_back(vkh::makeShaderStageCreateInfo(vk::ShaderStageFlagBits::eFragment, **fragShaderOpt));  
		pipelineBuilder.shaderStages.push_back(vkh::makeShaderStageCreateInfo(vk::ShaderStageFlagBits::eVertex, **vertShaderOpt));

		pipelineBuilder.depthStencil = vk::PipelineDepthStencilStateCreateInfo{
			.depthTestEnable = VK_TRUE,
			.depthWriteEnable = VK_TRUE,
			.depthCompareOp = vk::CompareOp::eLess,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.minDepthBounds = 0.0f,
			.maxDepthBounds = 1.0f,
		};

		meshPipeline = pipelineBuilder.build(*mainRenderpass);
	}

	void Application::init_default_renderpass()
	{
		std::array<vk::AttachmentDescription, 2> attachmentDescriptions{
			vk::AttachmentDescription{
				.format = windowMutex->lock()->swapchainImageFormat,
				.samples = vk::SampleCountFlagBits::e1,
				.loadOp = vk::AttachmentLoadOp::eClear,
				.storeOp = vk::AttachmentStoreOp::eStore,
				.stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
				.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
				.initialLayout = vk::ImageLayout::eUndefined,
				.finalLayout = vk::ImageLayout::ePresentSrcKHR,
			},
			vk::AttachmentDescription{
				.format = windowMutex->lock()->depthImageFormat,
				.loadOp = vk::AttachmentLoadOp::eClear,
				.storeOp = vk::AttachmentStoreOp::eStore,
				.stencilLoadOp = vk::AttachmentLoadOp::eClear,
				.stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
				.finalLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal
			}
		};

		std::array colorAttachmentRefs{
			vk::AttachmentReference{
				.attachment = 0,
				.layout = vk::ImageLayout::eColorAttachmentOptimal
			},
		};

		vk::AttachmentReference depthAttachmentRef{
			.attachment = 1,
			.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal
		};

		//we are going to create 1 subpass, which is the minimum you can do
		vk::SubpassDescription subpass{
			.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
			.colorAttachmentCount = (u32)colorAttachmentRefs.size(),
			.pColorAttachments = colorAttachmentRefs.data(),
			.pDepthStencilAttachment = &depthAttachmentRef,
		};

		mainRenderpass = vkh::device.createRenderPassUnique(vk::RenderPassCreateInfo{
			.attachmentCount = (u32)attachmentDescriptions.size(),
			.pAttachments = attachmentDescriptions.data(),
			.subpassCount = 1,
			.pSubpasses = &subpass
			});
	}

	void Application::init_framebuffers()
	{
		auto window = windowMutex->lock();
		//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
		vk::FramebufferCreateInfo framebufferCI{
			.renderPass = *mainRenderpass,
			.width = window->getSize()[0],
			.height = window->getSize()[1],
			.layers = 1,
		};
		//grab how many images we have in the swapchain
		const u32 swapchain_imagecount = window->swapchainImages.size();
		//_framebuffers = std::vector<vkh::UniqueHandle<vk::Framebuffer>>(swapchain_imagecount);
		framebuffers.resize(swapchain_imagecount);

		//create framebuffers for each of the swapchain image views
		for (i32 i = 0; i < swapchain_imagecount; i++) {
			std::array<vk::ImageView, 2> attachments{ window->swapchainImageViews[i], window->depthImageView.get() };
			framebufferCI.attachmentCount = (u32)attachments.size();
			framebufferCI.pAttachments = attachments.data();
			framebuffers[i] = vkh::device.createFramebufferUnique(framebufferCI);
		}
	}

	void Application::cleanup()
	{ }

	void Application::loadMeshes()
	{
		auto monkeyOpt = daxa::loadMeshFromObj("assets/monkey.obj");
		if (!monkeyOpt) {
			std::cout << "could not load assets/monkey.obj\n";
			exit(-1);
		}

		monkeyMesh = std::move(monkeyOpt.value());
		uploadMesh(monkeyMesh);
	}

	void Application::uploadMesh(SimpleMesh& mesh)
	{
		vk::BufferCreateInfo bufferInfo{
			.size = sizeof(Vertex) * mesh.vertices.size(),
			.usage = vk::BufferUsageFlagBits::eVertexBuffer
		};

		//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		//allocate the buffer
		vmaCreateBuffer(vkh::allocator, (VkBufferCreateInfo*)&bufferInfo, &vmaallocInfo,
			(VkBuffer*)&mesh.vertexBuffer.buffer,
			&mesh.vertexBuffer.allocation,
			nullptr);

		void* gpuMemory;
		vmaMapMemory(vkh::allocator, mesh.vertexBuffer.allocation, &gpuMemory);

		memcpy(gpuMemory, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));

		vmaUnmapMemory(vkh::allocator, mesh.vertexBuffer.allocation);
	}
}