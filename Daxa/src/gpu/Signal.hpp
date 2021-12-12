#pragma once

#include "../DaxaCore.hpp"

#include <memory>

#include <vulkan/vulkan.h>

namespace daxa {
	namespace gpu {
		class Signal {
		public:
			Signal(VkDevice device);
			Signal(Signal&&) noexcept				= delete;
			Signal& operator=(Signal&&) noexcept	= delete;
			Signal(Signal const&) 					= delete;
			Signal& operator=(Signal const&) 		= delete;
			~Signal();

			VkSemaphore getVkSemaphore() const { return semaphore; }
		private:
			friend class Device;

			VkDevice device = {};
			VkSemaphore semaphore = {};
		};

		class SignalHandle {
		public:
			SignalHandle(std::shared_ptr<Signal> signal) 
				: signal{ std::move(signal) }
			{}
			SignalHandle() = default;

			Signal const& operator*() const { return *signal; }
			Signal& operator*() { return *signal; }
			Signal const* operator->() const { return signal.get(); }
			Signal* operator->() { return signal.get(); }

			size_t getRefCount() const { return signal.use_count(); }

			operator bool() const { return signal.operator bool(); }
			bool operator!() const { return !signal; }
			bool valid() const { return *this; }
		private:
			std::shared_ptr<Signal> signal = {};
		};
	}
}