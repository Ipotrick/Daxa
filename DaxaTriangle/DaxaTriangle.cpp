#include <iostream>

#include "Daxa.hpp"

class MyUser {
public:
	MyUser(daxa::AppState& app) 
		: device{ daxa::Device::create() }
		, queue{ this->device->createCommandQueue({.batchCount = 2 })}
		, swapchain{ this->device->createSwapchain({
			.surface = app.window->getSurface(),
			.width = app.window->getWidth(),
			.height = app.window->getHeight(),
			.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR
		})}
		, swapchainImage{ this->swapchain->aquireNextImage() }
		, presentSignal{ this->device->createSignal({}) }
	{ 

		//char const* vertexShader = R"(
        //    struct PS_INPUT {
        //        float4 position : SV_POSITION;
        //        float4 color : A;
        //    };
//
		//	PS_INPUT main(float3 position : POSITION0, float4 color : COLOR0)
		//	{
		//		PS_INPUT output;
		//		output.color = color;
		//		output.position = float4( position, 1.0f );
		//		return output;
		//	}
		//)";
//
		//char const* fragmentShader = R"(
		//	#include "daxa.hlsl"
//
        //    struct PS_INPUT {
        //        float4 position : SV_POSITION;
        //        float4 color : A;
        //    };
//
		//	struct PS_OUT {
		//		float4 color : SV_Target;
		//	};
//
		//	struct SomeBuffer {
		//		float4 data;
		//	};
		//	DAXA_DEFINE_BA_BUFFER(SomeBuffer)
//
		//	struct PushConstant {
		//		uint bufferId;
        //    };
        //    [[vk::push_constant]] const PushConstant pc;
//
		//	PS_OUT main(PS_INPUT input)
		//	{
		//		//StructuredBuffer<SomeBuffer> buffer = daxa::getBuffer<SomeBuffer>(pc.bufferId);
//
		//		PS_OUT output;
		//		output.color = input.color;
		//		//output.color *= buffer.Load(0).data.b;
		//		return output;
		//	}
		//)";

		// char const* vertexShader = R"(
		// 	float4 main() : SV_POSITION
		// 	{
		// 		return float4(0,0,0,0);
		// 	}
		// )";

		// char const* fragmentShader = R"(
		// 	float4 main() : SV_Target
		// 	{
		// 		return float4(1,1,1,1);
		// 	}
		// )";
		
		char const *compute_src = R"(
			[numthreads(8, 8, 8)] void main(uint3 global_i : SV_DispatchThreadID) {
				// my code!
			}
		)";

		// daxa::GraphicsPipelineBuilder pipelineBuilder;
		// pipelineBuilder
		// 	.addShaderStage({ .source = vertexShader, .stage = VK_SHADER_STAGE_VERTEX_BIT })
		// 	.addShaderStage({ .source = fragmentShader, .stage = VK_SHADER_STAGE_FRAGMENT_BIT })
		// 	// adding a vertex input attribute binding:
		// 	//.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
		// 	//// all added vertex input attributes are added to the previously added vertex input attribute binding
		// 	//.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)			// positions
		// 	//.addVertexInputAttribute(VK_FORMAT_R32G32B32A32_SFLOAT)			// colors
		// 	//.endVertexInputAttributeBinding()
		// 	// location of attachments in a shader are implied by the order they are added in the pipeline builder:
		// 	.addColorAttachment(swapchain->getVkFormat());

		auto pipelineCompiler = device->createPipelineCompiler();
		pipelineCompiler->addShaderSourceRootPath("./Daxa/shaders/");

		// this->pipeline = pipelineCompiler->createGraphicsPipeline(pipelineBuilder).value();
		this->compute_pipeline = pipelineCompiler->createComputePipeline({
			.shaderCI = {
				.source = compute_src,
				.stage = VK_SHADER_STAGE_COMPUTE_BIT,
			},
		}).value();

		// use the explicit create info structs:
		//daxa::BufferInfo bufferCI{
		//	.size = sizeof(float) * 3 * 3 /* positions */ + sizeof(float) * 4 * 3 /* colors */,
		//	.memoryType = daxa::MemoryType::GPU_ONLY,
		//};
		//this->vertexBuffer = device->createBuffer(bufferCI);
//
		//// or use them embedded, like a named parameter list:
		//this->uniformBuffer = device->createBuffer({
		//	.size = sizeof(float) * 4,
		//	.memoryType = daxa::MemoryType::GPU_ONLY,
		//});
	}

	void update(daxa::AppState& app) {
		if (app.window->getWidth() != swapchain->getSize().width || app.window->getHeight() != swapchain->getSize().height) {
			device->waitIdle();
			swapchain->resize(VkExtent2D{ .width = app.window->getWidth(), .height = app.window->getHeight() });
			swapchainImage = swapchain->aquireNextImage();
		}

		auto cmdList = queue->getCommandList({});

		/// ------------ Begin Data Uploading ---------------------


		//std::array vertecies = {
		//	 1.f, 1.f, 0.0f,	1.f, 0.f, 0.f, 1.f,
		//	-1.f, 1.f, 0.0f,	0.f, 1.f, 0.f, 1.f,
		//	 0.f,-1.f, 0.0f,	0.f, 0.f, 1.f, 1.f,
		//};
		//cmdList.singleCopyHostToBuffer(daxa::SingleCopyHostToBufferInfo{
		//	.src = (u8*)vertecies.data(),
		//	.dst = vertexBuffer,
		//	.region = daxa::HostToBufferCopyRegion{
		//		.size = sizeof(decltype(vertecies))
		//	}
		//});
//
		//std::array someBufferdata = { 1.0f , 1.0f , 1.0f ,1.0f };
		//cmdList.singleCopyHostToBuffer(daxa::SingleCopyHostToBufferInfo{
		//	.src = (u8*)someBufferdata.data(),
		//	.dst = uniformBuffer,
		//	.region = daxa::HostToBufferCopyRegion{
		//		.size = sizeof(someBufferdata.size() * sizeof(float))
		//	}
		//});

		// array because we can allways pass multiple barriers at once for driver efficiency
		cmdList.queueImageBarrier({
			.barrier = daxa::FULL_MEMORY_BARRIER,
			.image = device->info(swapchainImage.getImageViewHandle()).image,
			.subRange = device->info(swapchainImage.getImageViewHandle()).subresourceRange,
			.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,						// dont care about previous layout
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,		// set new layout to color attachment optimal
		});
		// array because we can allways pass multiple barriers at once for driver efficiency
		//cmdList.queueMemoryBarrier({
		//	.srcAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,				// wait for writing the vertex buffer
		//	.dstStages = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR,		// the vertex creating must wait
		//});
		
		
		/// ------------ End Data Uploading ---------------------

		cmdList.bindPipeline(compute_pipeline);
		cmdList.dispatch(1, 1, 1);

		cmdList.queueImageBarrier({
			.image = swapchainImage.getImageViewHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
			.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		});
		cmdList.queueImageBarrier({
			.image = render_col_image,
			.layoutBefore = VK_IMAGE_LAYOUT_GENERAL,
			.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		});

		auto render_extent = device->info(render_col_image).image->getVkExtent3D();
		auto swap_extent = device->info(render_col_image).image->getVkExtent3D();
		VkImageBlit blit{
			.srcSubresource = VkImageSubresourceLayers{
				.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.srcOffsets = {
				VkOffset3D{0, 0, 0},
				VkOffset3D{
					static_cast<int32_t>(render_extent.width),
					static_cast<int32_t>(render_extent.height),
					static_cast<int32_t>(render_extent.depth),
				},
			},
			.dstSubresource = VkImageSubresourceLayers{
				.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
				.mipLevel = 0,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
			.dstOffsets = {
				VkOffset3D{0, 0, 0},
				VkOffset3D{
					static_cast<int32_t>(swap_extent.width),
					static_cast<int32_t>(swap_extent.height),
					static_cast<int32_t>(swap_extent.depth),
				},
			},
		};
		cmdList.insertQueuedBarriers();
		vkCmdBlitImage(
			cmdList.getVkCommandBuffer(),
			device->info(render_col_image).image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			device->info(swapchainImage).image->getVkImage(),
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_NEAREST);

		cmdList.queueImageBarrier({
			.image = device->info(swapchainImage).image,
			.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		});
		cmdList.queueImageBarrier({
			device->info(render_col_image).image,
			.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_GENERAL,
		});

		// totalElapsedTime += app.getDeltaTimeSeconds();
		// f32 r = std::cosf((f32)totalElapsedTime * 0.21313f) * 0.3f + 0.5f;
		// f32 g = std::cosf((f32)totalElapsedTime * 0.75454634f) * 0.3f + 0.5f;
		// f32 b = std::cosf((f32)totalElapsedTime) * 0.3f + 0.5f;

		// VkClearValue clear{ .color = VkClearColorValue{.float32 = { r, g, b, 1.0f } } };

		// std::array framebuffer{
		// 	daxa::RenderAttachmentInfo{
		// 		.image = swapchainImage.getImageViewHandle(),
		// 		.clearValue = clear,
		// 		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		// 	}
		// };
		// cmdList.beginRendering(daxa::BeginRenderingInfo{
		// 	.colorAttachments = framebuffer,
		// });
		//
		// cmdList.bindPipeline(pipeline);
		
		//cmdList.pushConstant(uniformBuffer);
		
		//cmdList.bindVertexBuffer(0, vertexBuffer);
		
		// cmdList.draw(3, 1, 0, 0);
		
		// cmdList.endRendering();
//
		// array because we can allways pass multiple barriers at once for driver efficiency
		// cmdList.queueImageBarrier(daxa::ImageBarrier{
		// 	.image = device->info(swapchainImage.getImageViewHandle()).image,
		// 	.subRange = device->info(swapchainImage.getImageViewHandle()).subresourceRange,
		// 	.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		// 	.layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		// });
//
		cmdList.finalize();
//
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

	void cleanup(daxa::AppState& app) {
		queue->waitIdle();
		queue->checkForFinishedSubmits();
		device->waitIdle();
	}

	~MyUser() {

	}
private:
	daxa::DeviceHandle device;
	daxa::CommandQueueHandle queue;
	daxa::SwapchainHandle swapchain;
	daxa::SwapchainImage swapchainImage;
	daxa::PipelineHandle pipeline;
	daxa::PipelineHandle compute_pipeline;
	daxa::BufferHandle vertexBuffer;
	daxa::BufferHandle uniformBuffer;
	daxa::ImageViewHandle render_col_image;
	daxa::SignalHandle presentSignal;
	double totalElapsedTime = 0.0f;
	u32 frame_i = 0;
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