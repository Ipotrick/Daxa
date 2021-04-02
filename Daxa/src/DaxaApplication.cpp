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
			vkh::instance, 
			vkh::mainDevice,
			vkh::mainPhysicalDevice
			);

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

		VK_CHECK(vkResetFences(vkh::mainDevice, 1, &*renderFence));

		auto window = windowMutex->lock();

		uint32_t swapchainImageIndex;
		VK_CHECK(vkAcquireNextImageKHR(vkh::mainDevice, window->swapchain, 1000000000, *presentSem, nullptr, &swapchainImageIndex));

		VkCommandBuffer cmd = cmdPool.getBuffer();

		auto cmdBeginInfo = vkh::makeCmdBeginInfo();
		VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));
		
		VkClearValue clearValue;
		float flash = std::abs(sin(_frameNumber / 120.f));
		clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

		VkRenderPassBeginInfo rpInfo = {};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.pNext = nullptr;
		rpInfo.renderPass = *_renderPass;
		rpInfo.renderArea.offset.x = 0;
		rpInfo.renderArea.offset.y = 0;
		rpInfo.renderArea.extent = window->getExtent();
		rpInfo.framebuffer = *_framebuffers[swapchainImageIndex];
		rpInfo.clearValueCount = 1;
		rpInfo.pClearValues = &clearValue;
		vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

		if (window->bSpacePressed) {
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _trianglePipeline);
		}
		else {
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _redTrianglePipeline);
		}
		vkCmdDraw(cmd, 3, 1, 0, 0);
		
		vkCmdEndRenderPass(cmd);
		VK_CHECK(vkEndCommandBuffer(cmd));

		VkSubmitInfo submit = {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.pNext = nullptr;

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		submit.pWaitDstStageMask = &waitStage;

		VkSemaphore renderSem = semaPool.get();
		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = &*presentSem;
		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &renderSem;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmd;
		VK_CHECK(vkQueueSubmit(vkh::mainGraphicsQueue, 1, &submit, *renderFence));

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;
		presentInfo.pSwapchains = &window->swapchain;
		presentInfo.swapchainCount = 1;
		presentInfo.pWaitSemaphores = &renderSem;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pImageIndices = &swapchainImageIndex;
		VK_CHECK(vkQueuePresentKHR(vkh::mainGraphicsQueue, &presentInfo));

		VK_CHECK(vkWaitForFences(vkh::mainDevice, 1, &*renderFence, true, 1000000000));
		_frameNumber++;
	}

	void Application::init_pipelines()
	{
		vkh::ShaderModule colorFragShader{ "shaders/colortri.frag.spv" };
		vkh::ShaderModule colorVertShader{ "shaders/colortri.vert.spv" };

		if (!colorFragShader || !colorVertShader) {
			std::cout << "Error when building the color triangle shader modules" << std::endl;
			exit(-1);
		}

		//build the pipeline layout that controls the inputs/outputs of the shader
		//we are not using descriptor sets or other systems yet, so no need to use anything other than empty default
		VkPipelineLayoutCreateInfo pipeline_layout_info = vkh::makeLayoutCreateInfo();

		VK_CHECK(vkCreatePipelineLayout(vkh::mainDevice, &pipeline_layout_info, nullptr, &_trianglePipelineLayout));


		vkh::PipelineBuilder pipelineBuilder;

		pipelineBuilder._shaderStages.push_back(vkh::makeShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, colorVertShader));

		pipelineBuilder._shaderStages.push_back(vkh::makeShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, colorFragShader));

		pipelineBuilder._vertexInputInfo = vkh::makeVertexInputStageCreateInfo();

		pipelineBuilder._inputAssembly = vkh::makeInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

		auto _windowExtent = windowMutex->lock()->getExtent();

		//build viewport and scissor from the swapchain extents
		pipelineBuilder._viewport.x = 0.0f;
		pipelineBuilder._viewport.y = 0.0f;
		pipelineBuilder._viewport.width = (float)_windowExtent.width;
		pipelineBuilder._viewport.height = (float)_windowExtent.height;
		pipelineBuilder._viewport.minDepth = 0.0f;
		pipelineBuilder._viewport.maxDepth = 1.0f;

		pipelineBuilder._scissor.offset = { 0, 0 };
		pipelineBuilder._scissor.extent = _windowExtent;

		//configure the rasterizer to draw filled triangles
		//pipelineBuilder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);
		pipelineBuilder._rasterizer = vkh::makeRasterisationStateCreateInfo(VK_POLYGON_MODE_FILL);

		//we don't use multisampling, so just run the default one
		//pipelineBuilder._multisampling = vkinit::multisampling_state_create_info();
		pipelineBuilder._multisampling = vkh::makeMultisampleStateCreateInfo();

		//a single blend attachment with no blending and writing to RGBA
		//pipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();
		pipelineBuilder._colorBlendAttachment = vkh::makeColorBlendSAttachmentState();

		//use the triangle layout we created
		pipelineBuilder._pipelineLayout = _trianglePipelineLayout;

		//finally build the pipeline
		_trianglePipeline = pipelineBuilder.build(*_renderPass, vkh::mainDevice);



		vkh::ShaderModule redVertShader{ "shaders/redtri.vert.spv" };
		vkh::ShaderModule redFragShader{ "shaders/redtri.frag.spv" };

		if (!redVertShader || !redFragShader) {
			std::cout << "Error when building the red triangle shader modules" << std::endl;
			exit(-1);
		}

		//clear the shader stages for the builder
		pipelineBuilder._shaderStages.clear();

		//add the other shaders
		pipelineBuilder._shaderStages.push_back(
			vkh::makeShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, redVertShader));
		//vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, triangleVertexShader));

		pipelineBuilder._shaderStages.push_back(
			vkh::makeShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, redFragShader));

		//build the red triangle pipeline
		_redTrianglePipeline = pipelineBuilder.build(*_renderPass, vkh::mainDevice);

		init_mesh_pipeline();
	}

	void Application::init_mesh_pipeline()
	{
		vkh::BetterPipelineBuilder pipelineBuilder;
		pipelineBuilder.setVertexInfo(Vertex::INFO);

		const VkExtent2D windowExtent = windowMutex->lock()->getExtent();
		pipelineBuilder.viewport = VkViewport{
			.x = 0.0f,
			.y = 0.0f,
			.width = (float)windowExtent.width,
			.height = (float)windowExtent.height,
			.minDepth = 0.0f,
			.maxDepth = 1.0f,
		};

		pipelineBuilder.pipelineLayout = _trianglePipelineLayout;

		vkh::ShaderModule fragShader{ "shaders/colortri.frag.spv" };
		if (!fragShader)
			std::cout << "Error when building the mesh fragment shader module" << std::endl;
		else
			std::cout << "Red mesh fragment shader succesfully loaded" << std::endl;

		vkh::ShaderModule vertShader{ "shaders/tri_mesh.vert.spv" };
		if (!vertShader)
			std::cout << "Error when building the mesh vertex shader module" << std::endl;
		else
			std::cout << "Red mesh vertex shader succesfully loaded" << std::endl;

		pipelineBuilder._shaderStages.push_back( vkh::makeShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, vertShader) );
		pipelineBuilder._shaderStages.push_back( vkh::makeShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, fragShader) );

		meshPipeline = pipelineBuilder.build(*_renderPass);
	}

	void Application::init_default_renderpass()
	{
		VkAttachmentDescription color_attachment = {};									// the renderpass will use this color attachment.

		color_attachment.format = windowMutex->lock()->swapchainImageFormat;			// the attachment will have the format needed by the swapchain

		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;								// 1 sample, we won't be doing MSAA

		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;							// we Clear when this attachment is loaded
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;						// we keep the attachment stored when the renderpass ends

		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;				// we don't care about stencil
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;				// we don't care about stencil

		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;						// we don't know or care about the starting layout of the attachment
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;					// after the renderpass ends, the image has to be on a layout ready for display



		VkAttachmentReference color_attachment_ref = {};
		//attachment number will index into the pAttachments array in the parent renderpass itself
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//we are going to create 1 subpass, which is the minimum you can do
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;



		VkRenderPassCreateInfo render_pass_info = {};
		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

		//connect the color attachment to the info
		render_pass_info.attachmentCount = 1;
		render_pass_info.pAttachments = &color_attachment;
		//connect the subpass to the info
		render_pass_info.subpassCount = 1;
		render_pass_info.pSubpasses = &subpass;

		_renderPass = vkh::makeRenderPass(render_pass_info);
	}

	void Application::init_framebuffers()
	{
		//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
		VkFramebufferCreateInfo fb_info = {};
		fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_info.pNext = nullptr;

		auto window = windowMutex->lock();

		fb_info.renderPass = *_renderPass;
		fb_info.attachmentCount = 1;
		fb_info.width = window->getSize()[0];
		fb_info.height = window->getSize()[1];
		fb_info.layers = 1;

		//grab how many images we have in the swapchain
		const u32 swapchain_imagecount = window->swapchainImages.size();
		_framebuffers = std::vector<vkh::UniqueHandle<VkFramebuffer>>(swapchain_imagecount);

		//create framebuffers for each of the swapchain image views
		for (i32 i = 0; i < swapchain_imagecount; i++) {

			fb_info.pAttachments = &window->swapchainImageViews[i];
			VkFramebuffer fb;
			vkCreateFramebuffer(vkh::mainDevice, &fb_info, nullptr, &fb);
			_framebuffers[i] = { fb };
		}
	}

	void Application::cleanup()
	{
		auto window = windowMutex->lock();
		vkDestroyPipeline(vkh::mainDevice, meshPipeline, nullptr);
		vmaDestroyBuffer(vkh::allocator, triangleMesh.vertexBuffer.buffer, triangleMesh.vertexBuffer.allocation);
		vkDestroyPipeline(vkh::mainDevice, _trianglePipeline, nullptr);
		vkDestroyPipeline(vkh::mainDevice, _redTrianglePipeline, nullptr);
		vkDestroyPipelineLayout(vkh::mainDevice, _trianglePipelineLayout, nullptr);
	}
	void Application::loadMeshes()
	{
		//make the array 3 vertices long
		triangleMesh.vertices.resize(3);

		//vertex positions
		triangleMesh.vertices[0].position = { 1.f, 1.f, 0.0f };
		triangleMesh.vertices[1].position = { -1.f, 1.f, 0.0f };
		triangleMesh.vertices[2].position = { 0.f,-1.f, 0.0f };

		//vertex colors, all green
		triangleMesh.vertices[0].color = { 0.f, 1.f, 0.0f }; //pure green
		triangleMesh.vertices[1].color = { 0.f, 1.f, 0.0f }; //pure green
		triangleMesh.vertices[2].color = { 0.f, 1.f, 0.0f }; //pure green

		//we don't care about the vertex normals

		uploadMesh(triangleMesh);
	}

	void Application::uploadMesh(SimpleMesh& mesh)
	{
		VkBufferCreateInfo bufferInfo = vkh::makeVertexBufferCreateInfo(sizeof(Vertex) * triangleMesh.vertices.size());

		//let the VMA library know that this data should be writeable by CPU, but also readable by GPU
		VmaAllocationCreateInfo vmaallocInfo = {};
		vmaallocInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

		//allocate the buffer
		VK_CHECK(vmaCreateBuffer(vkh::allocator, &bufferInfo, &vmaallocInfo,
			&mesh.vertexBuffer.buffer,
			&mesh.vertexBuffer.allocation,
			nullptr));

		void* gpuMemory;
		vmaMapMemory(vkh::allocator, mesh.vertexBuffer.allocation, &gpuMemory);

		memcpy(gpuMemory, mesh.vertices.data(), mesh.vertices.size() * sizeof(Vertex));

		vmaUnmapMemory(vkh::allocator, mesh.vertexBuffer.allocation);
	}
}