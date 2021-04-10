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
				.setAttribute(vk::Format::eR32G32B32Sfloat)
				.setAttribute(vk::Format::eR32G32B32Sfloat)
				.setAttribute(vk::Format::eR32G32B32Sfloat)
				.build();
		}

		inline static const vkh::VertexDescription INFO{ getVertexDescription() };
	};
	struct SimpleMesh {
		std::vector<Vertex> vertices;

		Buffer vertexBuffer;
	};

	std::optional<SimpleMesh> loadMeshFromObj(const char* filename);
}
