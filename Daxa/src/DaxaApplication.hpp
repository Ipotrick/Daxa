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

	struct FrameData {
		FrameData(vk::DescriptorSetLayout* descriptorLayout) :
			semaPool{
				[]() { return vkh_old::device.createSemaphore({}); },
				[](vk::Semaphore sem) { vkh_old::device.destroySemaphore(sem,nullptr); },
				[](vk::Semaphore sem) { /* dont need to reset a semaphore */ }
			},
			cmdPool{ vkh_old::device, vk::CommandPoolCreateInfo{.queueFamilyIndex = vkh_old::mainGraphicsQueueFamiltyIndex } },
			gpuDataBuffer{ createBuffer(sizeof(GPUData), vk::BufferUsageFlagBits::eUniformBuffer, VMA_MEMORY_USAGE_CPU_TO_GPU) },

			fence{ vkh_old::device.createFenceUnique(vk::FenceCreateInfo{}) },
			presentSem{ vkh_old::device.createSemaphoreUnique({}) }
		{ 
			std::vector sizes{
				vk::DescriptorPoolSize{ .type = vk::DescriptorType::eUniformBuffer, .descriptorCount = 10 }
			};

			descPool = vkh_old::device.createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo{
				.maxSets = 10,
				.poolSizeCount = static_cast<u32>(sizes.size()),
				.pPoolSizes = sizes.data()
			});

			descSet = std::move(vkh_old::device.allocateDescriptorSetsUnique(vk::DescriptorSetAllocateInfo{
				.descriptorPool = descPool.get(),
				.descriptorSetCount = 1,
				.pSetLayouts = descriptorLayout,
			}).front());

			// link uniform buffer with descriptor set:
			vk::DescriptorBufferInfo binfo{
				.buffer = gpuDataBuffer.buffer,
				.offset = 0,
				.range = sizeof(GPUData)
			};

			vk::WriteDescriptorSet write{
				.dstSet = descSet.get(),
				.dstBinding = 0,
				.descriptorCount = 1,
				.descriptorType = vk::DescriptorType::eUniformBuffer,
				.pBufferInfo = &binfo,
			};

			vkh_old::device.updateDescriptorSets({ write }, {});
		}

		vkh::Pool<vk::Semaphore> semaPool;
		vkh::CommandBufferPool cmdPool;
		Buffer gpuDataBuffer;
		vk::UniqueDescriptorSetLayout descLayout;
		vk::UniqueDescriptorPool descPool;
		vk::UniqueDescriptorSet descSet;
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

		void cleanup();

		void loadMeshes();

		void uploadMesh(SimpleMesh& mesh);

		// PERSISTENT DATA:

		std::vector<vk::UniqueFramebuffer> framebuffers;

		u32 _frameNumber{ 0 };

		SimpleMesh monkeyMesh;
		vkh::Pipeline meshPipeline;

		vk::UniqueRenderPass mainRenderpass;

		vk::UniqueDescriptorSetLayout descLayout{

		};

		Camera camera;
		bool bCameraControll{ false };

		// PER FRAME DATA:

		inline static constexpr u32 FRAME_OVERLAP{ 2 };

		std::vector<FrameData> frames;
	};
}
