#include "Swapchain.hpp"

#include <iostream>
#include <chrono>

#include <VkBootstrap.h>

#include "Instance.hpp"
#include "backend/DeviceBackend.hpp"

namespace daxa {
	void Swapchain::construct(std::shared_ptr<DeviceBackend> deviceBackend, SwapchainCreateInfo ci) {
		this->deviceBackend = deviceBackend;
		this->size = { .width = ci.width, .height = ci.height };
		this->surface = ci.surface;
		this->presentMode = ci.presentMode;
		this->additionalimageUses = ci.additionalUses;

		vkb::SwapchainBuilder swapchainBuilder{ deviceBackend->device, surface };

		auto oldSwapchain = swapchain;
		if (swapchain != VK_NULL_HANDLE) {
			swapchainBuilder.set_old_swapchain(swapchain);
		}

		vkb::Swapchain vkbSwapchain = swapchainBuilder
			.use_default_format_selection()
			.set_desired_present_mode(presentMode)
			.set_desired_extent(ci.width, ci.height)
			.add_image_usage_flags(additionalimageUses)
			.build()
			.value();
		
		//store swapchain and its related images
		this->swapchain = vkbSwapchain.swapchain;
		auto vkImages = vkbSwapchain.get_images().value();
		auto vkImageViews = vkbSwapchain.get_image_views().value();
		for (int i = 0; i < vkImages.size(); i++) {
			auto image = ImageHandle{ std::make_shared<Image>() };
			auto& img = *image;
			img.extent.width = ci.width;
			img.extent.height = ci.height;
			img.extent.depth = 1;
			img.format = vkbSwapchain.image_format;
			img.allocation = nullptr;
			img.image = vkImages[i];
			img.imageType = VK_IMAGE_TYPE_2D;
			img.tiling = VK_IMAGE_TILING_OPTIMAL;
			img.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | additionalimageUses;
			img.arrayLayers = 1;
			img.mipLevels = 1;
			
			auto sampler = std::move(SamplerHandle{ std::make_shared<Sampler>(deviceBackend, SamplerCreateInfo{})});
			ImageViewCreateInfo ci{
				.image = std::move(image),
				.format = vkbSwapchain.image_format,
				.defaultSampler = sampler,
				.debugName = "swapchain image view",
			};
			auto view = ImageViewHandle{ std::make_shared<ImageView>(deviceBackend, ci, vkImageViews[i]) };
			this->swapchainImageViews.push_back(std::move(view));
		}

		this->swapchainImageFormat = vkbSwapchain.image_format;

		if (oldSwapchain) {
			vkDestroySwapchainKHR(deviceBackend->device.device, oldSwapchain, nullptr);
		}

		VkFenceCreateInfo fenceCI{
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
		};

		if (aquireFence == VK_NULL_HANDLE) {
			DAXA_CHECK_VK_RESULT_M(vkCreateFence(deviceBackend->device.device, &fenceCI, nullptr, &this->aquireFence), "failed to create swaichain fence");
		}

		if (daxa::instance->pfnSetDebugUtilsObjectNameEXT != nullptr && ci.debugName != nullptr) {
			this->debugName = ci.debugName;

			auto nameInfo = VkDebugUtilsObjectNameInfoEXT{
				.sType =  VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_SWAPCHAIN_KHR,
				.objectHandle = (uint64_t)this->swapchain,
				.pObjectName = ci.debugName,
			};
			daxa::instance->pfnSetDebugUtilsObjectNameEXT(deviceBackend->device.device, &nameInfo);

			std::string nameBuffer = ci.debugName;
			nameBuffer += " fence";

			nameInfo = VkDebugUtilsObjectNameInfoEXT{
				.sType =  VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
				.pNext = NULL,
				.objectType = VK_OBJECT_TYPE_FENCE,
				.objectHandle = (uint64_t)this->aquireFence,
				.pObjectName = nameBuffer.c_str(),
			};
			daxa::instance->pfnSetDebugUtilsObjectNameEXT(deviceBackend->device.device, &nameInfo);
		}
	}

	Swapchain::~Swapchain() {
		swapchainImageViews.clear();
		if (deviceBackend->device.device) {
			vkDestroySwapchainKHR(deviceBackend->device.device, swapchain, nullptr);
			vkDestroyFence(deviceBackend->device.device, aquireFence, nullptr);
			deviceBackend = {};
		}
	}

	SwapchainImage Swapchain::aquireNextImage() {
		u32 index{ 0 };
		DAXA_CHECK_VK_RESULT_M(vkAcquireNextImageKHR(deviceBackend->device.device, swapchain, UINT64_MAX, nullptr, aquireFence, &index), "could not aquire next image from swapchain");
		SwapchainImage si{};
		si.swapchain = swapchain;
		si.imageIndex = index;
		si.image = swapchainImageViews[index];

		DAXA_CHECK_VK_RESULT_M(vkWaitForFences(deviceBackend->device.device, 1, &aquireFence, VK_TRUE, UINT64_MAX), "failed to wait on swapchain fence");
		DAXA_CHECK_VK_RESULT_M(vkResetFences(deviceBackend->device.device, 1, &aquireFence), "failed to reset swapchain fence");

		return si;
	}

	void Swapchain::resize(VkExtent2D newSize) {
		swapchainImageViews.clear();
		construct(deviceBackend, {surface, newSize.width, newSize.height, presentMode, additionalimageUses, debugName.c_str()});
	}

	void Swapchain::setPresentMode(VkPresentModeKHR newPresentMode) {
		swapchainImageViews.clear();
		construct(deviceBackend, {surface, size.width, size.height, newPresentMode, additionalimageUses, debugName.c_str()});
	}
	
	Result<Swapchain2> Swapchain2::construct(std::shared_ptr<DeviceBackend>& deviceBackend, SwapchainCreateInfo const& ci, Swapchain2* old) {
		struct {
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		} SwapchainCapabilities;

		Swapchain2 res = {};
		res.deviceBackend = deviceBackend;
		res.ci = ci;

		VkSwapchainCreateInfoKHR scci = {};
		scci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		scci.pNext = nullptr;
		scci.surface = ci.surface;
		scci.presentMode = ci.presentMode;
		scci.oldSwapchain = VK_NULL_HANDLE;
		if (old != nullptr) {
			DAXA_ASSERT_M(old->swapchain != VK_NULL_HANDLE, "invalid old swapchain");
			scci.oldSwapchain = old->swapchain;
			old->swapchain = VK_NULL_HANDLE;
		}
		scci.minImageCount = 2; 
		scci.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
		scci.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		scci.imageArrayLayers = 1;
		scci.imageExtent = { .width = ci.width, .height = ci.height };
		scci.imageUsage = ci.additionalUses;
		scci.pQueueFamilyIndices = &deviceBackend->graphicsQFamilyIndex;
		scci.queueFamilyIndexCount = 1;
		scci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;

		vkCreateSwapchainKHR(deviceBackend->device.device, &scci, nullptr, &res.swapchain);

        return res;
	}
}
