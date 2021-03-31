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
			vk::instance, 
			vk::mainDevice,
			vk::mainPhysicalDevice
			);

		init_default_renderpass();

		init_framebuffers();

		init_pipelines();
	}

	Application::~Application()
	{
		cleanup();
	}

	void Application::draw()
	{
		VK_CHECK(vkResetFences(vk::mainDevice, 1, &_renderFence.get()));

		auto window = windowMutex->lock();

		uint32_t swapchainImageIndex;
		VK_CHECK(vkAcquireNextImageKHR(vk::mainDevice, window->swapchain, 1000000000, _presentSemaphore, nullptr, &swapchainImageIndex));

		//now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
		VK_CHECK(vkResetCommandBuffer(cmd, 0));

		//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
		VkCommandBufferBeginInfo cmdBeginInfo = {};
		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBeginInfo.pNext = nullptr;

		cmdBeginInfo.pInheritanceInfo = nullptr;
		cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));


		///
		/// RENDER PASS BEGIN
		/// 
		/// 


		
		
		VkClearValue clearValue;
		float flash = abs(sin(_frameNumber / 120.f));
		clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };

		//start the main renderpass. 
		//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
		VkRenderPassBeginInfo rpInfo = {};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.pNext = nullptr;

		rpInfo.renderPass = _renderPass;
		rpInfo.renderArea.offset.x = 0;
		rpInfo.renderArea.offset.y = 0;
		rpInfo.renderArea.extent = window->getExtent();
		rpInfo.framebuffer = _framebuffers[swapchainImageIndex];

		//connect clear values
		rpInfo.clearValueCount = 1;
		rpInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);

		//once we start adding rendering commands, they will go here

		if (window->bSpacePressed) {
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _trianglePipeline);
		}
		else {
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _redTrianglePipeline);
		}
		vkCmdDraw(cmd, 3, 1, 0, 0);
		
		//finalize the render pass
		vkCmdEndRenderPass(cmd);
		//finalize the command buffer (we can no longer add commands, but it can now be executed)
		VK_CHECK(vkEndCommandBuffer(cmd));



		///
		/// RENDER PASS END
		/// 



		//prepare the submission to the queue. 
		//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
		//we will signal the _renderSemaphore, to signal that rendering has finished

		VkSubmitInfo submit = {};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.pNext = nullptr;

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		submit.pWaitDstStageMask = &waitStage;

		submit.waitSemaphoreCount = 1;
		submit.pWaitSemaphores = &_presentSemaphore.get();

		submit.signalSemaphoreCount = 1;
		submit.pSignalSemaphores = &_renderSemaphore.get();

		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &cmd.get();

		//submit command buffer to the queue and execute it.
		// _renderFence will now block until the graphic commands finish execution
		VK_CHECK(vkQueueSubmit(vk::mainGraphicsQueue, 1, &submit, _renderFence));



		// this will put the image we just rendered into the visible window.
		// we want to wait on the _renderSemaphore for that, 
		// as it's necessary that drawing commands have finished before the image is displayed to the user
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.pNext = nullptr;

		presentInfo.pSwapchains = &window->swapchain;
		presentInfo.swapchainCount = 1;

		presentInfo.pWaitSemaphores = &_renderSemaphore.get();
		presentInfo.waitSemaphoreCount = 1;

		presentInfo.pImageIndices = &swapchainImageIndex;

		VK_CHECK(vkQueuePresentKHR(vk::mainGraphicsQueue, &presentInfo));

		//increase the number of frames drawn
		_frameNumber++;
		VK_CHECK(vkWaitForFences(vk::mainDevice, 1, &_renderFence.get(), true, 1000000000));
	}

	void Application::init_pipelines()
	{
		auto fragShaderOpt = vk::loadShaderModule("shaders/colortri.fspv");
		auto vertShaderOpt = vk::loadShaderModule("shaders/colortri.vspv");

		if (!fragShaderOpt || !vertShaderOpt) {
			std::cout << "Error when building the triangle shader modules" << std::endl;
			exit(-1);
		}

		auto triangleFragShader = fragShaderOpt.value();
		auto triangleVertexShader = vertShaderOpt.value();

		//build the pipeline layout that controls the inputs/outputs of the shader
		//we are not using descriptor sets or other systems yet, so no need to use anything other than empty default
		VkPipelineLayoutCreateInfo pipeline_layout_info = vk::makeLayoutCreateInfo();

		VK_CHECK(vkCreatePipelineLayout(vk::mainDevice, &pipeline_layout_info, nullptr, &_trianglePipelineLayout));




		vk::PipelineBuilder pipelineBuilder;

		pipelineBuilder._shaderStages.push_back(
			vk::makeShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, triangleVertexShader));
			//vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, triangleVertexShader));

		pipelineBuilder._shaderStages.push_back(
			vk::makeShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader));
			//vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, triangleFragShader));


		//vertex input controls how to read vertices from vertex buffers. We aren't using it yet
		pipelineBuilder._vertexInputInfo = vk::makeVertexInputStageCreateInfo();
		//pipelineBuilder._vertexInputInfo = vkinit::vertex_input_state_create_info();

		//input assembly is the configuration for drawing triangle lists, strips, or individual points.
		//we are just going to draw triangle list
		//pipelineBuilder._inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		pipelineBuilder._inputAssembly = vk::makeInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

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
		pipelineBuilder._rasterizer = vk::makeRasterisationStateCreateInfo(VK_POLYGON_MODE_FILL);

		//we don't use multisampling, so just run the default one
		//pipelineBuilder._multisampling = vkinit::multisampling_state_create_info();
		pipelineBuilder._multisampling = vk::makeMultisampleStateCreateInfo();

		//a single blend attachment with no blending and writing to RGBA
		//pipelineBuilder._colorBlendAttachment = vkinit::color_blend_attachment_state();
		pipelineBuilder._colorBlendAttachment = vk::makeColorBlendSAttachmentState();

		//use the triangle layout we created
		pipelineBuilder._pipelineLayout = _trianglePipelineLayout;

		//finally build the pipeline
		_trianglePipeline = pipelineBuilder.build(_renderPass, vk::mainDevice);






		auto redfragShaderOpt = vk::loadShaderModule("shaders/redtri.fspv");
		auto redvertShaderOpt = vk::loadShaderModule("shaders/redtri.vspv");

		if (!redfragShaderOpt || !redvertShaderOpt) {
			std::cout << "Error when building the triangle shader modules" << std::endl;
			exit(-1);
		}

		auto redtriangleFragShader = redfragShaderOpt.value();
		auto redtriangleVertexShader = redvertShaderOpt.value();

		//clear the shader stages for the builder
		pipelineBuilder._shaderStages.clear();

		//add the other shaders
		pipelineBuilder._shaderStages.push_back(
			vk::makeShaderStageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT, redtriangleVertexShader));
		//vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, triangleVertexShader));

		pipelineBuilder._shaderStages.push_back(
			vk::makeShaderStageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT, redtriangleFragShader));

		//build the red triangle pipeline
		_redTrianglePipeline = pipelineBuilder.build(_renderPass, vk::mainDevice);
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


		vkCreateRenderPass(vk::mainDevice, &render_pass_info, nullptr, &_renderPass);
	}
	void Application::init_framebuffers()
	{
		//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
		VkFramebufferCreateInfo fb_info = {};
		fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fb_info.pNext = nullptr;

		auto window = windowMutex->lock();

		fb_info.renderPass = _renderPass;
		fb_info.attachmentCount = 1;
		fb_info.width = window->getSize()[0];
		fb_info.height = window->getSize()[1];
		fb_info.layers = 1;

		//grab how many images we have in the swapchain
		const u32 swapchain_imagecount = window->swapchainImages.size();
		_framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);

		//create framebuffers for each of the swapchain image views
		for (i32 i = 0; i < swapchain_imagecount; i++) {

			fb_info.pAttachments = &window->swapchainImageViews[i];
			vkCreateFramebuffer(vk::mainDevice, &fb_info, nullptr, &_framebuffers[i]);
		}
	}
	void Application::cleanup()
	{
		auto window = windowMutex->lock();

		//vkDestroySemaphore(vulkan::mainDevice, _renderSemaphore, nullptr);
		//vkDestroySemaphore(vulkan::mainDevice, _presentSemaphore, nullptr);
		//vkDestroyFence(vulkan::mainDevice, _renderFence, nullptr);

		//destroy the main renderpass
		vkDestroyRenderPass(vk::mainDevice, _renderPass, nullptr);

		//destroy swapchain resources
		for (int i = 0; i < _framebuffers.size(); i++) {
			vkDestroyFramebuffer(vk::mainDevice, _framebuffers[i], nullptr);
		}

	}
}