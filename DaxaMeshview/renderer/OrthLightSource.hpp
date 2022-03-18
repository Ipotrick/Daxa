#pragma once

#include "../Components.hpp"
#include "RenderContext.hpp"

class OrthLightSourcePass {
public:
	struct GPUInfo {
		glm::mat4 vp;
	};

	void recreateShadowMap(RenderContext& renderCTX, u32 width, u32 height) {
		using namespace daxa::gpu;

		shadowMap = renderCTX.device->createImageView({
			.image = renderCTX.device->createImage({
				.format = VK_FORMAT_D32_SFLOAT,
				.extent = { width, height, 1 },
				.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			}),
			.format = VK_FORMAT_D32_SFLOAT,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			},
			.defaultSampler = renderCTX.device->createSampler({}),
			.debugName = "orth light shadow map"
		});
	}

	void init(RenderContext& renderCTX) {
		using namespace daxa::gpu;
		recreateShadowMap(renderCTX, 4096, 4096);
		ShaderModuleCreateInfo vsci {
			.pathToSource = "./DaxaMeshview/renderer/depthPass.vert",
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
		};

		ShaderModuleCreateInfo fsci{
			.pathToSource = "./DaxaMeshview/renderer/depthPass.frag",
			.shaderLang = daxa::gpu::ShaderLang::GLSL,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		};

		auto pipeBuilder = GraphicsPipelineBuilder{};
		pipeBuilder
			.addShaderStage(renderCTX.device->createShaderModule(vsci).value())
			.addShaderStage(renderCTX.device->createShaderModule(fsci).value())
			.overwriteSet(1, BIND_ALL_SET_DESCRIPTION)
			;
		pipeline = renderCTX.device->createGraphicsPipeline(pipeBuilder).value();

		persistentSetAlloc = renderCTX.device->createBindingSetAllocator({ .setLayout = pipeline->getSetLayout(0), .setPerPool = 3 });

		infoBuffer = renderCTX.device->createBuffer({
			.size = sizeof(GPUInfo),
			.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			.debugName = "orth shadow pass info buffer",
		});
	}

	void render(
		RenderContext& renderCTX, 
		daxa::gpu::CommandListHandle& cmd, 
		std::vector<DrawPrimCmd>& draws, 
		daxa::gpu::BufferHandle& primitiveInfosBuffer,
		glm::vec3 const& direction, 
		f32 distance, 
		glm::vec4 color
	) {
		using namespace daxa::gpu;

		glm::mat4 vp{ 1.0f };
		cmd->copyHostToBuffer({
			.src = &vp,
			.dst = infoBuffer,
			.size = sizeof(GPUInfo),
		});
		auto memBarrs = std::array{
			MemoryBarrier {
				.srcStages = VK_PIPELINE_STAGE_2_TRANSFER_BIT_KHR,
				.srcAccess = VK_ACCESS_2_TRANSFER_WRITE_BIT_KHR,
				.dstStages = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR,
				.dstAccess = VK_ACCESS_2_SHADER_STORAGE_READ_BIT_KHR,
			},
		};
		auto imgBarrs = std::array{
			ImageBarrier{
				.barrier = {
					.dstStages = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT_KHR,
					.dstAccess = VK_ACCESS_2_SHADER_READ_BIT_KHR,
				},
				.image = shadowMap,
				.layoutAfter = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			}
		};
		cmd->insertBarriers( memBarrs, imgBarrs);

		RenderAttachmentInfo depthAttach{
			.image = shadowMap,
			.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
			.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
			.clearValue = VkClearValue{.depthStencil = {.depth = 1.0f}},
		};
		cmd->beginRendering({ .depthAttachment = &depthAttach, });
		cmd->bindPipeline(pipeline);
		auto persistentSet = persistentSetAlloc->getSet("orth light persistent set");
		persistentSet->bindBuffer(0, infoBuffer);
		persistentSet->bindBuffer(1, primitiveInfosBuffer);
		cmd->bindSet(0, persistentSet);
		cmd->bindAll(1);

		for (u32 i = 0; i < draws.size(); i++) {
			auto& draw = draws[i];

			struct {
				u32 vertexBufferId;
				u32 drawId;
			} push {
				draw.prim->vertexPositions->getStorageBufferDescriptorIndex().value(),
				i
			};
			cmd->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, push);
			cmd->bindIndexBuffer(draw.prim->indiexBuffer);
			cmd->drawIndexed(draw.prim->indexCount, 1, 0, 0, 0);
		}

		cmd->unbindPipeline();
		cmd->endRendering();

		cmd->insertImageBarrier({
			.barrier = {
				.srcStages = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT_KHR,
				.srcAccess = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT_KHR,
				.dstStages = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT_KHR,
				.dstAccess = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT_KHR,
			},
			.image = shadowMap,
			.layoutBefore = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
			.layoutAfter = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		});
	}
	daxa::gpu::ImageViewHandle shadowMap = {};
private:
	daxa::gpu::PipelineHandle pipeline = {};
	daxa::gpu::BindingSetAllocatorHandle persistentSetAlloc = {};
	daxa::gpu::BufferHandle infoBuffer = {};
};