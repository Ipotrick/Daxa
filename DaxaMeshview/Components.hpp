#pragma once

#include "Daxa.hpp"
#include "renderer/RenderContext.hpp"

struct ModelComp {
	std::vector<Primitive> meshes;
};

struct ChildComp {
	daxa::EntityHandle parent = {};
};

struct ParentComp {
	std::vector<daxa::EntityHandle> children = {};
};

struct LightComp {
	f32 strength;
	glm::vec4 color;
};