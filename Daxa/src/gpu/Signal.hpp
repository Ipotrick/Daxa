#pragma once

#include "../DaxaCore.hpp"

#include <memory>

#include <vulkan/vulkan.h>

namespace daxa {
	namespace gpu {
		class Signal {
		public:
			Signal(Signal&&) noexcept;
			Signal& operator=(Signal&&) noexcept;
			Signal(Signal const&) = delete;
			Signal& operator=(Signal const&) = delete;
			~Signal();

			VkSemaphore getVkSemaphore() const { return semaphore; }
		private:
			friend class Device;

			Signal(VkDevice device);

			VkDevice device = {};
			VkSemaphore semaphore = {};
		};

		class SignalHandle {
		public:
			Signal& operator*() { return *signal; }
			Signal const& operator*() const { return *signal; }
			Signal* operator->() { return &*signal; }
			Signal const* operator->() const { return &*signal; }
		private:
			friend class Device;

			SignalHandle(std::shared_ptr<Signal> signal);

			std::shared_ptr<Signal> signal;
		};
	}
}