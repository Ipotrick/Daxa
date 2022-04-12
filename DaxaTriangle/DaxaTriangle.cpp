#include <iostream>

#include "Daxa.hpp"

class MyUser {
public:
	MyUser(daxa::AppState& app) 
		: device{ daxa::Device::create() }
		, queue{ this->device->createQueue({.batchCount = 2 })}
		, swapchain{ this->device->createSwapchain({
			.surface = app.window->getSurface(),
			.width = app.window->getWidth(),
			.height = app.window->getHeight(),
			.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR
		})}
		, swapchainImage{ this->swapchain->aquireNextImage() }
		, presentSignal{ this->device->createSignal({}) }
	{ 

		char const* vertexShaderGLSL = R"(
			#version 450
			#extension GL_KHR_vulkan_glsl : enable

			layout(location = 0) in vec3 position;
			layout(location = 1) in vec4 color;

			layout(location = 10) out vec4 v_color;

			void main()
			{
				v_color = color;
				gl_Position = vec4(position, 1.0f);
			}
		)";

		char const* fragmentShaderGLSL = R"(
			#version 450
			#extension GL_KHR_vulkan_glsl : enable

			layout(location = 10) in vec4 v_color;

			layout (location = 0) out vec4 outFragColor;

			layout(set = 0, binding = 0) uniform SomeBuffer {
				vec4 data;
			} someBuffer;

			void main()
			{
				vec4 color = v_color;
				color *= someBuffer.data.b;
				outFragColor = color;
			}
		)";

		daxa::ShaderModuleHandle vertexShader = device->createShaderModule({
			.sourceToGLSL = vertexShaderGLSL,
			.stage = VK_SHADER_STAGE_VERTEX_BIT
		}).value();

		daxa::ShaderModuleHandle fragmenstShader = device->createShaderModule({
			.sourceToGLSL = fragmentShaderGLSL,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT
		}).value();

		daxa::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder
			.addShaderStage(vertexShader)
			.addShaderStage(fragmenstShader)
			// adding a vertex input attribute binding:
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			// all added vertex input attributes are added to the previously added vertex input attribute binding
			.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)			// positions
			.addVertexInputAttribute(VK_FORMAT_R32G32B32A32_SFLOAT)			// colors
			// location of attachments in a shader are implied by the order they are added in the pipeline builder:
			.addColorAttachment(swapchain->getVkFormat());

		this->pipeline = device->createGraphicsPipeline(pipelineBuilder);

		this->bindingSetAllocator = device->createBindingSetAllocator({ .setDescription = pipeline->getSetDescription(0) });

		// use the explicit create info structs:
		daxa::BufferCreateInfo bufferCI{
			.size = sizeof(float) * 3 * 3 /* positions */ + sizeof(float) * 4 * 3 /* colors */,
			.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		};
		this->vertexBuffer = device->createBuffer(bufferCI);

		// or use them embedded, like a named parameter list:
		this->uniformBuffer = device->createBuffer({
			.size = sizeof(float) * 4,
			.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		});
	}

	void update(daxa::AppState& app) {
		if (app.window->getWidth() != swapchain->getSize().width || app.window->getHeight() != swapchain->getSize().height) {
			device->waitIdle();
			swapchain->resize(VkExtent2D{ .width = app.window->getWidth(), .height = app.window->getHeight() });
			swapchainImage = swapchain->aquireNextImage();
		}

		auto cmdList = device->getCommandList();

		/// ------------ Begin Data Uploading ---------------------


		std::array vertecies = {
			 1.f, 1.f, 0.0f,	1.f, 0.f, 0.f, 1.f,
			-1.f, 1.f, 0.0f,	0.f, 1.f, 0.f, 1.f,
			 0.f,-1.f, 0.0f,	0.f, 0.f, 1.f, 1.f,
		};
		cmdList.copyHostToBuffer(daxa::HostToBufferCopyInfo{
			.src = vertecies.data(),
			.dst = vertexBuffer,
			.size = sizeof(decltype(vertecies))
		});

		std::array someBufferdata = { 1.0f , 1.0f , 1.0f ,1.0f };
		cmdList.copyHostToBuffer(daxa::HostToBufferCopyInfo{
			.src = someBufferdata.data(),
			.dst = uniformBuffer,
			.size = someBufferdata.size() * sizeof(float)
		});

		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array imgBarrier0 = { daxa::ImageBarrier{
			.dstStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,	// as we write to the image in the frag shader we need to make sure its finished transitioning the layout
			.image = swapchainImage.getImageHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,						// dont care about previous layout
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,		// set new layout to color attachment optimal
		} };
		// array because we can allways pass multiple barriers at once for driver efficiency
		std::array memBarrier0 = { daxa::MemoryBarrier{
			.srcAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,				// wait for writing the vertex buffer
			.dstStages = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR,		// the vertex creating must wait
		} };
		cmdList.insertBarriers(memBarrier0, imgBarrier0);
		
		
		/// ------------ End Data Uploading ---------------------


		totalElapsedTime += app.getDeltaTimeSeconds();
		f32 r = std::cosf((f32)totalElapsedTime * 0.21313f) * 0.3f + 0.5f;
		f32 g = std::cosf((f32)totalElapsedTime * 0.75454634f) * 0.3f + 0.5f;
		f32 b = std::cosf((f32)totalElapsedTime) * 0.3f + 0.5f;

		VkClearValue clear{ .color = VkClearColorValue{.float32 = { r, g, b, 1.0f } } };

		std::array framebuffer{
			daxa::RenderAttachmentInfo{
				.image = swapchainImage.getImageHandle(),
				.clearValue = clear,
			}
		};
		cmdList.beginRendering(daxa::BeginRenderingInfo{
			.colorAttachments = framebuffer,
		});
		
		cmdList.bindPipeline(pipeline);
		
		auto set = bindingSetAllocator->getSet();
		set->bindBuffer(0, uniformBuffer);
		cmdList.bindSet(0, set);
		
		cmdList.bindVertexBuffer(0, vertexBuffer);
		
		cmdList.draw(3, 1, 0, 0);
		
		cmdList.endRendering();

		// array because we can allways pass multiple barriers at once for driver efficiency
		cmdList.insertImageBarrier(daxa::ImageBarrier{
			.image = swapchainImage.getImageHandle(),
			.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
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

	void cleanup(daxa::AppState& app) {
		queue->waitIdle();
		queue->checkForFinishedSubmits();
		device->waitIdle();
	}

	~MyUser() {

	}
private:
	daxa::DeviceHandle device;
	daxa::QueueHandle queue;
	daxa::SwapchainHandle swapchain;
	daxa::SwapchainImage swapchainImage;
	daxa::PipelineHandle pipeline;
	daxa::BindingSetAllocatorHandle bindingSetAllocator;
	daxa::BufferHandle vertexBuffer;
	daxa::BufferHandle uniformBuffer;
	daxa::SignalHandle presentSignal;
	double totalElapsedTime = 0.0f;
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