#pragma once

#include "Daxa.hpp"

struct ModelComp {
	daxa::gpu::BufferHandle indiexBuffer = {};
	daxa::gpu::BufferHandle vertexPositions = {};
	daxa::gpu::BufferHandle vertexUVs = {};
	daxa::gpu::ImageHandle image = {};
	u32 indexCount = 0;
};

struct ChildComp {
	daxa::EntityHandle parent = {};
};