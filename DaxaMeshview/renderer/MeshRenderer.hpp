#pragma once

#include "Daxa.hpp"

#include "RenderContext.hpp"
#include "../Components.hpp"

struct GlobalData {
	glm::mat4 vp;
	glm::mat4 view;
};

class MeshRenderer {
public:

    void init(RenderContext& renderCTX) {
		auto vertexShader = renderCTX.device->tryCreateShderModuleFromFile(
			"./DaxaMeshview/renderer/g_pass.vert",
			VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT
		);
		if (vertexShader.isErr()) {
			std::cout << "could not load vertex shader due to: " << vertexShader.message() << std::endl;
		}

		auto fragmenstShader = renderCTX.device->tryCreateShderModuleFromFile(
			"./DaxaMeshview/renderer/g_pass.frag",
			VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT
		);
		if (vertexShader.isErr()) {
			std::cout << "could not load fragment shader due to: " << vertexShader.message() << std::endl;
		}

		daxa::gpu::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder
			.addShaderStage(vertexShader.value())
			.addShaderStage(fragmenstShader.value())
			.setDebugName("mesh render pipeline")
			.configurateDepthTest({.enableDepthTest = true, .enableDepthWrite = true, .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT})
			// adding a vertex input attribute binding:
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			// all added vertex input attributes are added to the previously added vertex input attribute binding
			.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)			// positions
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			.addVertexInputAttribute(VK_FORMAT_R32G32_SFLOAT)				// uvs
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)
			// location of attachments in a shader are implied by the order they are added in the pipeline builder:
			.addColorAttachment(renderCTX.swapchain->getVkFormat())
			.addColorAttachment(renderCTX.normalsImage->getVkViewFormat())
			.setRasterization({
				.cullMode = VK_CULL_MODE_BACK_BIT,
			});

		this->pipeline = renderCTX.device->createGraphicsPipeline(pipelineBuilder);

		this->globalSetAlloc = renderCTX.device->createBindingSetAllocator({
			.setDescription = pipeline->getSetDescription(0),
			.debugName = "mesh renderer global set allocator",
		});
		this->perDrawSetAlloc = renderCTX.device->createBindingSetAllocator({
			.setDescription = pipeline->getSetDescription(1),
			.debugName = "mesh renderer per draw set allocator",
		});

        this->globalDataBufffer = renderCTX.device->createBuffer({
            .size = sizeof(GlobalData),
            .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
        });

		this->transformsBuffer = renderCTX.device->createBuffer({
            .size = sizeof(glm::mat4) * 128,
            .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
		});

        this->globalSet = globalSetAlloc->getSet();
        globalSet->bindBuffer(0, this->globalDataBufffer);
		globalSet->bindBuffer(1, this->transformsBuffer);
    }

    struct DrawMesh {
        glm::mat4 transform = {};
		Primitive* prim = {};
    };

	void setCamera(daxa::gpu::CommandListHandle& cmd, glm::mat4 const& vp, glm::mat4 const& view) {
		globData.vp = vp;
		globData.view = view;
	}

    void render(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawMesh>& draws) {

		cmd->insertImageBarrier({
			.image = renderCTX.normalsImage,
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.waitingStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
		});

		if (!dummyTexture) {
			dummyTexture = renderCTX.device->createImage2d({
				.width = 1,
				.height = 1,
				.imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
				.sampler = renderCTX.device->createSampler({}),
			});

			u32 pink = 0xFFFF00FF;

			cmd->copyHostToImageSynced({
				.dst = dummyTexture,
				.dstFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.size = sizeof(u32),
				.src = &pink,
			});
		}

		if (!draws.empty()) {
			cmd->copyHostToBuffer({
				.src = (void*)&globData,
				.size = sizeof(decltype(globData)),
				.dst = globalDataBufffer,
			});

			if (draws.size() * sizeof(glm::mat4) > transformsBuffer->getSize()) {
				size_t newSize = std::pow(2, std::ceil(std::log(draws.size() * sizeof(glm::mat4))/std::log(2)));
				this->transformsBuffer = renderCTX.device->createBuffer({
					.size = newSize,
					.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
				});
			}

			auto mm = cmd->mapMemoryStaged(transformsBuffer, draws.size() * sizeof(glm::mat4), 0);
			for (int i = 0; i < draws.size(); i++) {
				((glm::mat4*)mm.hostPtr)[i] = draws[i].transform;
			}
			cmd->unmapMemoryStaged(mm);

			cmd->insertMemoryBarrier({
				.awaitedStages = VK_PIPELINE_STAGE_2_COPY_BIT_KHR,
				.waitingStages = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR,
			});
		}

		cmd->bindPipeline(pipeline);

        cmd->bindSet(0, globalSet);
		
		std::array framebuffer{
			daxa::gpu::RenderAttachmentInfo{
				.image = renderCTX.swapchainImage.getImageHandle(),
				.clearValue = { .color = VkClearColorValue{.float32 = { 0.02f, 0.02f, 0.02f, 1.f } } },
			},
			daxa::gpu::RenderAttachmentInfo{
				.image = renderCTX.normalsImage,
				.clearValue = { .color = VkClearColorValue{.float32 = { 0.0f,0.0f,0.0f,0.0f } } },
			}
		};
		daxa::gpu::RenderAttachmentInfo depthAttachment{
			.image = renderCTX.depthImage,
			.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.clearValue = { .depthStencil = VkClearDepthStencilValue{ .depth = 1.0f } },
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		};
		cmd->beginRendering(daxa::gpu::BeginRenderingInfo{
			.colorAttachments = framebuffer,
			.depthAttachment = &depthAttachment,
		});

        cmd->bindPipeline(pipeline);
		u32 index = 0;
        for (auto& draw : draws) {
            auto thisDrawSet = perDrawSetAlloc->getSet();
			if (draw.prim->albedoTexture) {
            	thisDrawSet->bindImage(0, draw.prim->albedoTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
			else {
				thisDrawSet->bindImage(0, dummyTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
            cmd->bindSet(1, thisDrawSet);
            cmd->bindIndexBuffer(draw.prim->indiexBuffer);
            cmd->bindVertexBuffer(0, draw.prim->vertexPositions);
            cmd->bindVertexBuffer(1, draw.prim->vertexUVs);
			cmd->bindVertexBuffer(2, draw.prim->vertexNormals);
			cmd->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, index);
            cmd->drawIndexed(draw.prim->indexCount, 1, 0, 0, 0);
			index += 1;
        }

		cmd->endRendering();

		cmd->insertImageBarrier({
			.image = renderCTX.normalsImage,
			.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.awaitedStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
		});
    }

private:
	GlobalData globData = {};
    daxa::gpu::PipelineHandle pipeline = {};
    daxa::gpu::BindingSetAllocatorHandle globalSetAlloc = {};
    daxa::gpu::BindingSetHandle globalSet = {};
    daxa::gpu::BufferHandle globalDataBufffer = {};
	daxa::gpu::BufferHandle transformsBuffer = {};
    daxa::gpu::BindingSetAllocatorHandle perDrawSetAlloc = {};
	daxa::gpu::ImageHandle dummyTexture = {};
};