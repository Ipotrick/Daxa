#include <iostream>

#include "Daxa.hpp"

class MyUser {
public:
	MyUser(daxa::AppState& app) 
		: device{ daxa::gpu::Device::create() }
		, queue{ this->device->createQueue() }
		, swapchain{ this->device->createSwapchain(app.window->getSurface(), app.window->getSize()[0], app.window->getSize()[1], VK_PRESENT_MODE_IMMEDIATE_KHR)}
		, swapchainImage{ this->swapchain->aquireNextImage() }
	{ 

		char const* computeShaderGLSL = R"(
			#version 450
			#extension GL_KHR_vulkan_glsl : enable

			layout(local_size_x = 64, local_size_y = 64) in;

			layout(set = 0, binding = 0) uniform CameraData {
				uint imageWidth;
				uint imageHeight;
			} cameraData;

			layout (set = 0, binding = 1, rgba8) uniform writeonly image2D resultImage;

			void main()
			{
				if (gl_GlobalInvocationID.x < cameraData.imageWidth && gl_GlobalInvocationID.y < cameraData.imageHeight) {

					imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), vec4(1.0f,0.66f,0.33f,1.0f));
				}
			}
		)";

		daxa::gpu::ShaderModuleHandle computeShader = device->tryCreateShderModuleFromGLSL(
			computeShaderGLSL,
			VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT
		).value();

		this->pipeline = device->createComputePipeline(computeShader);

		this->bindingSetAllocator = device->createBindingSetAllocator(pipeline->getSetDescription(0));

		// or use them embedded, like a named parameter list:
		this->uniformBuffer = device->createBuffer({
			.size = sizeof(u32) * 2,
			.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		});

		for (int i = 0; i < 3; i++) {
			frames.push_back(PerFrameData{ .presentSignal = device->createSignal(), .timeline = device->createTimelineSemaphore(), .timelineCounter = 0 });
		}
	}

	void update(daxa::AppState& app) {
		//printf("update, dt: %f\n", app.getDeltaTimeSeconds());

		if (app.window->getSize()[0] != swapchain->getSize().width || app.window->getSize()[1] != swapchain->getSize().height) {
			device->waitIdle();
			swapchain->resize(VkExtent2D{ .width = app.window->getSize()[0], .height = app.window->getSize()[1] });
			swapchainImage = swapchain->aquireNextImage();
		}

		auto* currentFrame = &frames.front();

		auto cmdList = device->getEmptyCommandList();

		cmdList->begin();


		/// ------------ Begin Data Uploading ---------------------

		std::array someBufferdata = { app.window->getSize()[0], app.window->getSize()[1] };
		cmdList->copyHostToBuffer(daxa::gpu::HostToBufferCopyInfo{
			.src = someBufferdata.data(),
			.dst = uniformBuffer,
			.size = sizeof(decltype(someBufferdata)),
		});

		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array imgBarrier0 = { daxa::gpu::ImageBarrier{
			.waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,	// as we write to the image in the frag shader we need to make sure its finished transitioning the layout
			.image = swapchainImage.getImageHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,						// dont care about previous layout
			.layoutAfter = VK_IMAGE_LAYOUT_GENERAL,							// set new layout to general, so it can be used as a storage image in the compute shader
		} };
		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array memBarrier0 = { daxa::gpu::MemoryBarrier{
			.awaitedAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,				// wait for writing the vertex buffer
			.waitingStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,	// the compute shader must wait
		} };
		cmdList->insertBarriers(memBarrier0, {}, imgBarrier0);
		
		/// ------------ End Data Uploading ---------------------

		cmdList->bindPipeline(pipeline);
		
		auto set = bindingSetAllocator->getSet();
		set->bindBuffer(0, uniformBuffer);
		set->bindImage(1, swapchainImage.getImageHandle(), VK_IMAGE_LAYOUT_GENERAL);
		cmdList->bindSet(0, set);

		cmdList->dispatch(app.window->getSize()[0] / 64 + 1, app.window->getSize()[1] / 64 + 1);

		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array imgBarrier1 = { daxa::gpu::ImageBarrier{
			.awaitedStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
			.image = swapchainImage.getImageHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
			.layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		} };
		cmdList->insertBarriers({}, {}, imgBarrier1);

		cmdList->end();

		// "++currentFrame->finishCounter " is the value that will be set to the timeline when the execution is finished, basicly incrementing it 
		// the timeline is the counter we use to see if the frame is finished executing on the gpu later.
		std::array signalTimelines = { std::tuple{ currentFrame->timeline, ++currentFrame->timelineCounter } };

		daxa::gpu::SubmitInfo submitInfo;
		submitInfo.commandLists.push_back(std::move(cmdList));
		submitInfo.signalOnCompletion = { &currentFrame->presentSignal, 1 };
		submitInfo.signalTimelines = signalTimelines;
		queue->submit(submitInfo);

		queue->present(std::move(swapchainImage), currentFrame->presentSignal);
		swapchainImage = swapchain->aquireNextImage();

		// we get the next frame context
		auto frameContext = std::move(frames.back());
		frames.pop_back();
		frames.push_front(std::move(frameContext));
		currentFrame = &frames.front();
		queue->checkForFinishedSubmits();

		// we wait on the gpu to finish executing the frame
		// as we have two frame contexts we are actually waiting on the previous frame to complete. 
		// if you only have one frame in flight you can just wait on the frame to finish here too.
		currentFrame->timeline->wait(currentFrame->timelineCounter);
	}

	void cleanup(daxa::AppState& app) {
		queue->waitForFlush();
		queue->checkForFinishedSubmits();
		device->waitIdle();
		frames.clear();
	}

	~MyUser() {

	}
private:
	daxa::gpu::DeviceHandle device;
	daxa::gpu::QueueHandle queue;
	daxa::gpu::SwapchainHandle swapchain;
	daxa::gpu::SwapchainImage swapchainImage;
	daxa::gpu::PipelineHandle pipeline;
	daxa::gpu::BindingSetAllocatorHandle bindingSetAllocator;
	daxa::gpu::BufferHandle uniformBuffer;
	double totalElapsedTime = 0.0f;
	struct PerFrameData {
		daxa::gpu::SignalHandle presentSignal;
		daxa::gpu::TimelineSemaphoreHandle timeline;
		u64 timelineCounter = 0;
	};
	std::deque<PerFrameData> frames;
};

int main()
{
	daxa::initialize();

	{
		daxa::Application<MyUser> app{ 1000, 1000, "Daxa Triangle Sample"};
		app.run();
	}

	daxa::cleanup();
}