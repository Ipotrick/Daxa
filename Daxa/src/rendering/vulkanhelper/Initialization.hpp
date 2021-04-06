#pragma once

#include "../Vulkan.hpp"

namespace daxa {
	namespace vkh {
		vk::FenceCreateInfo makeDefaultFenceCI();

		vk::SemaphoreCreateInfo makeDefaultSemaphoreCI();

		vk::AttachmentDescription makeDefaultAttackmentDescription();

		vk::PipelineRasterizationStateCreateInfo makeDefaultPipelineRasterizationSCI();
	}
}
