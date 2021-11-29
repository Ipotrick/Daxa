#pragma once

#include <vector>
#include <functional>

#include "GUIDrawContext.hpp"
#include "GUIDrawUtil.hpp"
#include "GUIElement.hpp"

namespace daxa {
	namespace gui {

		struct Root {
			std::function<void(Root&, u32)> onUpdate;
			Sizing sizing;
			Placing placing;
			u32 child{ INVALID_ELEMENT_ID };
		};
	}
}
