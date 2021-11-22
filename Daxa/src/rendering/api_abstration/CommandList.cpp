#include "CommandList.hpp"
#include "common.hpp"

namespace gpu {

	DAXA_DEFINE_TRIVIAL_MOVE(CommandList)

	CommandList::CommandList() {
		
	}

	CommandList::~CommandList() {
		assert(!bUnfinishedOperationInProgress);
		assert(empty);
	}

	void CommandList::beginRendering(BeginRenderingInfo ri) {
		vk::RenderingInfoKHR renderInfo{};

		// Maybe add the ability to change the renderArea later.
		// For now this is enough:
		if (ri.colorAttachmentCount > 0) {
			renderInfo.renderArea.extent.width = ri.colorAttachemnts[0]->getVkCreateInfo().extent.width;
			renderInfo.renderArea.extent.height = ri.colorAttachemnts[0]->getVkCreateInfo().extent.height;
		}
		else {
			assert(ri.depthStencilAttachment);
			renderInfo.renderArea.extent.width = (**ri.depthStencilAttachment).getVkCreateInfo().extent.width;
			renderInfo.renderArea.extent.height = (**ri.depthStencilAttachment).getVkCreateInfo().extent.height;
		}

		// TODO
	}
	void CommandList::endRendering() {

		// TODO
	}

	void CommandList::reset() {
		empty = true;
		cmd.reset();
		usedBuffers.clear();
		usedImages.clear();
	}
}
