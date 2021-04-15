#pragma once

#include "DaxaCore.hpp"

#include "threading/OwningMutex.hpp"
#include "threading/Jobs.hpp"
#include "platform/Window.hpp"
#include "rendering/Rendering.hpp"

namespace daxa {

	struct Camera {
		Vec3 position{ 0,0,3 };
		f32 yaw{ 0.0f };
		f32 pitch{ 0.0f };
		f32 rotation{ 0.0f };
		daxa::Mat4x4 view;
	};

	struct FrameData {
		FrameData(vk::DescriptorSetLayout* descriptorLayout) :
			semaPool{
				[]() { return vkh_old::device.createSemaphore({}); },
				[](vk::Semaphore sem) { vkh_old::device.destroySemaphore(sem,nullptr); },
				[](vk::Semaphore sem) { /* dont need to reset a semaphore */ }
			},
			cmdPool{ vkh_old::device, vk::CommandPoolCreateInfo{.queueFamilyIndex = vkh_old::mainGraphicsQueueFamiltyIndex } },


			fence{ vkh_old::device.createFenceUnique(vk::FenceCreateInfo{}) },
			presentSem{ vkh_old::device.createSemaphoreUnique({}) }
		{ }

		vkh::Pool<vk::Semaphore> semaPool;
		vkh::CommandBufferPool cmdPool;
		vk::UniqueDescriptorPool descPool;
		vk::DescriptorSet descSet;
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

		std::array<FrameData, FRAME_OVERLAP> frames;
	};
}
