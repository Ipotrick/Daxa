#pragma once

#include "Daxa.hpp"

#include "RenderContext.hpp"
#include "../Components.hpp"

#include "OrthLightSource.hpp"

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
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
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
			.configurateDepthTest({
				.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
				.enableDepthTest = true, 
				.enableDepthWrite = true, 
			})
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT) // positions
			.endVertexInputAttributeBinding()
			.setRasterization({
				.cullMode = VK_CULL_MODE_BACK_BIT,
			});

		this->prePassPipeline = renderCTX.device->createGraphicsPipeline(prePassPipelineBuilder).value();

        this->globalDataBufffer = renderCTX.device->createBuffer({
            .size = sizeof(GlobalData),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
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
			.configurateDepthTest({
				.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
				.enableDepthTest = true, 
				.enableDepthWrite = true, 
				.depthTestCompareOp = VK_COMPARE_OP_EQUAL
			})
			// adding a vertex input attribute binding:
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			// all added vertex input attributes are added to the previously added vertex input attribute binding
			.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)			// positions
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			.addVertexInputAttribute(VK_FORMAT_R32G32_SFLOAT)				// uvs
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)
			.endVertexInputAttributeBinding()
			// location of attachments in a shader are implied by the order they are added in the prePassPipeline builder:
			.addColorAttachment(renderCTX.swapchain->getVkFormat())
			.addColorAttachment(renderCTX.normalsImage->getVkFormat())
			.setRasterization({
				.cullMode = VK_CULL_MODE_BACK_BIT,
			});

		this->opaquePassPipeline = renderCTX.device->createGraphicsPipeline(opaquePipelineBuilder).value();

		this->globalSetAlloc = renderCTX.device->createBindingSetAllocator({
			.setLayout = opaquePassPipeline->getSetLayout(0),
			.debugName = "mesh renderer global set allocator",
		});

		this->perDrawOpaquePassSetAlloc = renderCTX.device->createBindingSetAllocator({ .setLayout = this->opaquePassPipeline->getSetLayout(1), .setPerPool = 32'000 });

		this->opaqueFragHotloader = daxa::GraphicsPipelineHotLoader(
			renderCTX.device,
			opaquePipelineBuilder,
			std::array{ vertexShaderOpaqueCI, fragmentShaderOpaqueCI }
		);

		initOpaque2(renderCTX);

		orthLightPass.init(renderCTX);
    }

	struct DrawLight{
		glm::vec3 position;
		f32 strength;
		glm::vec4 color;
	};

	void initOpaque2(RenderContext& renderCTX) {
		auto vertexShaderOpaqueCI = daxa::gpu::ShaderModuleCreateInfo{
			.pathToSource = "./DaxaMeshview/renderer/opaque2.vert",
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.debugName = "opaque2 vertex",
		};
		auto fragmentShaderOpaqueCI = daxa::gpu::ShaderModuleCreateInfo{
			.pathToSource = "./DaxaMeshview/renderer/opaque2.frag",
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.debugName = "opaque2 fragment",
		};

		daxa::gpu::GraphicsPipelineBuilder opaque2PipelineBuilder = {};
		opaque2PipelineBuilder
			.addShaderStage(renderCTX.device->createShaderModule(vertexShaderOpaqueCI).value())
			.addShaderStage(renderCTX.device->createShaderModule(fragmentShaderOpaqueCI).value())
			.overwriteSet(0, daxa::gpu::BIND_ALL_SET_DESCRIPTION)
			.setDebugName("mesh render opaque2 pass pipeline")
			.configurateDepthTest({
				.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
				.enableDepthTest = true, 
				.enableDepthWrite = true, 
				//.depthTestCompareOp = VK_COMPARE_OP_EQUAL
			})
			.addColorAttachment(renderCTX.swapchain->getVkFormat())
			.addColorAttachment(renderCTX.normalsImage->getVkFormat())
			.setRasterization({
				.cullMode = VK_CULL_MODE_BACK_BIT,
		});

		opaquePass2Pipeline = renderCTX.device->createGraphicsPipeline(opaque2PipelineBuilder).value();

		opaqueFragHotloader2 = daxa::GraphicsPipelineHotLoader{
			renderCTX.device, opaque2PipelineBuilder, std::array{ vertexShaderOpaqueCI, fragmentShaderOpaqueCI }
		};
	}

	void setCamera(daxa::gpu::CommandListHandle& cmd, glm::mat4 const& vp, glm::mat4 const& view) {
		globData.vp = vp;
		globData.view = view;
	}

	void uploadBuffers(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawPrimCmd>& draws, std::vector<DrawLight>& lights) {
		if (!dummyTexture) {
			dummyTexture = renderCTX.device->createImageView({
				.image = renderCTX.device->createImage({
					.format = VK_FORMAT_R8G8B8A8_SRGB,
					.extent = { 1,1,1 },
					.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
				}),
				.format = VK_FORMAT_R8G8B8A8_SRGB,
				.defaultSampler = renderCTX.device->createSampler({}),
				.debugName = "dummyTexture",
			});

			u32 pink = 0xFFFF00FF;

			cmd->copyHostToImageSynced({
				.src = &pink,
				.dst = dummyTexture,
				.size = sizeof(u32),
				.dstFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			});
		}

		if (!dummyNormalsTexture) {
			dummyNormalsTexture = renderCTX.device->createImageView({
				.image = renderCTX.device->createImage({
					.format = VK_FORMAT_R8G8B8A8_UNORM,
					.extent = { 1,1,1 },
					.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
				}),
				.format = VK_FORMAT_R8G8B8A8_UNORM,
				.defaultSampler = renderCTX.device->createSampler({}),
				.debugName = "dummyNormalsTexture",
			});
			
			// 0xFF (alpha = 1.0f) FF (blue/z = 1.0f) 7F (green/y = 0.5f/0.0f) 7F (red/x = 0.5f/0.0f)  
			u32 up = 0xFFFF7F7F;

			cmd->copyHostToImageSynced({
				.src = &up,
				.dst = dummyNormalsTexture,
				.size = sizeof(u32),
				.dstFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			});
		}

		cmd->copyHostToBuffer({
			.src = (void*)&globData,
			.dst = globalDataBufffer,
			.size = sizeof(decltype(globData)),
		});



		if (!primitiveInfoBuffer || draws.size() * sizeof(GPUPrimitiveInfo) > primitiveInfoBuffer->getSize()) {
			size_t newSize = (draws.size() + 64) * sizeof(GPUPrimitiveInfo);
			this->primitiveInfoBuffer = renderCTX.device->createBuffer({
				.size = newSize,
				.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
				.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				.debugName = "primitiveInfoBuffer",
			});
		}
		if (!draws.empty()) {
			auto mm = cmd->mapMemoryStagedBuffer<GPUPrimitiveInfo>(primitiveInfoBuffer, draws.size() * sizeof(GPUPrimitiveInfo), 0);
			for (int i = 0; i < draws.size(); i++) {
				mm.hostPtr[i].transform 				= draws[i].transform;
				mm.hostPtr[i].inverseTransposeTransform = glm::inverse(glm::transpose(draws[i].transform));
			}
		}
		
		if (!lightsBuffer || (lights.size() * sizeof(DrawLight) + sizeof(glm::vec4)) > lightsBuffer->getSize()) {
			size_t newSize = (lights.size() + 64) * sizeof(DrawLight) + sizeof(glm::vec4);
			this->lightsBuffer = renderCTX.device->createBuffer({
				.size = newSize,
				.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
				.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				.debugName = "lightsBuffer",
			});
		}
		{
			auto mm = cmd->mapMemoryStagedBuffer(lightsBuffer, lights.size() * sizeof(DrawLight) + sizeof(glm::vec4), 0);
			*((u32*)mm.hostPtr) = (u32)lights.size();
			mm.hostPtr += sizeof(glm::vec4);
			for (int i = 0; i < lights.size(); i++) {
				((DrawLight*)mm.hostPtr)[i] = lights[i];
			}
		}
	}

	void prePass(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawPrimCmd>& draws) {

		cmd->bindPipeline(prePassPipeline);
		cmd->bindSet(0, globalSet);
		
		daxa::gpu::RenderAttachmentInfo depthAttachment{
			.image = renderCTX.depthImage,
			.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {.depthStencil = VkClearDepthStencilValue{.depth = 1.0f } },
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
		cmd->unbindPipeline();

		cmd->insertMemoryBarrier({
			.srcStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR,
			.srcAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT_KHR,
			.dstStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR,
			.dstAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT_KHR,
		});
	}

	void opaquePass(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawPrimCmd>& draws) {
		if (auto newOpaque = opaqueFragHotloader.getNewIfChanged(); newOpaque.has_value()) {
			this->opaquePassPipeline = newOpaque.value();
            printf("hot laoded\n");
		}

		cmd->bindPipeline(opaquePassPipeline);
		cmd->bindSet(0, globalSet);

		std::array framebuffer{
			daxa::gpu::RenderAttachmentInfo{
				.image = renderCTX.swapchainImage.getImageViewHandle(),
				.clearValue = { .color = VkClearColorValue{.float32 = { 0.01f, 0.01f, 0.01f, 1.0f } } },
			},
			daxa::gpu::RenderAttachmentInfo{
				.image = renderCTX.normalsImage,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.clearValue = { .color = VkClearColorValue{.float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } },
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

	void opaquePass2(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawPrimCmd>& draws) {
		if (auto newOpaque = opaqueFragHotloader2.getNewIfChanged(); newOpaque.has_value()) {
			this->opaquePass2Pipeline = newOpaque.value();
			printf("hot loaded\n");
		}
		cmd->bindPipeline(opaquePass2Pipeline);
		cmd->bindAll(0);

		std::array framebuffer{
			daxa::gpu::RenderAttachmentInfo{
				.image = renderCTX.swapchainImage.getImageViewHandle(),
				.clearValue = {.color = VkClearColorValue{.float32 = { 0.01f, 0.01f, 0.01f, 1.0f } } },
			},
			daxa::gpu::RenderAttachmentInfo{
				.image = renderCTX.normalsImage,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.clearValue = {.color = VkClearColorValue{.float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } },
			},
		};
		daxa::gpu::RenderAttachmentInfo depthAttachment{
			.image = renderCTX.depthImage,
			.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			//.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {.depthStencil = VkClearDepthStencilValue{.depth = 1.0f } },
		};
		cmd->beginRendering(daxa::gpu::BeginRenderingInfo{
			.colorAttachments = framebuffer,
			.depthAttachment = &depthAttachment,
			});

		primitivesDrawn = 0;
		for (u32 i = 0; i < draws.size(); i++) {
			auto& draw = draws[i];
			primitivesDrawn += draw.prim->indexCount / 3;
			struct {
				u32 albedoMap;
				u32 normalMap;
				u32 globals;
				u32 primitives;
				u32 lights;
				u32 vertexBufId;
				u32 vertexUVBufId;
				u32 vertexNormalBufId;
				u32 drawIndex;
			} push{
				dummyTexture->getDescriptorIndex(),
				dummyNormalsTexture->getDescriptorIndex(),
				globalDataBufffer->getStorageBufferDescriptorIndex().value(),
				primitiveInfoBuffer->getStorageBufferDescriptorIndex().value(),
				lightsBuffer->getStorageBufferDescriptorIndex().value(),
				draw.prim->vertexPositions->getStorageBufferDescriptorIndex().value(),
				draw.prim->vertexUVs->getStorageBufferDescriptorIndex().value(),
				draw.prim->vertexNormals->getStorageBufferDescriptorIndex().value(),
				i
			};
			if (draw.prim->albedoTexture.valid()) {
				push.albedoMap = draw.prim->albedoTexture->getDescriptorIndex();
			}
			if (draw.prim->normalTexture.valid()) {
				push.normalMap = draw.prim->normalTexture->getDescriptorIndex();
			}
			if (
				push.albedoMap == 0 ||
				push.normalMap == 0 ||
				push.globals == 0 ||
				push.primitives == 0 ||
				push.lights == 0 ||
				push.vertexBufId == 0 ||
				push.vertexUVBufId == 0 ||
				push.vertexNormalBufId == 0
			) {
				printf("dumdum\n");
			}
			cmd->pushConstant(VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, push);
			cmd->bindIndexBuffer(draw.prim->indiexBuffer);
			cmd->drawIndexed(draw.prim->indexCount, 1, 0, 0, 0);
		}
		//printf("%i triangles drawn\n", primitivesDrawn);

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

	void render(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawPrimCmd>& draws) {
		uploadBuffers(renderCTX, cmd, draws, drawLights);
		cmd->insertMemoryBarrier({
			.srcStages = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR | VK_PIPELINE_STAGE_2_COPY_BIT_KHR,
			.srcAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
			.dstStages = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
			.dstAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR,
			});
		//orthLightPass.render(
		//	renderCTX,
		//	cmd,
		//	draws,
		//	primitiveInfoBuffer,
		//	glm::vec3(0,-1,0),
		//	1000.0f,
		//	glm::vec4(1,1,1,1)
		//);

		this->globalSet = globalSetAlloc->getSet();
		globalSet->bindBuffer(0, this->globalDataBufffer);
		globalSet->bindBuffer(1, this->primitiveInfoBuffer);
		globalSet->bindBuffer(2, this->lightsBuffer);
		cmd->insertImageBarrier({
			.barrier = {
				.dstStages = VK_PIPELINE_STAGE_2_CLEAR_BIT_KHR | VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
				.dstAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR | VK_ACCESS_2_MEMORY_READ_BIT_KHR,
			},
			.image = renderCTX.normalsImage,
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		});
		//prePass(renderCTX, cmd, draws);
		opaquePass2(renderCTX, cmd, draws);
	}

private:
	size_t primitivesDrawn = 0;
	bool first = true;
	daxa::GraphicsPipelineHotLoader opaqueFragHotloader = {};
	GlobalData globData = {};
	std::vector<DrawLight> drawLights;
    daxa::gpu::PipelineHandle prePassPipeline = {};
    daxa::gpu::BindingSetAllocatorHandle globalSetAlloc = {};
    daxa::gpu::BindingSetHandle globalSet = {};
    daxa::gpu::BufferHandle globalDataBufffer = {};
	daxa::gpu::BufferHandle primitiveInfoBuffer = {};
	daxa::gpu::ImageViewHandle dummyTexture = {};
	daxa::gpu::ImageViewHandle dummyNormalsTexture = {};

	daxa::gpu::PipelineHandle opaquePassPipeline = {};
	daxa::gpu::BufferHandle lightsBuffer = {};
    daxa::gpu::BindingSetAllocatorHandle perDrawOpaquePassSetAlloc = {};

	daxa::gpu::PipelineHandle opaquePass2Pipeline = {};
	daxa::GraphicsPipelineHotLoader opaqueFragHotloader2 = {};
public:
	OrthLightSourcePass orthLightPass = {};
};