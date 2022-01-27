#pragma once

#include "Daxa.hpp"

struct Primitive {
	u32 indexCount = 0;
	daxa::gpu::BufferHandle indiexBuffer = {};
	daxa::gpu::BufferHandle vertexPositions = {};
	daxa::gpu::BufferHandle vertexUVs = {};
	daxa::gpu::BufferHandle vertexNormals = {};
	daxa::gpu::BufferHandle vertexTangents = {};
	daxa::gpu::ImageViewHandle albedoTexture = {};
	daxa::gpu::ImageViewHandle normalTexture = {};
};

struct ModelComp {
	std::vector<Primitive> meshes;
};

struct ChildComp {
	daxa::EntityHandle parent = {};
};

struct LightComp {
	f32 strength;
	glm::vec4 color;
};