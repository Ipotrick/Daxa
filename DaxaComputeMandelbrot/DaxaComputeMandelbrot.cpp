#include <iostream>

#include "Daxa.hpp"

class MyUser {
public:
	MyUser(daxa::AppState& app) 
		: device{ daxa::gpu::Device::create() }
		, queue{ this->device->createQueue() }
		, swapchain{ this->device->createSwapchain({
			.surface = app.window->getSurface(), 
			.width = app.window->getSize()[0], 
			.height = app.window->getSize()[1], 
			.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
			.additionalUses = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		})}
		, swapchainImage{ this->swapchain->aquireNextImage() }
	{ 

		char const* computeShaderGLSL = R"(
			#version 450
			#extension GL_KHR_vulkan_glsl : enable

			layout(local_size_x = 8, local_size_y = 8) in;

			layout(set = 0, binding = 0) uniform CameraData {
				ivec2 imgSize;
			} cameraData;

			layout (set = 0, binding = 1, rgba8) uniform writeonly image2D resultImage;

			float mandelbrot(in vec2 c) {const float B = 256.0;
				float l = 0.0;
				vec2 z  = vec2(0.0);
				for( int i=0; i<1024; i++ )
				{
					z = vec2( z.x*z.x - z.y*z.y, 2.0*z.x*z.y ) + c;
					if( dot(z,z)>(B*B) ) break;
					l += 1.0;
				}

				if( l>511.0 ) return 0.0;

				float sl = l - log2(log2(dot(z,z))) + 4.0;

				l = mix( l, sl, 1.0f );

				return l;
			}

			void main()
			{
				if (gl_GlobalInvocationID.x < cameraData.imgSize.x && gl_GlobalInvocationID.y < cameraData.imgSize.y) {
					vec2 smoothCoord = vec2(gl_GlobalInvocationID.xy) / vec2(cameraData.imgSize.xy) * 2.0f - vec2(1,1);
					smoothCoord *= 2.0f;

					vec4 colorAcc = vec4(0,0,0,0);

					const int AA = 4;
					vec2 aaStep = vec2(1,1) / vec2(cameraData.imgSize.xy) / AA * 4;

					for (int x = 0; x < AA; x++) {
						for (int y = 0; y < AA; y++) {
							vec2 subCoord = smoothCoord + aaStep * vec2(x,y);

							colorAcc += vec4(1.0f,0.8f,0.8f,1.0f) * mandelbrot(subCoord)*(1.0f/80.0f);
						}
					}
					colorAcc *= 1.0f / (AA*AA);

					ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
					imageStore(resultImage, coord, colorAcc);
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

		resultImage = device->createImage2d({
			.width = app.window->getSize()[0],
			.height = app.window->getSize()[1],
			.format = VK_FORMAT_R8G8B8A8_UNORM,
			.imageUsage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		});

		auto cmdList = device->getEmptyCommandList();
		cmdList->begin();
		cmdList->insertImageBarrier({.image = resultImage, .layoutAfter = VK_IMAGE_LAYOUT_GENERAL});
		cmdList->end();
		queue->submitBlocking({
			.commandLists = { cmdList }
		});

		for (int i = 0; i < 3; i++) {
			frames.push_back(PerFrameData{ .presentSignal = device->createSignal(), .timeline = device->createTimelineSemaphore(), .timelineCounter = 0 });
		}
	}

	void update(daxa::AppState& app) {
		//printf("update, dt: %f\n", app.getDeltaTimeSeconds());

		auto cmdList = device->getEmptyCommandList();

		cmdList->begin();

		if (app.window->getSize()[0] != swapchain->getSize().width || app.window->getSize()[1] != swapchain->getSize().height) {
			device->waitIdle();
			swapchain->resize(VkExtent2D{ .width = app.window->getSize()[0], .height = app.window->getSize()[1] });
			swapchainImage = swapchain->aquireNextImage();
			resultImage = device->createImage2d({
				.width = app.window->getSize()[0],
				.height = app.window->getSize()[1],
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.imageUsage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			});
			cmdList->insertImageBarrier({.image = resultImage, .layoutAfter = VK_IMAGE_LAYOUT_GENERAL, .waitingStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR});
		}

		auto* currentFrame = &frames.front();


		/// ------------ Begin Data Uploading ---------------------

		std::array someBufferdata = { app.window->getSize()[0], app.window->getSize()[1] };
		cmdList->copyHostToBuffer(daxa::gpu::HostToBufferCopyInfo{
			.src = someBufferdata.data(),
			.dst = uniformBuffer,
			.size = sizeof(decltype(someBufferdata)),
		});

		// array because we can allways pass multiple barriers at once for driver efficiency
		cmdList->insertMemoryBarrier(daxa::gpu::MemoryBarrier{
			.awaitedAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,				// wait for writing the vertex buffer
			.waitingStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,	// the compute shader must wait
		});
		
		/// ------------ End Data Uploading ---------------------

		cmdList->bindPipeline(pipeline);
		
		auto set = bindingSetAllocator->getSet();
		set->bindBuffer(0, uniformBuffer);
		set->bindImage(1, resultImage, VK_IMAGE_LAYOUT_GENERAL);
		cmdList->bindSet(0, set);

		cmdList->dispatch(app.window->getSize()[0] / 8 + 1, app.window->getSize()[1] / 8 + 1);

		cmdList->copyImageToImageSynced({
			.src = resultImage,
			.srcFinalLayout = VK_IMAGE_LAYOUT_GENERAL,
			.dst = swapchainImage.getImageHandle(),
			.dstFinalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			.size = resultImage->getVkExtent(),
		});

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
	daxa::gpu::ImageHandle resultImage;
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
		daxa::Application<MyUser> app{ 640, 640, "Daxa Compute Sample"};
		app.run();
	}

	daxa::cleanup();
}