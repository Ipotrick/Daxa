#pragma once

#include "../DaxaCore.hpp"

#include "../math/Mat.hpp"

namespace daxa {
	class Camera2d {
		Vec2 position{ 0,0 };
		Vec2 frustumBend{ 1,1 };
		float zoom{ 1 };
		float rotation{ 0 };
	};
}
