#include <iostream>

#include "Daxa.hpp"

class MyUser {
public:
	MyUser(daxa::AppState& app) 
		: device{ daxa::gpu::Device::create() }
		, pipelineCompiler{ this->device->createPipelineCompiler() }
		, queue{ this->device->createCommandQueue({.batchCount = 2 })}
		, swapchain{ this->device->createSwapchain({
			.surface = app.window->getSurface(), 
			.width = app.window->getWidth(), 
			.height = app.window->getHeight(), 
			.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR,
			.additionalUses = VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		})}
		, swapchainImage{ this->swapchain->aquireNextImage() }
		, presentSignal{ this->device->createSignal({}) }
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
				for( int i=0; i<256; i++ )
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
					vec2 aaStep = vec2(1,1) / vec2(cameraData.imgSize.xy) / AA * 2;

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

		this->pipeline = pipelineCompiler->createComputePipeline({ 
			.shaderCI = {
				.source = computeShaderGLSL,
				.stage = VK_SHADER_STAGE_COMPUTE_BIT
			} 
		}).value();

		this->bindingSetAllocator = device->createBindingSetAllocator({ .setLayout = pipeline->getSetLayout(0) });

		// or use them embedded, like a named parameter list:
		this->uniformBuffer = device->createBuffer({
			.size = sizeof(u32) * 2,
			.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		});

		resultImage = device->createImageView({
			.image = device->createImage({
				.extent = { app.window->getWidth(), app.window->getHeight(), 1},
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
				.debugName = "resultImage",
			}),
			.defaultSampler = device->createSampler({}),
			.format = VK_FORMAT_R8G8B8A8_UNORM,
			.debugName = "resultImageView",
		});

		auto cmdList = queue->getCommandList({});
		cmdList->insertImageBarrier({.image = resultImage, .layoutAfter = VK_IMAGE_LAYOUT_GENERAL});
		cmdList->finalize();
		queue->submitBlocking({
			.commandLists = { cmdList }
		});
	}

	void update(daxa::AppState& app) {
		auto cmdList = queue->getCommandList({});

		if (app.window->getWidth() != swapchain->getSize().width || app.window->getHeight() != swapchain->getSize().height) {
			device->waitIdle();
			swapchain->resize(VkExtent2D{ .width = app.window->getWidth(), .height = app.window->getHeight() });
			swapchainImage = swapchain->aquireNextImage();
			resultImage = device->createImageView({
				.image = device->createImage({
					.format = VK_FORMAT_R8G8B8A8_UNORM,
					.extent = { app.window->getWidth(), app.window->getHeight(), 1},
					.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
					.debugName = "resultImage",
				}),
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.defaultSampler = device->createSampler({}),
				.debugName = "resultImageView",
			});
			cmdList->queueImageBarrier({
				.image = resultImage,
				.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
				.layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
			});
		}

		/// ------------ Begin Data Uploading ---------------------

		std::array someBufferdata = { app.window->getWidth(), app.window->getHeight() };
		cmdList->singleCopyHostToBuffer({
			.src = (u8*)someBufferdata.data(),
			.dst = uniformBuffer,
			.region = {
				.size = sizeof(decltype(someBufferdata)),
			},
		});

		// array because we can allways pass multiple barriers at once for driver efficiency
		cmdList->queueMemoryBarrier(daxa::gpu::FULL_MEMORY_BARRIER);
		
		/// ------------ End Data Uploading ---------------------

		cmdList->bindPipeline(pipeline);
		
		auto set = bindingSetAllocator->getSet();
		set->bindBuffer(0, uniformBuffer);
		set->bindImage(1, resultImage, VK_IMAGE_LAYOUT_GENERAL);
		cmdList->bindSet(0, set);

		cmdList->dispatch(app.window->getWidth() / 8 + 1, app.window->getHeight() / 8 + 1);

		cmdList->queueImageBarrier({
			.image = swapchainImage.getImageViewHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
			.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		});
		cmdList->queueImageBarrier({
			.image = resultImage,
			.layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
			.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		});
		cmdList->singleCopyImageToImage({
			.src = resultImage->getImageHandle(),
			.dst = swapchainImage.getImageViewHandle()->getImageHandle(),
		});
		cmdList->queueImageBarrier({
			.image = swapchainImage.getImageViewHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		});
		cmdList->queueImageBarrier({
			.image = resultImage,
			.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
		});

		cmdList->finalize();

		daxa::gpu::SubmitInfo submitInfo;
		submitInfo.commandLists.push_back(std::move(cmdList));
		submitInfo.signalOnCompletion = { &presentSignal, 1 };
		queue->submit(submitInfo);

		queue->present(std::move(swapchainImage), presentSignal);
		swapchainImage = swapchain->aquireNextImage();

		// switches to the next batch. If all available batches are still beeing executed, 
		// this function will block until there is a batch available again.
		// the batches are reclaimed in order.
		queue->nextBatch();

		// reclaim ressources and free ref count handles
		queue->checkForFinishedSubmits();
	}

	void cleanup(daxa::AppState& app) {
		queue->waitIdle();
		queue->checkForFinishedSubmits();
		device->waitIdle();
	}

	~MyUser() {

	}
private:
	daxa::gpu::DeviceHandle device;
	daxa::PipelineCompilerHandle pipelineCompiler;
	daxa::gpu::CommandQueueHandle queue;
	daxa::gpu::SwapchainHandle swapchain;
	daxa::gpu::SwapchainImage swapchainImage;
	daxa::gpu::ImageViewHandle resultImage;
	daxa::gpu::PipelineHandle pipeline;
	daxa::gpu::BindingSetAllocatorHandle bindingSetAllocator;
	daxa::gpu::BufferHandle uniformBuffer;
	double totalElapsedTime = 0.0f;
	daxa::gpu::SignalHandle presentSignal;
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