#include <iostream>

#include "Daxa.hpp"

class MyUser {
public:
	MyUser(daxa::AppState& app) 
		: device{ daxa::Device::create() }
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
		char const* computeShaderHLSL = R"(
			struct CameraData {
				int2 imgSize;
			};
			[[vk::binding(0, 0)]]
			ConstantBuffer<CameraData> cameraData;

			[[vk::binding(1,0)]]
			RWTexture2D<float4> resultImage;

			float mandelbrot(float2 c) {const float B = 256.0;
				float l = 0.0;
				float2 z  = float2(0.0, 0.0);
				for( int i=0; i<256; i++ )
				{
					z = float2( z.x*z.x - z.y*z.y, 2.0*z.x*z.y ) + c;
					if( dot(z,z)>(B*B) ) break;
					l += 1.0;
				}

				if( l>511.0 ) return 0.0;

				float sl = l - log2(log2(dot(z,z))) + 4.0;

				l = lerp( l, sl, 1.0f );

				return l;
			}

			[numthreads(8,8,8)]
			void main(uint3 threadId : SV_DispatchThreadID)
			{
				if (threadId.x < cameraData.imgSize.x && threadId.y < cameraData.imgSize.y) {
					float2 smoothCoord = float2(threadId.xy) / float2(cameraData.imgSize.xy) * 2.0f - float2(1,1);
					smoothCoord *= 2.0f;

					float4 colorAcc = float4(0,0,0,0);

					const int AA = 4;
					float2 aaStep = float2(1,1) / float2(cameraData.imgSize.xy) / AA * 2;

					for (int x = 0; x < AA; x++) {
						for (int y = 0; y < AA; y++) {
							float2 subCoord = smoothCoord + aaStep * float2(x,y);

							colorAcc += float4(1.0f,0.8f,0.8f,1.0f) * mandelbrot(subCoord)*(1.0f/80.0f);
						}
					}
					colorAcc *= 1.0f / (AA*AA);

					int2 coord = int2(threadId.xy);
					resultImage[coord] = colorAcc;
					//imageStore(resultImage, coord, colorAcc);
				}
			}
		)";

		this->pipeline = pipelineCompiler->createComputePipeline({ 
			.shaderCI = {
				.source = computeShaderHLSL,
				.shaderLang = daxa::ShaderLang::HLSL,
				.stage = VK_SHADER_STAGE_COMPUTE_BIT,
			} 
		}).value();

		this->bindingSetAllocator = device->createBindingSetAllocator({ .setLayout = pipeline->getSetLayout(0) });

		// or use them embedded, like a named parameter list:
		this->uniformBuffer = device->createBuffer({
			.size = sizeof(u32) * 2,
			//.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			//.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		});

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

		auto cmdList = queue->getCommandList({});
		cmdList.queueImageBarrier({.image = resultImage, .layoutAfter = VK_IMAGE_LAYOUT_GENERAL});
		cmdList.finalize();
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
			cmdList.queueImageBarrier({
				.image = resultImage,
				.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
				.layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
			});
		}

		/// ------------ Begin Data Uploading ---------------------

		std::array someBufferdata = { app.window->getWidth(), app.window->getHeight() };
		cmdList.singleCopyHostToBuffer({
			.src = reinterpret_cast<u8*>(someBufferdata.data()),
			.dst = uniformBuffer,
			.region = {
				.size = sizeof(decltype(someBufferdata)),
			},
		});

		// array because we can allways pass multiple barriers at once for driver efficiency
		cmdList.queueMemoryBarrier(daxa::FULL_MEMORY_BARRIER);
		
		/// ------------ End Data Uploading ---------------------

		cmdList.bindPipeline(pipeline);
		
		auto set = bindingSetAllocator->getSet();
		set->bindBuffer(0, uniformBuffer);
		set->bindImage(1, resultImage, VK_IMAGE_LAYOUT_GENERAL);
		cmdList.bindSet(0, set);

		cmdList.dispatch(app.window->getWidth() / 8 + 1, app.window->getHeight() / 8 + 1);

		cmdList.queueImageBarrier({
			.image = swapchainImage.getImageViewHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
			.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		});
		cmdList.queueImageBarrier({
			.image = resultImage,
			.layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
			.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		});
		cmdList.singleCopyImageToImage({
			.src = resultImage->getImageHandle(),
			.dst = swapchainImage.getImageViewHandle()->getImageHandle(),
		});
		cmdList.queueImageBarrier({
			.image = swapchainImage.getImageViewHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		});
		cmdList.queueImageBarrier({
			.image = resultImage,
			.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
		});

		cmdList.finalize();

		daxa::SubmitInfo submitInfo;
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

	void cleanup(daxa::AppState&) {
		queue->waitIdle();
		queue->checkForFinishedSubmits();
		device->waitIdle();
	}

	~MyUser() {

	}
private:
	daxa::DeviceHandle device;
	daxa::PipelineCompilerHandle pipelineCompiler;
	daxa::CommandQueueHandle queue;
	daxa::SwapchainHandle swapchain;
	daxa::SwapchainImage swapchainImage;
	daxa::ImageViewHandle resultImage;
	daxa::PipelineHandle pipeline;
	daxa::BindingSetAllocatorHandle bindingSetAllocator;
	daxa::BufferHandle uniformBuffer;
	daxa::SignalHandle presentSignal;
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