#pragma once

#include "threading/OwningMutex.hpp"
#include "threading/Jobs.hpp"
#include "platform/Window.hpp"

namespace daxa {

	class Application {
	public:
		Application(std::string name, u32 width, u32 height);
		~Application();

		std::unique_ptr<OwningMutex<Window>> windowMutex; 
	private:

		void init_default_renderpass()
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


			vkCreateRenderPass(vulkan::mainDevice, &render_pass_info, nullptr, &_renderPass);
		}		

		void init_framebuffers()
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
				vkCreateFramebuffer(vulkan::mainDevice, &fb_info, nullptr, &_framebuffers[i]);
			}
		}

		void cleanup()
		{
			auto window = windowMutex->lock();
			vkDestroySwapchainKHR(vulkan::mainDevice, window->swapchain, nullptr);

			//destroy the main renderpass
			vkDestroyRenderPass(vulkan::mainDevice, _renderPass, nullptr);

			//destroy swapchain resources
			for (int i = 0; i < _framebuffers.size(); i++) {
				vkDestroyFramebuffer(vulkan::mainDevice, _framebuffers[i], nullptr);

				vkDestroyImageView(vulkan::mainDevice, window->swapchainImageViews[i], nullptr);
			}

		}


		VkRenderPass _renderPass;

		std::vector<VkFramebuffer> _framebuffers;
	};
}
