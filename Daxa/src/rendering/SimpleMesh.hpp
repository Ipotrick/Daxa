#pragma once

#include <vector>

#include "Rendering.hpp"

#include "../math/Mat.hpp"

namespace daxa {
	struct Vertex {
		Vec3 position;
		Vec3 normal;
		Vec3 color;

		static vkh::VertexDescription getVertexDescription()
		{
			vkh::VertexDiscriptionBuilder builder;
			return builder
				.beginBinding(sizeof(Vertex))
				.setAttribute(VK_FORMAT_R32G32B32_SFLOAT)
				.setAttribute(VK_FORMAT_R32G32B32_SFLOAT)
				.setAttribute(VK_FORMAT_R32G32B32_SFLOAT)
				.build();
		}

		inline static const vkh::VertexDescription INFO{ getVertexDescription() };
	};
	struct SimpleMesh {
		std::vector<Vertex> vertices;

		vkh::Buffer vertexBuffer;
	};
}
