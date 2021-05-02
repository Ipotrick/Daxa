#pragma once

#include "../DaxaCore.hpp"

#include "../math/Vec.hpp"
#include "../math/Rota2.hpp"
#include "../rendering/ImageManager.hpp"
#include "../rendering/CoordSystem2d.hpp"

namespace daxa {
	struct UISprite {
		Vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Vec3 position;
		Rotation2 rotationVec;
		Vec2 scale{ 1.0f, 1.0f };
		uint32_t imageIndex;
		Vec2 texMin{ 0,0 };
		Vec2 texMax{ 1,1 };
		Vec2 clipMin{ -1.0f, -1.0f };
		Vec2 clipMax{ 1.0f, 1.0f };
		float cornerRounding{ 0.0f };
		uint32_t isMSDF{ false };
		RenderSpace2d drawMode{ RenderSpace2d::Window };
		uint8_t p0;
		uint16_t p0;
	};
}
