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
		auto vertexShader = renderCTX.device->createShaderModule({
			.pathToSource = "./DaxaMeshview/renderer/g_pass.vert",
			.stage = VK_SHADER_STAGE_VERTEX_BIT
		});
		if (vertexShader.isErr()) {
			std::cout << "could not load vertex shader due to: " << vertexShader.message() << std::endl;
		}

		auto fragmenstShader = renderCTX.device->createShaderModule({
			.pathToSource = "./DaxaMeshview/renderer/g_pass.frag",
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT
		});
		if (vertexShader.isErr()) {
			std::cout << "could not load fragment shader due to: " << vertexShader.message() << std::endl;
		}

		daxa::gpu::GraphicsPipelineBuilder prePassPipelineBuilder;
		prePassPipelineBuilder
			.addShaderStage(vertexShader.value())
			.addShaderStage(fragmenstShader.value())
			.setDebugName("mesh render prePassPipeline")
			.configurateDepthTest({.enableDepthTest = true, .enableDepthWrite = true, .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT})
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

		this->prePassPipeline = renderCTX.device->createGraphicsPipeline(prePassPipelineBuilder);

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

		auto vertexOpaqueShader = renderCTX.device->createShaderModule({
			.pathToSource = "./DaxaMeshview/renderer/opaque.vert",
			.stage = VK_SHADER_STAGE_VERTEX_BIT
		}).value();

		auto fragmenstOpaqueShader = renderCTX.device->createShaderModule({
			.pathToSource = "./DaxaMeshview/renderer/opaque.frag",
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT
		}).value();

		daxa::gpu::GraphicsPipelineBuilder opaquePipelineBuilder;
		opaquePipelineBuilder.addShaderStage(vertexOpaqueShader);
		opaquePipelineBuilder.addShaderStage(fragmenstOpaqueShader);
			opaquePipelineBuilder.setDebugName("mesh render opaque pass pipeline")
			.configurateDepthTest({.enableDepthTest = true, .enableDepthWrite = true, .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT})
			// adding a vertex input attribute binding:
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			// all added vertex input attributes are added to the previously added vertex input attribute binding
			.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)			// positions
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			.addVertexInputAttribute(VK_FORMAT_R32G32_SFLOAT)				// uvs
			// location of attachments in a shader are implied by the order they are added in the prePassPipeline builder:
			.addColorAttachment(renderCTX.swapchain->getVkFormat())
			.setRasterization({
				.cullMode = VK_CULL_MODE_BACK_BIT,
			});

		this->opaquePassPipeline = renderCTX.device->createGraphicsPipeline(opaquePipelineBuilder);

		this->perDrawOpaquePassSetAlloc = renderCTX.device->createBindingSetAllocator({ .setDescription = this->opaquePassPipeline->getSetDescription(1) });
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

		cmd->copyHostToBuffer({
			.src = (void*)&globData,
			.size = sizeof(decltype(globData)),
			.dst = globalDataBufffer,
		});

		if (!transformsBuffer || draws.size() * sizeof(glm::mat4) > transformsBuffer->getSize()) {
			size_t newSize = (draws.size() + 64) * sizeof(glm::mat4);
			this->transformsBuffer = renderCTX.device->createBuffer({
				.size = newSize,
				.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
				.debugName = "mesh render transform buffer",
			});
		}
		if (!draws.empty()) {
			auto mm = cmd->mapMemoryStaged<glm::mat4>(transformsBuffer, draws.size() * sizeof(glm::mat4), 0);
			for (int i = 0; i < draws.size(); i++) {
				mm.hostPtr[i] = draws[i].transform;
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
			auto mm = cmd->mapMemoryStaged(lightsBuffer, lights.size() * sizeof(glm::mat4) + sizeof(glm::vec4), 0);
			*((u32*)mm.hostPtr) = (u32)lights.size();
			mm.hostPtr += sizeof(glm::vec4);
			for (int i = 0; i < lights.size(); i++) {
				((glm::mat4*)mm.hostPtr)[i] = draws[i].transform;
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
		globalSet->bindBuffer(1, this->transformsBuffer);
		globalSet->bindBuffer(2, this->lightsBuffer);
	}

	void prePass(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawMesh>& draws) {
		cmd->insertImageBarrier({
			.image = renderCTX.normalsImage,
			.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		});

		cmd->bindPipeline(prePassPipeline);
		cmd->bindSet(0, globalSet);
		
		std::array framebuffer{
			daxa::gpu::RenderAttachmentInfo{
				.image = renderCTX.normalsImage,
				.clearValue = { .color = VkClearColorValue{.float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } },
			}
		};
		daxa::gpu::RenderAttachmentInfo depthAttachment{
			.image = renderCTX.depthImage,
			.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.clearValue = { .depthStencil = VkClearDepthStencilValue{ .depth = 1.0f } },
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		};
		cmd->beginRendering(daxa::gpu::BeginRenderingInfo{
			.colorAttachments = framebuffer,
			.depthAttachment = &depthAttachment,
		});

		for (u32 i = 0; i < draws.size(); i++) {
			auto& draw = draws[i];
			cmd->bindIndexBuffer(draw.prim->indiexBuffer);
			cmd->bindVertexBuffer(0, draw.prim->vertexPositions);
			cmd->bindVertexBuffer(1, draw.prim->vertexUVs);
			cmd->bindVertexBuffer(2, draw.prim->vertexNormals);
			cmd->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, i);
			cmd->drawIndexed(draw.prim->indexCount, 1, 0, 0, 0);
		}

		cmd->endRendering();

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

	void opaquePass(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawMesh>& draws) {

		cmd->bindPipeline(opaquePassPipeline);
		//cmd->bindSet(0, globalSet);

		std::array framebuffer{
			daxa::gpu::RenderAttachmentInfo{
				.image = renderCTX.swapchainImage.getImageHandle(),
				.clearValue = { .color = VkClearColorValue{.float32 = { 0.01f, 0.01f, 0.01f, 1.0f } } },
			}
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
			cmd->bindSet(1, set);
			cmd->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, i);
			cmd->bindIndexBuffer(draw.prim->indiexBuffer);
			cmd->bindVertexBuffer(0, draw.prim->vertexPositions);
			cmd->bindVertexBuffer(1, draw.prim->vertexUVs);
			cmd->bindVertexBuffer(2, draw.prim->vertexNormals);
			cmd->drawIndexed(draw.prim->indexCount, 1, 0, 0, 0);
		}

		cmd->endRendering();
	}

	void render(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawMesh>& draws) {
		std::vector<DrawLight> drawLights;
		uploadBuffers(renderCTX, cmd, draws, drawLights);
		prePass(renderCTX, cmd, draws);
		opaquePass(renderCTX, cmd, draws);
	}

    //void render(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawMesh>& draws) {
	//	cmd->insertImageBarrier({
	//		.image = renderCTX.normalsImage,
	//		.layoutAfter = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	//	});
//
	//	if (!dummyTexture) {
	//		dummyTexture = renderCTX.device->createImage2d({
	//			.width = 1,
	//			.height = 1,
	//			.imageUsage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
	//			.sampler = renderCTX.device->createSampler({}),
	//		});
//
	//		u32 pink = 0xFFFF00FF;
//
	//		cmd->copyHostToImageSynced({
	//			.dst = dummyTexture,
	//			.dstFinalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	//			.size = sizeof(u32),
	//			.src = &pink,
	//		});
	//	}
//
	//	if (!draws.empty()) {
	//		cmd->copyHostToBuffer({
	//			.src = (void*)&globData,
	//			.size = sizeof(decltype(globData)),
	//			.dst = globalDataBufffer,
	//		});
//
	//		if (draws.size() * sizeof(glm::mat4) > transformsBuffer->getSize()) {
	//			size_t newSize = std::pow(2, std::ceil(std::log(draws.size() * sizeof(glm::mat4))/std::log(2)));
	//			this->transformsBuffer = renderCTX.device->createBuffer({
	//				.size = newSize,
	//				.memoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
	//				.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	//				.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY,
	//			});
	//		}
	//		{
	//			auto mm = cmd->mapMemoryStaged(transformsBuffer, draws.size() * sizeof(glm::mat4), 0);
	//			for (int i = 0; i < draws.size(); i++) {
	//				((glm::mat4*)mm.hostPtr)[i] = draws[i].transform;
	//			}
	//		}
//
	//		cmd->insertMemoryBarrier({
	//			.srcStages = VK_PIPELINE_STAGE_2_COPY_BIT_KHR,
	//			.srcAccess = VK_ACCESS_2_MEMORY_WRITE_BIT_KHR,
	//			.dstStages = VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT_KHR,
	//			.dstAccess = VK_ACCESS_2_MEMORY_READ_BIT_KHR,
	//		});
	//	}
//
	//	cmd->bindPipeline(prePassPipeline);
//
    //    cmd->bindSet(0, globalSet);
	//	
	//	std::array framebuffer{
	//		daxa::gpu::RenderAttachmentInfo{
	//			.image = renderCTX.swapchainImage.getImageHandle(),
	//			.clearValue = { .color = VkClearColorValue{.float32 = { 0.02f, 0.02f, 0.02f, 1.f } } },
	//		},
	//		daxa::gpu::RenderAttachmentInfo{
	//			.image = renderCTX.normalsImage,
	//			.clearValue = { .color = VkClearColorValue{.float32 = { 0.0f, 0.0f, 0.0f, 0.0f } } },
	//		}
	//	};
	//	daxa::gpu::RenderAttachmentInfo depthAttachment{
	//		.image = renderCTX.depthImage,
	//		.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
	//		.clearValue = { .depthStencil = VkClearDepthStencilValue{ .depth = 1.0f } },
	//		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
	//		.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
	//	};
	//	cmd->beginRendering(daxa::gpu::BeginRenderingInfo{
	//		.colorAttachments = framebuffer,
	//		.depthAttachment = &depthAttachment,
	//	});
//
    //    cmd->bindPipeline(prePassPipeline);
	//	u32 index = 0;
    //    for (auto& draw : draws) {
    //        auto thisDrawSet = perDrawPrePassSetAlloc->getSet();
	//		if (draw.prim->albedoTexture) {
    //        	thisDrawSet->bindImage(0, draw.prim->albedoTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	//		}
	//		else {
	//			thisDrawSet->bindImage(0, dummyTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	//		}
    //        cmd->bindSet(1, thisDrawSet);
    //        cmd->bindIndexBuffer(draw.prim->indiexBuffer);
    //        cmd->bindVertexBuffer(0, draw.prim->vertexPositions);
    //        cmd->bindVertexBuffer(1, draw.prim->vertexUVs);
	//		cmd->bindVertexBuffer(2, draw.prim->vertexNormals);
	//		cmd->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, index);
    //        cmd->drawIndexed(draw.prim->indexCount, 1, 0, 0, 0);
	//		index += 1;
    //    }
//
	//	cmd->endRendering();
//
	//	cmd->insertImageBarrier({
	//		.barrier = {
	//			.srcStages = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR,
	//			.srcAccess = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT_KHR,
	//			.dstStages = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT_KHR,
	//			.dstAccess = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR | VK_ACCESS_2_SHADER_STORAGE_READ_BIT_KHR,
	//		},
	//		.image = renderCTX.normalsImage,
	//		.layoutBefore = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	//		.layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	//	});
//
	//	std::array opaquePassFrameBuffer{
	//		daxa::gpu::RenderAttachmentInfo{
	//			.image = renderCTX.swapchainImage.getImageHandle(),
	//			.clearValue = { .color = VkClearColorValue{.float32 = { 0.02f, 0.02f, 0.02f, 1.f } } },
	//		}
	//	};
	//	cmd->beginRendering({
	//		.colorAttachments = opaquePassFrameBuffer,
	//		.depthAttachment = &depthAttachment,
	//	});
//
	//	cmd->bindPipeline(opaquePassPipeline);
	//	
    //    for (auto& draw : draws) {
    //        auto thisDrawSet = perDrawPrePassSetAlloc->getSet();
	//		if (draw.prim->albedoTexture) {
    //        	thisDrawSet->bindImage(0, draw.prim->albedoTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	//		}
	//		else {
	//			thisDrawSet->bindImage(0, dummyTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	//		}
    //        cmd->bindSet(1, thisDrawSet);
    //        cmd->bindIndexBuffer(draw.prim->indiexBuffer);
    //        cmd->bindVertexBuffer(0, draw.prim->vertexPositions);
    //        cmd->bindVertexBuffer(1, draw.prim->vertexUVs);
	//		cmd->bindVertexBuffer(2, draw.prim->vertexNormals);
	//		cmd->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, index);
    //        cmd->drawIndexed(draw.prim->indexCount, 1, 0, 0, 0);
	//		index += 1;
    //    }
//
	//	cmd->endRendering();
    //}

private:
	GlobalData globData = {};
    daxa::gpu::PipelineHandle prePassPipeline = {};
    daxa::gpu::BindingSetAllocatorHandle globalSetAlloc = {};
    daxa::gpu::BindingSetHandle globalSet = {};
    daxa::gpu::BufferHandle globalDataBufffer = {};
	daxa::gpu::BufferHandle transformsBuffer = {};
	daxa::gpu::ImageHandle dummyTexture = {};

	daxa::gpu::PipelineHandle opaquePassPipeline = {};
	daxa::gpu::BufferHandle lightsBuffer = {};
    daxa::gpu::BindingSetAllocatorHandle perDrawOpaquePassSetAlloc = {};
};