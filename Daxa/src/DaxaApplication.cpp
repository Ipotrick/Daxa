#include "DaxaApplication.hpp"

#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <VkBootstrap.hpp>

namespace daxa {
	Application::Application(std::string name, u32 width, u32 height): 
		gpu{ VulkanGlobals::getGlobalContext() }, descLayoutCache(gpu.device)
	{
		windowMutex = std::make_unique<OwningMutex<Window>>(
			name, 
			std::array<u32, 2>{ width, height },
			gpu
		);

		for (i64 i = 0; i < FRAME_OVERLAP; i++) {
			frames.emplace_back(&descLayoutCache, globalSetAllocator.allocate());
		}

		vk::SamplerCreateInfo samplerCI{
			.addressModeU = vk::SamplerAddressMode::eClampToEdge,
			.addressModeV = vk::SamplerAddressMode::eClampToEdge,
			.borderColor = vk::BorderColor::eFloatOpaqueWhite,
		};
		sampler = gpu.device.createSamplerUnique(samplerCI);

		init_default_renderpass();

		init_framebuffers();

		init_pipelines();

		loadMeshes();

		constexpr u8 DEFAULT_COLOR[4]{ 0xFF,0xF0,0xFF,0xFF };
		defaultDummyImage = createImage2d(gpu, { .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst });
		uploadImage2d(gpu, frames[0].cmdPool.getElement(), defaultDummyImage, DEFAULT_COLOR);
	}

	Application::~Application()
	{
		for (auto& frame : frames) {
			gpu.device.waitForFences(*frame.fence, VK_TRUE, 1000000000);
			gpu.device.resetFences(*frame.fence);
		}
	}

	void Application::update(f32 dt) 
	{
		testHotLoader.update();

		constexpr f32 cameraMoveSpeed = 4.0f;
		constexpr f32 cameraSensitivity = 0.2f;

		auto window = windowMutex->lock();

		auto [ax, ay, az] = daxa::getFPSViewAxis(camera.position, camera.pitch, camera.yaw);
		if (bCameraControll) {
			if (window->keyPressed(daxa::Scancode::A)) {
				camera.position -= ax * dt * cameraMoveSpeed;
			} 
			if (window->keyPressed(daxa::Scancode::D)) {
				camera.position += ax * dt * cameraMoveSpeed;
			}
			if (window->keyPressed(daxa::Scancode::SPACE)) {
				camera.position -= ay * dt * cameraMoveSpeed;
			}
			if (window->keyPressed(daxa::Scancode::LSHIFT)) {
				camera.position += ay * dt * cameraMoveSpeed;
			}
			if (window->keyPressed(daxa::Scancode::W)) {
				camera.position += az * dt * cameraMoveSpeed;
			}
			if (window->keyPressed(daxa::Scancode::S)) {
				camera.position -= az * dt * cameraMoveSpeed;
			}
			if (window->keyPressed(daxa::Scancode::Q)) {
				camera.rotation += dt * cameraMoveSpeed;
			}
			if (window->keyPressed(daxa::Scancode::E)) {
				camera.rotation -= dt * cameraMoveSpeed;
			}
			camera.yaw -= window->getCursorPositionChange()[0] / 180.0f * cameraSensitivity;
			camera.pitch -= window->getCursorPositionChange()[1] / 180.0f * cameraSensitivity;
		}
		if (window->keyJustPressed(daxa::Scancode::ESCAPE)) {
			bCameraControll = !bCameraControll;
			if (bCameraControll) {
				window->captureCursor();
			}
			else {
				window->releaseCursor();
			}
		}

		{
			auto [ax, ay, az] = daxa::getFPSViewAxis(camera.position, camera.pitch, camera.yaw);

			ay = rotate(ay, az, camera.rotation);
			ax = rotate(ax, az, camera.rotation);

			camera.view = daxa::makeView({ax,ay,az}, camera.position);
		}
	}

	void Application::draw()
	{ 
		auto& frame = frames[_frameNumber & 1];

		gpu.device.waitForFences(*frame.fence,VK_TRUE, 1000000000);
		gpu.device.resetFences(*frame.fence);

		frame.semaPool.flush();
		frame.cmdPool.flush();

		auto window = windowMutex->lock();
		//
		u32 swapchainImageIndex;
		VK_CHECK(vkAcquireNextImageKHR(gpu.device, window->swapchain, 1000000000, *frame.presentSem, nullptr, &swapchainImageIndex));

		vk::CommandBuffer cmd = frame.cmdPool.getElement();

		auto cmdBeginInfo = vk::CommandBufferBeginInfo{};
		cmd.begin(cmdBeginInfo);

		std::array<vk::ClearValue, 2> clearValues{
			vk::ClearColorValue{ std::array{ 0.5f, 0.5f, 0.7f, 1.0f} },
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

		daxa::Mat4x4 projection = daxa::makeProjection<4,f32>(daxa::radians(70.f), window->getSizeVec().x / window->getSizeVec().y, 0.1f, 200.0f);
		projection[1][1] *= -1;
		camera.proj = projection;
		daxa::Mat4x4 model = daxa::rotate(daxa::Mat4x4{1.0f}, 0.0f, daxa::radians(_frameNumber * 0.04f), 0.0f);
		daxa::Mat4x4 mesh_matrix = projection * camera.view * model;
		MeshPushConstants constants;
		constants.renderMatrix = daxa::transpose(mesh_matrix);
		std::array< MeshPushConstants, 1> c{ constants };

		//upload the matrix to the GPU via pushconstants
		GPUData gpudata{
			.model = transpose(model),
			.view = transpose(camera.view),
			.proj = transpose(camera.proj)
		};
		{
			// UPDATE UNIFORM BUFFERS:
			void* data;
			vmaMapMemory(gpu.allocator, frame.gpuDataBuffer.allocation, &data);

			memcpy(data, &gpudata, frame.gpuDataBuffer.size);

			vmaUnmapMemory(gpu.allocator, frame.gpuDataBuffer.allocation);
		}
		{
			//update texture table:
			std::array< vk::DescriptorImageInfo, 1 << 12> imageInfos;
			for (auto i = 0; i < 1 << 12; i++) {
				imageInfos[i] = vk::DescriptorImageInfo{
					.imageView = defaultDummyImage.view.get(),
					.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal,
				};
			}

			vk::WriteDescriptorSet write{
				.dstSet = frame.globalSet,
				.dstBinding = 0,
				.descriptorCount = 1 << 12,
				.descriptorType = vk::DescriptorType::eSampledImage,
				.pImageInfo = &imageInfos[0],
			};

			vk::DescriptorImageInfo samplerImageInfo{
				.sampler = sampler.get()
			};
			vk::WriteDescriptorSet writeSampler{
				.dstSet = frame.globalSet,
				.dstBinding = 1,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eSampler,
				.pImageInfo = &samplerImageInfo,
			};
			gpu.device.updateDescriptorSets({ write, writeSampler }, {});
		}

		cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, meshPipeline.layout.get(), 0, std::array{ frame.globalSet, frame.descSet }, {});
		 
		vkCmdPushConstants(cmd, meshPipeline.layout.get(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConstants), &constants);

		vk::DeviceSize offset = 0;
		vkCmdBindVertexBuffers(cmd, 0, 1, (VkBuffer*)&monkeyMesh.vertexBuffer, &offset);
		cmd.draw(monkeyMesh.vertices.size(), 1, 0, 0);

		cmd.endRenderPass();
		cmd.end();

		vk::SubmitInfo submit = {};
		vk::PipelineStageFlags waitStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		submit.pWaitDstStageMask = &waitStage;

		vk::Semaphore renderSem = frame.semaPool.get();
		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = &*frame.presentSem;
		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &renderSem;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmd;
		vkQueueSubmit(gpu.graphicsQ, 1, (VkSubmitInfo*)&submit, *frame.fence);

		vk::PresentInfoKHR presentInfo = {};
		presentInfo.pSwapchains = &window->swapchain;
		presentInfo.swapchainCount = 1;
		presentInfo.pWaitSemaphores = &renderSem;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pImageIndices = &swapchainImageIndex;
		VK_CHECK(vkQueuePresentKHR(gpu.graphicsQ, (VkPresentInfoKHR*)&presentInfo));
		_frameNumber++;
	}

	void Application::init_pipelines()
	{
		init_mesh_pipeline();
	}
	void Application::init_mesh_pipeline()
	{
		auto opt = createMeshPipeline();
		if (!opt) {
			std::cerr << "ERROR: failed to create mesh pipeline!\n";
			exit(-1);
		}
		else {
			this->meshPipeline = std::move(opt.value());
		}
	}

	std::optional<vkh::Pipeline> Application::createMeshPipeline() {
		auto f = loadGLSLShaderToSpv("Daxa/shaders/mesh.frag").value();
		auto v = loadGLSLShaderToSpv("Daxa/shaders/mesh.vert").value();

		vkh::GraphicsPipelineBuilder pipelineBuilder(
			gpu.device,
			*mainRenderpass);

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

	void Application::init_default_renderpass()
	{
		auto swapchainImageFormat = windowMutex->lock()->swapchainImageFormat;
		auto depthFormat = windowMutex->lock()->depthImageFormat;
		mainRenderpass = vkh::RenderPassBuilder { gpu.device }
			.addAttachment(vk::AttachmentDescription{
				.format = swapchainImageFormat,
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
			std::array<vk::ImageView, 2> attachments{ window->swapchainImageViews[i], window->depthImage.view.get() };
			framebufferCI.attachmentCount = (u32)attachments.size();
			framebufferCI.pAttachments = attachments.data();
			framebuffers[i] = gpu.device.createFramebufferUnique(framebufferCI);
		}
	}

	void Application::cleanup()
	{ }

	void Application::loadMeshes()
	{
		auto monkeyOpt = daxa::loadMeshFromObj("DaxaTriangle/assets/monkey.obj");
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
		vmaCreateBuffer(gpu.allocator, (VkBufferCreateInfo*)&bufferInfo, &vmaallocInfo,
			(VkBuffer*)&mesh.vertexBuffer.buffer,
			&mesh.vertexBuffer.allocation,
			nullptr);

		void* gpuMemory;
		vmaMapMemory(gpu.allocator, mesh.vertexBuffer.allocation, &gpuMemory);

		memcpy(gpuMemory, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));

		vmaUnmapMemory(gpu.allocator, mesh.vertexBuffer.allocation);
	}
}