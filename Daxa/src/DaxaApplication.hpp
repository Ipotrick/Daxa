#pragma once

#include "DaxaCore.hpp"

#include "threading/OwningMutex.hpp"
#include "threading/Jobs.hpp"
#include "platform/Window.hpp"
#include "rendering/Rendering.hpp"

namespace vkh {
	class DescriptorSetPool {
	public:
		vk::UniqueDescriptorSetLayout layout;
		vk::UniqueDescriptorPool pool;
	};
}

namespace daxa {

	struct Camera {
		Vec3 position{ 0,0,3 };
		f32 yaw{ 0.0f };
		f32 pitch{ 0.0f };
		f32 rotation{ 0.0f };
		daxa::Mat4x4 view;
		daxa::Mat4x4 proj;
	};

	struct GPUData {
		daxa::Mat4x4 model;
		daxa::Mat4x4 view;
		daxa::Mat4x4 proj;
	};

	struct TestData {
		daxa::Vec4 color{ 1,0,0,1 };
	};

	struct FrameData {
		FrameData(vkh::DescriptorSetLayoutCache* layoutCache) :
			semaPool{
				[]() { return VulkanContext::device.createSemaphore({}); },
				[](vk::Semaphore sem) { VulkanContext::device.destroySemaphore(sem,nullptr); },
				[](vk::Semaphore sem) { /* dont need to reset a semaphore */ }
			},
			cmdPool{ VulkanContext::device, vk::CommandPoolCreateInfo{.queueFamilyIndex = VulkanContext::mainGraphicsQueueFamiltyIndex } },
			gpuDataBuffer{ createBuffer(sizeof(GPUData), vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU) },
			testBuffer{ createBuffer(sizeof(TestData), vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU) },

			fence{ VulkanContext::device.createFenceUnique(vk::FenceCreateInfo{}) },
			presentSem{ VulkanContext::device.createSemaphoreUnique({}) },
			descAlloc{ VulkanContext::device }
		{
			descSet = vkh::DescriptorSetBuilder(&descAlloc, layoutCache)
				.addBufferBinding(
					{ .binding = 0, .descriptorType = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eVertex },
					{ .buffer = gpuDataBuffer.buffer , .range = sizeof(GPUData) })
				.build();

			testSet = vkh::DescriptorSetBuilder(&descAlloc, layoutCache)
				.addBufferBinding(
					{ .binding = 0, .descriptorType = vk::DescriptorType::eUniformBuffer , .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eFragment },
					{ .buffer = testBuffer.buffer, .range= sizeof(TestData) })
				.build();
		}

		vkh::Pool<vk::Semaphore> semaPool;
		vkh::CommandBufferAllocator cmdPool;
		Buffer gpuDataBuffer;
		Buffer testBuffer;
		vkh::DescriptorAllocator descAlloc;
		vk::DescriptorSet descSet;
		vk::DescriptorSet testSet;
		vk::UniqueFence fence;
		vk::UniqueSemaphore presentSem;
	};

	struct MeshPushConstants {
		daxa::Mat4x4 renderMatrix;
		daxa::Vec4 data;
	};

	class Application {
	public:
		Application(std::string name, u32 width, u32 height);

		~Application();

		void update(f32 dt);

		void draw();

		std::unique_ptr<OwningMutex<Window>> windowMutex; 
	private:

		void init_default_renderpass();

		void init_framebuffers();

		void init_pipelines();

		void init_mesh_pipeline();

		std::optional<vkh::Pipeline> createMeshPipeline();

		void cleanup();

		void loadMeshes();

		void uploadMesh(SimpleMesh& mesh);

		daxa::ShaderHotLoader testHotLoader{ {"Daxa/shaders/mesh.vert","Daxa/shaders/mesh.frag"},
			[&]() {
				std::cout << "hotload\n";
				VulkanContext::device.waitIdle();
				auto pipeOpt = createMeshPipeline();
				if (pipeOpt) {
					meshPipeline = std::move(pipeOpt.value());
				}
			} 
		};

		// PERSISTENT DATA:

		vkh::DescriptorSetLayoutCache descLayoutCache;

		std::vector<vk::UniqueFramebuffer> framebuffers;

		u32 _frameNumber{ 0 };

		SimpleMesh monkeyMesh;
		vkh::Pipeline meshPipeline;

		vk::UniqueRenderPass mainRenderpass;

		vk::DescriptorSetLayout descLayout;
		vk::DescriptorSetLayout testLayout;

		Camera camera;
		bool bCameraControll{ false };

		// PER FRAME DATA:

		inline static constexpr u32 FRAME_OVERLAP{ 2 };

		std::vector<FrameData> frames;
	};
}
