#pragma once

#include "../DaxaCore.hpp"

#include <memory>

#include "Handle.hpp"
#include <vulkan/vulkan.h>

namespace daxa {
	namespace gpu {

		struct SignalCreateInfo {
			char const* debugName = {};
		};

		class Signal {
		public:
			Signal(VkDevice device, SignalCreateInfo const& ci);
			Signal(Signal&&) noexcept				= delete;
			Signal& operator=(Signal&&) noexcept	= delete;
			Signal(Signal const&) 					= delete;
			Signal& operator=(Signal const&) 		= delete;
			~Signal();

			VkSemaphore getVkSemaphore() const { return semaphore; }

			std::string const& getDebugName() const { return debugName; }
		private:
			friend class Device;

			VkDevice 	device 		= {};
			VkSemaphore semaphore 	= {};
			std::string debugName 	= {};
		};

		class SignalHandle : public SharedHandle<Signal>{};
	}
}