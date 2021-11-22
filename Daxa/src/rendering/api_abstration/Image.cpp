#include "Image.hpp"

namespace gpu {
	Image::~Image() {
		if (allocator) {
			vmaDestroyImage(allocator, image, allocation);
			allocator = nullptr;
			allocation = nullptr;
			image = nullptr;
			view = {};
		}
	}
}
