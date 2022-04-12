#pragma once

#include "../DaxaCore.hpp"

#include <memory>

#include <vulkan/vulkan.h>

#include "Handle.hpp"

namespace daxa {
	struct SignalCreateInfo {
		char const* debugName = {};
	};

	class Signal {
	public:
		Signal(std::shared_ptr<DeviceBackend> deviceBackend, SignalCreateInfo const& ci);
		Signal(Signal&&) noexcept				= delete;
		Signal& operator=(Signal&&) noexcept	= delete;
		Signal(Signal const&) 					= delete;
		Signal& operator=(Signal const&) 		= delete;
		~Signal();

		VkSemaphore getVkSemaphore() const { return semaphore; }

		std::string const& getDebugName() const { return debugName; }
	private:
		friend class Device;

		std::shared_ptr<DeviceBackend> 	deviceBackend 	= {};
		VkSemaphore 					semaphore 		= {};
		std::string 					debugName 		= {};
	};

	class SignalHandle : public SharedHandle<Signal>{};
}