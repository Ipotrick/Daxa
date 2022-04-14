#pragma once

#include "Daxa.hpp"

#include "RenderContext.hpp"
#include "../Components.hpp"

#include "OrthLightSource.hpp"
#include "fft.hpp"

#include "tinyexr.h"

struct GlobalData {
	glm::mat4 perspective;
	glm::mat4 view;
	glm::mat4 vp;
	glm::mat4 iView;
	glm::vec4 cameraPosition;
	float verticalFOV;
	u32 renderTargetWidth;
	u32 renderTargetHeight;
	u32 generalSampler;
	u32 skyboxId;
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

		initOpaque2(renderCTX);
		initTonemapPass(renderCTX);

		orthLightPass.init(renderCTX);

		generalSampler = renderCTX.device->createSampler({
			.debugName = "general sampler",
		});

		initSkyBox(renderCTX);
		auto cmd = renderCTX.queue->getCommandList({});
		fft.init(renderCTX, cmd, renderCTX.hdrImage->getImageHandle()->getVkExtent3D().width, renderCTX.hdrImage->getImageHandle()->getVkExtent3D().height);
		cmd.finalize();
		renderCTX.queue->submit({.commandLists={cmd}});
	}

	void initSkyBox(RenderContext& renderCTX) {	
		u32 hardcodedDim = 2048;

		// size_t faceMemorySize = hardcodedDim * hardcodedDim * 4 * 4;

		u32 mips = static_cast<u32>(std::log2(hardcodedDim));

		skybox = renderCTX.device->createImageView({
			.image = renderCTX.device->createImage({
				.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
				.imageType = VK_IMAGE_TYPE_2D,
				.format = VK_FORMAT_R32G32B32A32_SFLOAT,
				.extent = { hardcodedDim, hardcodedDim, 1 },
				.mipLevels = mips,
				.arrayLayers = 6,
				.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
				.debugName = "skybox",
			}),
			.viewType = VK_IMAGE_VIEW_TYPE_CUBE,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.debugName = "skybox",
		});

		auto cmd = renderCTX.queue->getCommandList({"skybox loading cmdlist"});

		cmd.queueImageBarrier({
			.barrier = {
				.srcStages = 0,
				.srcAccess = 0,
				.dstStages = daxa::STAGE_TRANSFER,
				.dstAccess = daxa::ACCESS_MEMORY_WRITE,
			},
			.image = skybox,
			.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
			.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		});

		std::array<char const*, 6> hardcodedSkyBoxFileNames = {
			"C:/Users/Patrick/Desktop/skyboxRight.exr",
			"C:/Users/Patrick/Desktop/skyboxLeft.exr",
			"C:/Users/Patrick/Desktop/skyboxTop.exr",
			"C:/Users/Patrick/Desktop/skyboxBottom.exr",
			"C:/Users/Patrick/Desktop/skyboxFront.exr",
			"C:/Users/Patrick/Desktop/skyboxBack.exr",
		};

		for (u32 i = 0; i < 6; i++) { 
			i32 width = -1;
			i32 height = -1;
			// const char* layer_name = "diffuse";
			// auto deleter = [](float* image){ free(image); };

			float* data = nullptr;
			char const* err = nullptr;
			/* i32 ret = */ LoadEXR(&data, &width, &height, hardcodedSkyBoxFileNames[i], &err);
			if (err != nullptr) {
				std::cout << err << std::endl;
				FreeEXRErrorMessage(err);
			}
			if (data) {
				cmd.singleCopyHostToImage({
					.src = reinterpret_cast<u8*>(data),
					.dst = skybox->getImageHandle(),
					.dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					.region = {
						.srcOffset = 0,
						.subRessource = { .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = i, .layerCount = 1 },
						.imageOffset = { 0, 0, 0 },
						.imageExtent = { hardcodedDim, hardcodedDim, 1 },
					}
				});
				delete data;
			}else {
				printf("could not load skybox image: %s\n", hardcodedSkyBoxFileNames[i]);
			}
		}
		daxa::generateMipLevels(cmd, skybox, VkImageSubresourceLayers{ .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 6 }, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		cmd.finalize();
		renderCTX.queue->submit({.commandLists = {cmd}});

		auto skyboxPipelineBuilder = daxa::GraphicsPipelineBuilder();
		skyboxPipelineBuilder.addShaderStage({.pathToSource = "skybox.frag", .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .debugName = "skybox"});
		skyboxPipelineBuilder.addShaderStage({.pathToSource = "skybox.vert", .stage = VK_SHADER_STAGE_VERTEX_BIT, .debugName = "skybox"});
		skyboxPipelineBuilder.addColorAttachment(renderCTX.hdrImage->getVkFormat());
		skyboxPipelineBuilder.overwriteSet(0, daxa::BIND_ALL_SET_DESCRIPTION);
		skyboxPipelineBuilder.setDebugName("skybox");
		skyboxPipeline = renderCTX.pipelineCompiler->createGraphicsPipeline(skyboxPipelineBuilder).value();
	}

	void skyboxPass(RenderContext& renderCTX, daxa::CommandListHandle& cmdList) {
		if (renderCTX.pipelineCompiler->checkIfSourcesChanged(skyboxPipeline)) {
			auto result = renderCTX.pipelineCompiler->recreatePipeline(skyboxPipeline);
			std::cout << result << std::endl;
			if (result) {
				skyboxPipeline = result.value();
			}
		}
		std::array<daxa::RenderAttachmentInfo, 1> colorAttachments{
			daxa::RenderAttachmentInfo{
				.image = renderCTX.hdrImage,
				.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.clearValue = {.color = VkClearColorValue{.float32 = { 0.00f, 0.00f, 0.00f, 1.0f } } },
			},
		};
		cmdList.beginRendering({
			.colorAttachments = colorAttachments,
		});
		cmdList.bindPipeline(skyboxPipeline);
		cmdList.bindAll();
		cmdList.pushConstant(VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, u32(globalDataBufffer.getDescriptorIndex()));
		cmdList.draw(3*2*6, 1, 0, 0);
		cmdList.unbindPipeline();
		cmdList.endRendering();

		cmdList.queueMemoryBarrier({
			.srcStages = daxa::STAGE_COLOR_ATTACHMENT_OUTPUT,
			.srcAccess = daxa::ACCESS_MEMORY_WRITE,
			.dstStages = daxa::STAGE_COLOR_ATTACHMENT_OUTPUT,
			.dstAccess = daxa::ACCESS_MEMORY_WRITE,
		});
	}

	void initTonemapPass(RenderContext& renderCTX) {
		auto toneMapBuilder = daxa::GraphicsPipelineBuilder();
		toneMapBuilder.addShaderStage({.pathToSource = "tonemap.frag", .stage = VK_SHADER_STAGE_FRAGMENT_BIT, .debugName = "tonemap"});
		toneMapBuilder.addShaderStage({.pathToSource = "tonemap.vert", .stage = VK_SHADER_STAGE_VERTEX_BIT, .debugName = "tonemap"});
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
		cmdList.queueImageBarrier({
			.image = renderCTX.hdrImage,
			.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		});

		std::array colorAttachments {
			daxa::RenderAttachmentInfo{
				.image = renderCTX.swapchainImage.getImageViewHandle(),
			},
		};
		cmdList.beginRendering({
			.colorAttachments = colorAttachments,
		});
		cmdList.bindPipeline(tonemapPipeline);
		cmdList.bindAll();
		struct Push {
			u32 src;
			u32 width;
			u32 height;
		} p {
			renderCTX.hdrImage->getDescriptorIndex(),
			renderCTX.swapchainImage.getImageViewHandle()->getImageHandle()->getVkExtent3D().width,
			renderCTX.swapchainImage.getImageViewHandle()->getImageHandle()->getVkExtent3D().height,
		};
		cmdList.pushConstant(VK_SHADER_STAGE_FRAGMENT_BIT, p);
		cmdList.draw(3, 1, 0, 0);
		cmdList.unbindPipeline();
		cmdList.endRendering();
		
		cmdList.queueImageBarrier({
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

	void setCamera(daxa::CommandListHandle&, glm::mat4 const& perspective, glm::mat4 const& view, float verticalFOV, glm::vec4 cameraPosition) {
		globData.perspective = perspective;
		globData.view = view;
		globData.vp = perspective * view;
		globData.iView = glm::inverse(glm::transpose(view));
		globData.cameraPosition = cameraPosition;
		globData.verticalFOV = verticalFOV;
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

			cmd.queueImageBarrier({
				.image = dummyTexture,
				.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
				.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			});
			cmd.singleCopyHostToImage({
				.src = reinterpret_cast<u8*>(&pink),
				.dst = dummyTexture->getImageHandle(),
				.dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.region = {},
			});
			cmd.queueImageBarrier({
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

			cmd.queueImageBarrier({
				.image = dummyNormalsTexture,
				.layoutBefore = VK_IMAGE_LAYOUT_UNDEFINED,
				.layoutAfter = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			});
			cmd.singleCopyHostToImage({
				.src = reinterpret_cast<u8*>(&up),
				.dst = dummyNormalsTexture->getImageHandle(),
				.dstLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.region = {},
			});
			cmd.queueImageBarrier({
				.image = dummyNormalsTexture,
				.layoutBefore = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				.layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			});
		}

		globData.generalSampler = generalSampler->getDescriptorIndex();
		globData.skyboxId = skybox->getDescriptorIndex();
		globData.renderTargetWidth = renderCTX.hdrImage->getImageHandle()->getVkExtent3D().width;
		globData.renderTargetHeight = renderCTX.hdrImage->getImageHandle()->getVkExtent3D().height;
		cmd.singleCopyHostToBuffer({
			.src = reinterpret_cast<u8*>(&globData),
			.dst = globalDataBufffer,
			.region= {.size = sizeof(decltype(globData))}
		});

		if (!primitiveInfoBuffer || draws.size() * sizeof(GPUPrimitiveInfo) > primitiveInfoBuffer.getSize()) {
			size_t newSize = (draws.size() + 64) * sizeof(GPUPrimitiveInfo);
			this->primitiveInfoBuffer = renderCTX.device->createBuffer({
				.size = newSize,
				.debugName = "primitiveInfoBuffer",
			});
		}
		if (!draws.empty()) {
			auto mm = cmd.mapMemoryStagedBuffer(primitiveInfoBuffer, draws.size() * sizeof(GPUPrimitiveInfo), 0);
			for (size_t i = 0; i < draws.size(); i++) {
				auto& prim = (reinterpret_cast<GPUPrimitiveInfo*>(mm.hostPtr))[i];
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
				prim.vertexPositionsId = draws[i].prim->vertexPositions.getDescriptorIndex();
				prim.vertexUVsId = draws[i].prim->vertexUVs.getDescriptorIndex();
				prim.vertexNormalsId = draws[i].prim->vertexNormals.getDescriptorIndex();
			}
		}
		
		if (!lightsBuffer || (lights.size() * sizeof(DrawLight) + sizeof(glm::vec4)) > lightsBuffer.getSize()) {
			size_t newSize = (lights.size() + 64) * sizeof(DrawLight) + sizeof(glm::vec4);
			this->lightsBuffer = renderCTX.device->createBuffer({
				.size = newSize,
				.debugName = "lightsBuffer",
			});
		}
		{
			auto mm = cmd.mapMemoryStagedBuffer(lightsBuffer, lights.size() * sizeof(DrawLight) + sizeof(glm::vec4), 0);
			*(reinterpret_cast<u32*>(mm.hostPtr)) = static_cast<u32>(lights.size());
			mm.hostPtr += sizeof(glm::vec4);
			for (size_t i = 0; i < lights.size(); i++) {
				(reinterpret_cast<DrawLight*>(mm.hostPtr))[i] = lights[i];
			}
		}

		fft.uploadData(renderCTX, cmd);
	}

	void prePass(RenderContext& renderCTX, daxa::CommandListHandle& cmd, std::vector<DrawPrimCmd>& draws) {
		cmd.bindPipeline(prePassPipeline);
		cmd.bindSet(0, globalSet);
		
		daxa::RenderAttachmentInfo depthAttachment{
			.image = renderCTX.depthImage,
			.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = {.depthStencil = VkClearDepthStencilValue{.depth = 1.0f } },
		};
		cmd.beginRendering(daxa::BeginRenderingInfo{
			.colorAttachments = {},
			.depthAttachment = &depthAttachment,
		});

		for (u32 i = 0; i < draws.size(); i++) {
			auto& draw = draws[i];
			cmd.pushConstant(VK_SHADER_STAGE_VERTEX_BIT, i);
			cmd.bindIndexBuffer(draw.prim->indiexBuffer);
			cmd.bindVertexBuffer(0, draw.prim->vertexPositions);
			cmd.drawIndexed(draw.prim->indexCount, 1, 0, 0, 0);
		}

		cmd.endRendering();
		cmd.unbindPipeline();

		cmd.queueMemoryBarrier({
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
		if (renderCTX.pipelineCompiler->checkIfSourcesChanged(opaquePass2Pipeline)) {
			auto result = renderCTX.pipelineCompiler->recreatePipeline(opaquePass2Pipeline);
			std::cout << result << std::endl;
			if (result) {
				opaquePass2Pipeline = result.value();
			}
		}
		cmd.bindPipeline(opaquePass2Pipeline);
		cmd.bindAll(0);

		std::array framebuffer{
			daxa::RenderAttachmentInfo{
				.image = renderCTX.hdrImage,
				.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
				//.clearValue = {.color = VkClearColorValue{.float32 = { 0.00f, 0.00f, 0.00f, 1.0f } } },
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
		cmd.beginRendering(daxa::BeginRenderingInfo{
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
				globalDataBufffer.getDescriptorIndex(),
				primitiveInfoBuffer.getDescriptorIndex(),
				lightsBuffer.getDescriptorIndex(),
				i
			};
			cmd.pushConstant(VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT, push);
			cmd.bindIndexBuffer(draw.prim->indiexBuffer);
			cmd.drawIndexed(draw.prim->indexCount, 1, 0, 0, 0);
		}
		//printf("%i triangles drawn\n", primitivesDrawn);

		cmd.endRendering();
		cmd.unbindPipeline();

		cmd.queueImageBarrier({
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
		cmd.queueMemoryBarrier({
			.srcStages = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR | VK_PIPELINE_STAGE_2_COPY_BIT_KHR,
			.srcAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
			.dstStages = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
			.dstAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR,
		});
		cmd.insertQueuedBarriers();

		cmd.queueImageBarrier({
			.barrier = {
				.dstStages = VK_PIPELINE_STAGE_2_CLEAR_BIT_KHR | VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT,
				.dstAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR | VK_ACCESS_2_MEMORY_READ_BIT_KHR,
			},
			.image = renderCTX.normalsImage,
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		});
		cmd.insertQueuedBarriers();
		skyboxPass(renderCTX, cmd);
		opaquePass2(renderCTX, cmd, draws);
		fft.update(renderCTX, cmd);
		tonemapPass(renderCTX, cmd);
	}

	daxa::PipelineHandle skyboxPipeline = {};

	size_t primitivesDrawn = 0;
	GlobalData globData = {};
	std::vector<DrawLight> drawLights;
    daxa::PipelineHandle prePassPipeline = {};
    daxa::BindingSetHandle globalSet = {};
    daxa::BufferHandle globalDataBufffer = {};
	daxa::BufferHandle primitiveInfoBuffer = {};
	daxa::ImageViewHandle dummyTexture = {};
	daxa::ImageViewHandle dummyNormalsTexture = {};
	daxa::ImageViewHandle skybox = {};
	daxa::SamplerHandle generalSampler = {};
	daxa::BufferHandle lightsBuffer = {};
	daxa::PipelineHandle opaquePass2Pipeline = {};
	daxa::PipelineHandle tonemapPipeline = {};
	FFT fft = {};
public:
	OrthLightSourcePass orthLightPass = {};
};