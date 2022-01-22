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

	struct GPUPrimitiveInfo {
		glm::mat4 transform;
		glm::mat4 inverseTransposeTransform;
	};

    void init(RenderContext& renderCTX) {
		auto vertexShader = renderCTX.device->createShaderModule({
			.pathToSource = "./DaxaMeshview/renderer/prePass.vert",
			.stage = VK_SHADER_STAGE_VERTEX_BIT
		}).value();

		auto fragmenstShader = renderCTX.device->createShaderModule({
			.pathToSource = "./DaxaMeshview/renderer/prePass.frag",
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT
		}).value();

		daxa::gpu::GraphicsPipelineBuilder prePassPipelineBuilder;
		prePassPipelineBuilder
			.addShaderStage(vertexShader)
			.addShaderStage(fragmenstShader)
			.setDebugName("mesh render prePassPipeline")
			.configurateDepthTest({.enableDepthTest = true, .enableDepthWrite = true, .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT})
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT) // positions
			.setRasterization({
				.cullMode = VK_CULL_MODE_BACK_BIT,
			});

		this->prePassPipeline = renderCTX.device->createGraphicsPipeline(prePassPipelineBuilder).value();

		this->globalSetAlloc = renderCTX.device->createBindingSetAllocator({
			.setDescription = prePassPipeline->getSetDescription(0),
			.debugName = "mesh renderer global set allocator",
		});

        this->globalDataBufffer = renderCTX.device->createBuffer({
            .size = sizeof(GlobalData),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
            .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				.debugName = "mesh render globals buffer",
        });

		auto vertexShaderOpaqueCI = daxa::gpu::ShaderModuleCreateInfo{
			.pathToSource = "./DaxaMeshview/renderer/opaque.vert",
			.stage = VK_SHADER_STAGE_VERTEX_BIT
		};
		auto fragmentShaderOpaqueCI = daxa::gpu::ShaderModuleCreateInfo{
			.pathToSource = "./DaxaMeshview/renderer/opaque.frag",
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT
		};

		daxa::gpu::GraphicsPipelineBuilder opaquePipelineBuilder = {};
		opaquePipelineBuilder.addShaderStage(renderCTX.device->createShaderModule(vertexShaderOpaqueCI).value());
		opaquePipelineBuilder.addShaderStage(renderCTX.device->createShaderModule(fragmentShaderOpaqueCI).value());
			opaquePipelineBuilder.setDebugName("mesh render opaque pass pipeline")
			.configurateDepthTest({.enableDepthTest = true, .enableDepthWrite = true, .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT })
			// adding a vertex input attribute binding:
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			// all added vertex input attributes are added to the previously added vertex input attribute binding
			.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)			// positions
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			.addVertexInputAttribute(VK_FORMAT_R32G32_SFLOAT)				// uvs
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)
			// location of attachments in a shader are implied by the order they are added in the prePassPipeline builder:
			.addColorAttachment(renderCTX.swapchain->getVkFormat())
			.addColorAttachment(renderCTX.normalsImage->getVkViewFormat())
			.setRasterization({
				.cullMode = VK_CULL_MODE_BACK_BIT,
			});

		this->opaquePassPipeline = renderCTX.device->createGraphicsPipeline(opaquePipelineBuilder).value();

		this->perDrawOpaquePassSetAlloc = renderCTX.device->createBindingSetAllocator({ .setDescription = this->opaquePassPipeline->getSetDescription(1) });

		this->opaqueFragHotloader = daxa::GraphicsPipelineHotLoader(
			renderCTX.device,
			opaquePipelineBuilder,
			std::array{ vertexShaderOpaqueCI, fragmentShaderOpaqueCI }
		);
    }

    struct DrawMesh {
        glm::mat4 transform = {};
		Primitive* prim = {};
    };

	struct DrawLight{
		glm::vec3 position;
		f32 strength;
		glm::vec4 color;
	};

	void setCamera(daxa::gpu::CommandListHandle& cmd, glm::mat4 const& vp, glm::mat4 const& view) {
		globData.vp = vp;
		globData.view = view;
	}

	void uploadBuffers(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawMesh>& draws, std::vector<DrawLight>& lights) {
		if (!dummyTexture) {
			dummyTexture = renderCTX.device->createImage2d({
				.width = 1,
				.height = 1,
				.format = VK_FORMAT_R8G8B8A8_SRGB,
				.imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
				.sampler = renderCTX.device->createSampler({}),
				.debugName = "dummyTexture",
			});

			u32 pink = 0xFFFF00FF;

			cmd->copyHostToImageSynced({
				.dst = dummyTexture,
				.dstFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.size = sizeof(u32),
				.src = &pink,
			});
		}

		if (!dummyNormalsTexture) {
			dummyNormalsTexture = renderCTX.device->createImage2d({
				.width = 1,
				.height = 1,
				.imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.sampler = renderCTX.device->createSampler({}),
				.debugName = "dummyNormalsTexture",
			});
			
			// 0xFF (alpha = 1.0f) FF (blue/z = 1.0f) 7F (green/y = 0.5f/0.0f) 7F (red/x = 0.5f/0.0f)  
			u32 up = 0xFFFF7F7F;

			cmd->copyHostToImageSynced({
				.dst = dummyNormalsTexture,
				.dstFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				.size = sizeof(u32),
				.src = &up,
			});
		}

		cmd->copyHostToBuffer({
			.src = (void*)&globData,
			.size = sizeof(decltype(globData)),
			.dst = globalDataBufffer,
		});

		if (!primitiveInfoBuffer || draws.size() * sizeof(GPUPrimitiveInfo) > primitiveInfoBuffer->getSize()) {
			size_t newSize = (draws.size() + 64) * sizeof(GPUPrimitiveInfo);
			this->primitiveInfoBuffer = renderCTX.device->createBuffer({
				.size = newSize,
				.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
				.debugName = "mesh render transform buffer",
			});
		}
		if (!draws.empty()) {
			auto mm = cmd->mapMemoryStaged<GPUPrimitiveInfo>(primitiveInfoBuffer, draws.size() * sizeof(GPUPrimitiveInfo), 0);
			for (int i = 0; i < draws.size(); i++) {
				mm.hostPtr[i].transform 				= draws[i].transform;
				mm.hostPtr[i].inverseTransposeTransform = glm::inverse(glm::transpose(draws[i].transform));
			}
		}
		
		if (!lightsBuffer || (lights.size() * sizeof(DrawLight) + sizeof(glm::vec4)) > lightsBuffer->getSize()) {
			size_t newSize = (lights.size() + 64) * sizeof(DrawLight) + sizeof(glm::vec4);
			this->lightsBuffer = renderCTX.device->createBuffer({
				.size = newSize,
				.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
				.debugName = "mesh render lights buffer",
			});
		}
		{
			auto mm = cmd->mapMemoryStaged(lightsBuffer, lights.size() * sizeof(DrawLight) + sizeof(glm::vec4), 0);
			*((u32*)mm.hostPtr) = (u32)lights.size();
			mm.hostPtr += sizeof(glm::vec4);
			for (int i = 0; i < lights.size(); i++) {
				((DrawLight*)mm.hostPtr)[i] = lights[i];
			}
		}

		cmd->insertMemoryBarrier({
			.srcStages = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR | VK_PIPELINE_STAGE_2_COPY_BIT_KHR,
			.srcAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
			.dstStages = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
			.dstAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR,
		});

        this->globalSet = globalSetAlloc->getSet();
        globalSet->bindBuffer(0, this->globalDataBufffer);
		globalSet->bindBuffer(1, this->primitiveInfoBuffer);
		globalSet->bindBuffer(2, this->lightsBuffer);
	}

	void prePass(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawMesh>& draws) {
		cmd->insertImageBarrier({
			.image = renderCTX.normalsImage,
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		});

		cmd->bindPipeline(prePassPipeline);
		cmd->bindSet(0, globalSet);
		
		daxa::gpu::RenderAttachmentInfo depthAttachment{
			.image = renderCTX.depthImage,
			.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.clearValue = { .depthStencil = VkClearDepthStencilValue{ .depth = 1.0f } },
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		};
		cmd->beginRendering(daxa::gpu::BeginRenderingInfo{
			.colorAttachments = {},
			.depthAttachment = &depthAttachment,
		});

		for (u32 i = 0; i < draws.size(); i++) {
			auto& draw = draws[i];
			cmd->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, i);
			cmd->bindIndexBuffer(draw.prim->indiexBuffer);
			cmd->bindVertexBuffer(0, draw.prim->vertexPositions);
			cmd->drawIndexed(draw.prim->indexCount, 1, 0, 0, 0);
		}

		cmd->endRendering();

		cmd->insertMemoryBarrier({
			.srcStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR,
			.srcAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT_KHR,
			.dstStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR,
			.dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT_KHR,
		});

		cmd->unbindPipeline();
	}

	void opaquePass(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawMesh>& draws) {
		if (auto newOpaque = opaqueFragHotloader.getNewIfChanged(); newOpaque.has_value()) {
			this->opaquePassPipeline = newOpaque.value();
            printf("hot laoded\n");
		}

		cmd->bindPipeline(opaquePassPipeline);
		//cmd->bindSet(0, globalSet);

		std::array framebuffer{
			daxa::gpu::RenderAttachmentInfo{
				.image = renderCTX.swapchainImage.getImageHandle(),
				.clearValue = { .color = VkClearColorValue{.float32 = { 0.01f, 0.01f, 0.01f, 1.0f } } },
			},
			daxa::gpu::RenderAttachmentInfo{
				.image = renderCTX.normalsImage,
				.clearValue = { .color = VkClearColorValue{.float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } },
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			},
		};
		daxa::gpu::RenderAttachmentInfo depthAttachment{
			.image = renderCTX.depthImage,
			.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		};
		cmd->beginRendering(daxa::gpu::BeginRenderingInfo{
			.colorAttachments = framebuffer,
			.depthAttachment = &depthAttachment,
		});

		for (u32 i = 0; i < draws.size(); i++) {
			auto& draw = draws[i];
			auto set = perDrawOpaquePassSetAlloc->getSet();
			if (draw.prim->albedoTexture) {
				set->bindImage(0, draw.prim->albedoTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			} else {
				set->bindImage(0, dummyTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
			if (draw.prim->normalTexture) {
				set->bindImage(1, draw.prim->normalTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			} else {
				set->bindImage(1, dummyNormalsTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
			cmd->bindSet(1, set);
			cmd->bindIndexBuffer(draw.prim->indiexBuffer);
			cmd->bindVertexBuffer(0, draw.prim->vertexPositions);
			cmd->bindVertexBuffer(1, draw.prim->vertexUVs);
			cmd->bindVertexBuffer(2, draw.prim->vertexNormals);
			cmd->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, i);
			cmd->drawIndexed(draw.prim->indexCount, 1, 0, 0, 0);
		}

		cmd->endRendering();
		cmd->unbindPipeline();

		cmd->insertImageBarrier({
			.barrier = {
				.srcStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR,
				.srcAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT_KHR,
				.dstStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
				.dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT_KHR | VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR | VK_ACCESS_2_SHADER_STORAGE_READ_BIT_KHR,
			},
			.image = renderCTX.normalsImage,
			.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		});
	}

	void setLights(std::vector<DrawLight>& lights) {
		drawLights = lights;
	}

	void render(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawMesh>& draws) {
		uploadBuffers(renderCTX, cmd, draws, drawLights);
		prePass(renderCTX, cmd, draws);
		opaquePass(renderCTX, cmd, draws);
	}

private:
	daxa::GraphicsPipelineHotLoader opaqueFragHotloader;
	GlobalData globData = {};
	std::vector<DrawLight> drawLights;
    daxa::gpu::PipelineHandle prePassPipeline = {};
    daxa::gpu::BindingSetAllocatorHandle globalSetAlloc = {};
    daxa::gpu::BindingSetHandle globalSet = {};
    daxa::gpu::BufferHandle globalDataBufffer = {};
	daxa::gpu::BufferHandle primitiveInfoBuffer = {};
	daxa::gpu::ImageHandle dummyTexture = {};
	daxa::gpu::ImageHandle dummyNormalsTexture = {};

	daxa::gpu::PipelineHandle opaquePassPipeline = {};
	daxa::gpu::BufferHandle lightsBuffer = {};
    daxa::gpu::BindingSetAllocatorHandle perDrawOpaquePassSetAlloc = {};
};