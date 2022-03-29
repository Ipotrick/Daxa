#pragma once

#include "Daxa.hpp"

#include "RenderContext.hpp"
#include "../Components.hpp"

#include "OrthLightSource.hpp"

struct GlobalData {
	glm::mat4 vp;
	glm::mat4 view;
	u32 generalSampler;
};

class MeshRenderer {
public:

	struct GPUPrimitiveInfo {
		glm::mat4 transform;
		glm::mat4 inverseTransposeTransform;
		u32 albedoMapId;
		u32 normalMapId;
		u32 vertexPositionsId;
		u32 vertexUVsId;
		u32 vertexNormalsId;
		u32 _pad[3];
	};

	struct DrawLight{
		glm::vec3 position;
		f32 strength;
		glm::vec4 color;
	};

    void init(RenderContext& renderCTX) {
		daxa::GraphicsPipelineBuilder prePassPipelineBuilder;
		prePassPipelineBuilder
			.addShaderStage({ .pathToSource = "./DaxaMeshview/renderer/prePass.vert", .stage = VK_SHADER_STAGE_VERTEX_BIT, })
			.addShaderStage({ .pathToSource = "./DaxaMeshview/renderer/prePass.frag", .stage = VK_SHADER_STAGE_FRAGMENT_BIT })
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

		this->prePassPipeline = renderCTX.pipelineCompiler->createGraphicsPipeline(prePassPipelineBuilder).value();

        this->globalDataBufffer = renderCTX.device->createBuffer({
            .size = sizeof(GlobalData),
            .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            .memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
            .memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				.debugName = "mesh render globals buffer",
        });

		auto vertexShaderOpaqueCI = daxa::ShaderModuleCreateInfo{
			.pathToSource = "./DaxaMeshview/renderer/opaque.vert",
			.stage = VK_SHADER_STAGE_VERTEX_BIT
		};
		auto fragmentShaderOpaqueCI = daxa::ShaderModuleCreateInfo{
			.pathToSource = "./DaxaMeshview/renderer/opaque.frag",
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT
		};

		daxa::GraphicsPipelineBuilder opaquePipelineBuilder = {};
		opaquePipelineBuilder.addShaderStage({ .pathToSource = "./DaxaMeshview/renderer/opaque.vert", .stage = VK_SHADER_STAGE_VERTEX_BIT });
		opaquePipelineBuilder.addShaderStage({ .pathToSource = "./DaxaMeshview/renderer/opaque.frag", .stage = VK_SHADER_STAGE_FRAGMENT_BIT });
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
			.addColorAttachment(renderCTX.hdrImage->getVkFormat())
			.addColorAttachment(renderCTX.normalsImage->getVkFormat())
			.setRasterization({
				.cullMode = VK_CULL_MODE_BACK_BIT,
			});

		this->opaquePassPipeline = renderCTX.pipelineCompiler->createGraphicsPipeline(opaquePipelineBuilder).value();

		this->globalSetAlloc = renderCTX.device->createBindingSetAllocator({
			.setLayout = opaquePassPipeline->getSetLayout(0),
			.debugName = "mesh renderer global set allocator",
		});

		this->perDrawOpaquePassSetAlloc = renderCTX.device->createBindingSetAllocator({ .setLayout = this->opaquePassPipeline->getSetLayout(1), .setPerPool = 32'000 });

		initOpaque2(renderCTX);
		initTonemapPass(renderCTX);

		orthLightPass.init(renderCTX);

		skyBox = renderCTX.device->createImageView({
			.image = renderCTX.device->createImage({
				.imageType = VK_IMAGE_TYPE_2D,
				.format = VK_FORMAT_R16G16B16A16_SFLOAT,
				.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
				.extent = { 1, 1, 1 },
				.mipLevels = 1,
				.arrayLayers = 6,
				.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
				.debugName = "skybox",
			}),
			.viewType = VK_IMAGE_VIEW_TYPE_CUBE,
			.format = VK_FORMAT_R16G16B16A16_SFLOAT,
			.debugName = "skybox",
		});

		generalSampler = renderCTX.defaultSampler;
    }

	void initTonemapPass(RenderContext& renderCTX) {
		auto toneMapBuilder = daxa::GraphicsPipelineBuilder();
		toneMapBuilder.addShaderStage({.debugName = "tonemap", .pathToSource = "tonemap.frag", .stage = VK_SHADER_STAGE_FRAGMENT_BIT});
		toneMapBuilder.addShaderStage({.debugName = "tonemap", .pathToSource = "tonemap.vert", .stage = VK_SHADER_STAGE_VERTEX_BIT});
		toneMapBuilder.addColorAttachment(renderCTX.swapchainImage.getImageViewHandle()->getVkFormat());
		toneMapBuilder.overwriteSet(0, daxa::BIND_ALL_SET_DESCRIPTION);
		toneMapBuilder.setDebugName("tonemapPipeline");
		tonemapPipeline = renderCTX.pipelineCompiler->createGraphicsPipeline(toneMapBuilder).value();
	}

	void tonemapPass(RenderContext& renderCTX, daxa::CommandListHandle& cmdList) {
		if (renderCTX.pipelineCompiler->checkIfSourcesChanged(tonemapPipeline)) {
			auto result = renderCTX.pipelineCompiler->recreatePipeline(tonemapPipeline);
			std::cout << result << std::endl;
			if (result) {
				tonemapPipeline = result.value();
			}
		}
		cmdList->queueImageBarrier({
			.image = renderCTX.hdrImage,
			.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		});

		std::array colorAttachments {
			daxa::RenderAttachmentInfo{
				.image = renderCTX.swapchainImage.getImageViewHandle(),
			},
		};
		cmdList->beginRendering({
			.colorAttachments = colorAttachments,
		});
		cmdList->bindPipeline(tonemapPipeline);
		cmdList->bindAll();
		struct Push {
			u32 src;
			u32 width;
			u32 height;
		} p {
			renderCTX.hdrImage->getDescriptorIndex(),
			renderCTX.swapchainImage.getImageViewHandle()->getImageHandle()->getVkExtent3D().width,
			renderCTX.swapchainImage.getImageViewHandle()->getImageHandle()->getVkExtent3D().height,
		};
		cmdList->pushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, p);
		cmdList->draw(3, 1, 0, 0);
		cmdList->unbindPipeline();
		cmdList->endRendering();
		
		cmdList->queueImageBarrier({
			.image = renderCTX.hdrImage,
			.layoutBefore = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		});
	}

	void initOpaque2(RenderContext& renderCTX) {
		auto vertexShaderOpaqueCI = daxa::ShaderModuleCreateInfo{
			.pathToSource = "./DaxaMeshview/renderer/opaque2.vert",
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.debugName = "opaque2 vertex",
		};
		auto fragmentShaderOpaqueCI = daxa::ShaderModuleCreateInfo{
			.pathToSource = "./DaxaMeshview/renderer/opaque2.frag",
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.debugName = "opaque2 fragment",
		};

		daxa::GraphicsPipelineBuilder opaque2PipelineBuilder = {};
		opaque2PipelineBuilder
			.addShaderStage(vertexShaderOpaqueCI)
			.addShaderStage(fragmentShaderOpaqueCI)
			.overwriteSet(0, daxa::BIND_ALL_SET_DESCRIPTION)
			.setDebugName("mesh render opaque2 pass pipeline")
			.configurateDepthTest({
				.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT,
				.enableDepthTest = true, 
				.enableDepthWrite = true, 
				//.depthTestCompareOp = VK_COMPARE_OP_EQUAL
			})
			.addColorAttachment(renderCTX.hdrImage->getVkFormat())
			.addColorAttachment(renderCTX.normalsImage->getVkFormat())
			.setRasterization({
				.cullMode = VK_CULL_MODE_BACK_BIT,
		});

		opaquePass2Pipeline = renderCTX.pipelineCompiler->createGraphicsPipeline(opaque2PipelineBuilder).value();
	}

	void setCamera(daxa::CommandListHandle& cmd, glm::mat4 const& vp, glm::mat4 const& view) {
		globData.vp = vp;
		globData.view = view;
	}

	void uploadBuffers(RenderContext& renderCTX, daxa::CommandListHandle& cmd, std::vector<DrawPrimCmd>& draws, std::vector<DrawLight>& lights) {
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

			cmd->queueImageBarrier({
				.image = dummyTexture,
				.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
				.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			});
			cmd->singleCopyHostToImage({
				.src = (u8*)&pink,
				.dst = dummyTexture->getImageHandle(),
				.dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.region = {},
			});
			cmd->queueImageBarrier({
				.image = dummyTexture,
				.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
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

			cmd->queueImageBarrier({
				.image = dummyNormalsTexture,
				.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
				.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			});
			cmd->singleCopyHostToImage({
				.src = (u8*)&up,
				.dst = dummyNormalsTexture->getImageHandle(),
				.dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.region = {},
			});
			cmd->queueImageBarrier({
				.image = dummyNormalsTexture,
				.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			});
		}

		globData.generalSampler = generalSampler->getDescriptorIndex();
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
				auto& prim = mm.hostPtr[i];
				prim.transform = draws[i].transform;
				prim.inverseTransposeTransform = glm::inverse(glm::transpose(draws[i].transform));
				if (draws[i].prim->albedoMap.valid()) {
					prim.albedoMapId = draws[i].prim->albedoMap->getDescriptorIndex();
				} else {
					prim.albedoMapId = dummyTexture->getDescriptorIndex();
				}
				if (draws[i].prim->normalMap.valid()) {
					prim.normalMapId = draws[i].prim->normalMap->getDescriptorIndex();
				} else {
					prim.normalMapId = dummyNormalsTexture->getDescriptorIndex();
				}
				prim.vertexPositionsId = draws[i].prim->vertexPositions->getDescriptorIndex();
				prim.vertexUVsId = draws[i].prim->vertexUVs->getDescriptorIndex();
				prim.vertexNormalsId = draws[i].prim->vertexNormals->getDescriptorIndex();
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

	void prePass(RenderContext& renderCTX, daxa::CommandListHandle& cmd, std::vector<DrawPrimCmd>& draws) {
		cmd->bindPipeline(prePassPipeline);
		cmd->bindSet(0, globalSet);
		
		daxa::RenderAttachmentInfo depthAttachment{
			.image = renderCTX.depthImage,
			.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {.depthStencil = VkClearDepthStencilValue{.depth = 1.0f } },
		};
		cmd->beginRendering(daxa::BeginRenderingInfo{
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

	void setLights(std::vector<DrawLight>& lights) {
		drawLights = lights;
	}

	void opaquePass2(RenderContext& renderCTX, daxa::CommandListHandle& cmd, std::vector<DrawPrimCmd>& draws) {
		cmd->bindPipeline(opaquePass2Pipeline);
		cmd->bindAll(0);

		std::array framebuffer{
			daxa::RenderAttachmentInfo{
				.image = renderCTX.hdrImage,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.clearValue = {.color = VkClearColorValue{.float32 = { 0.00f, 0.00f, 0.00f, 1.0f } } },
			},
			daxa::RenderAttachmentInfo{
				.image = renderCTX.normalsImage,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.clearValue = {.color = VkClearColorValue{.float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } },
			},
		};
		daxa::RenderAttachmentInfo depthAttachment{
			.image = renderCTX.depthImage,
			.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			//.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {.depthStencil = VkClearDepthStencilValue{.depth = 1.0f } },
		};
		cmd->beginRendering(daxa::BeginRenderingInfo{
			.colorAttachments = framebuffer,
			.depthAttachment = &depthAttachment,
			});

		primitivesDrawn = 0;
		for (u32 i = 0; i < draws.size(); i++) {
			auto& draw = draws[i];
			primitivesDrawn += draw.prim->indexCount / 3;
			struct {
				u32 globals;
				u32 primitives;
				u32 lights;
				u32 drawIndex;
			} push{
				globalDataBufffer->getDescriptorIndex(),
				primitiveInfoBuffer->getDescriptorIndex(),
				lightsBuffer->getDescriptorIndex(),
				i
			};
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

	void render(RenderContext& renderCTX, daxa::CommandListHandle& cmd, std::vector<DrawPrimCmd>& draws) {
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
		tonemapPass(renderCTX, cmd);
	}

private:
	size_t primitivesDrawn = 0;
	bool first = true;
	GlobalData globData = {};
	std::vector<DrawLight> drawLights;
    daxa::PipelineHandle prePassPipeline = {};
    daxa::BindingSetAllocatorHandle globalSetAlloc = {};
    daxa::BindingSetHandle globalSet = {};
    daxa::BufferHandle globalDataBufffer = {};
	daxa::BufferHandle primitiveInfoBuffer = {};
	daxa::ImageViewHandle dummyTexture = {};
	daxa::ImageViewHandle dummyNormalsTexture = {};
	daxa::ImageViewHandle skyBox = {};
	daxa::SamplerHandle generalSampler = {};

	daxa::PipelineHandle opaquePassPipeline = {};
	daxa::BufferHandle lightsBuffer = {};
    daxa::BindingSetAllocatorHandle perDrawOpaquePassSetAlloc = {};

	daxa::PipelineHandle opaquePass2Pipeline = {};

	daxa::PipelineHandle tonemapPipeline = {};
public:
	OrthLightSourcePass orthLightPass = {};
};