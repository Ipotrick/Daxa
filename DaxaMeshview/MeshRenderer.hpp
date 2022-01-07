#pragma once

#include "Daxa.hpp"

#include "RenderContext.hpp"

class MeshRenderer {
public:

    void init(RenderContext& renderCTX) {
		char const* vertexShaderGLSL = R"(
			#version 450
			#extension GL_KHR_vulkan_glsl : enable

			layout(location = 0) in vec3 position;
			layout(location = 1) in vec2 uv;

			layout(location = 10) out vec2 vtf_uv;

			layout(set = 0, binding = 0) uniform Globals {
				mat4 vp;
			} globals;

			layout(std140, set = 0, binding = 1) buffer ModelData {
				mat4 transforms[];
			} modelData;

			layout(push_constant) uniform PushConstants {
				uint modelIndex;
			} pushConstants;
			
			void main()
			{
				vtf_uv = uv;
				gl_Position = globals.vp * modelData.transforms[pushConstants.modelIndex] * vec4(position, 1.0f);
			}
		)";

		char const* fragmentShaderGLSL = R"(
			#version 450
			#extension GL_KHR_vulkan_glsl : enable

			layout(location = 10) in vec2 vtf_uv;

			layout (location = 0) out vec4 outFragColor;

			layout(set = 1, binding = 0) uniform sampler2D albedo;

			void main()
			{
				vec4 color = texture(albedo, vtf_uv);
				outFragColor = color;
			}
		)";

		daxa::gpu::ShaderModuleHandle vertexShader = renderCTX.device->tryCreateShderModuleFromGLSL(
			vertexShaderGLSL,
			VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT
		).value();

		daxa::gpu::ShaderModuleHandle fragmenstShader = renderCTX.device->tryCreateShderModuleFromGLSL(
			fragmentShaderGLSL,
			VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT
		).value();

		daxa::gpu::GraphicsPipelineBuilder pipelineBuilder;
		pipelineBuilder
			.addShaderStage(vertexShader)
			.addShaderStage(fragmenstShader)
			.configurateDepthTest({.enableDepthTest = true, .enableDepthWrite = true, .depthAttachmentFormat = VK_FORMAT_D32_SFLOAT})
			// adding a vertex input attribute binding:
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			// all added vertex input attributes are added to the previously added vertex input attribute binding
			.addVertexInputAttribute(VK_FORMAT_R32G32B32_SFLOAT)			// positions
			.beginVertexInputAttributeBinding(VK_VERTEX_INPUT_RATE_VERTEX)
			.addVertexInputAttribute(VK_FORMAT_R32G32_SFLOAT)				// uvs
			// location of attachments in a shader are implied by the order they are added in the pipeline builder:
			.addColorAttachment(renderCTX.swapchain->getVkFormat());

		this->pipeline = renderCTX.device->createGraphicsPipeline(pipelineBuilder);

		this->globalSetAlloc = renderCTX.device->createBindingSetAllocator(pipeline->getSetDescription(0));
		this->perDrawSetAlloc = renderCTX.device->createBindingSetAllocator(pipeline->getSetDescription(1));

        this->globalDataBufffer = renderCTX.device->createBuffer({
            .size = sizeof(glm::mat4),
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
        daxa::gpu::ImageHandle albedo = {};
        daxa::gpu::BufferHandle indices = {};
        u32 indexCount = {};
        daxa::gpu::BufferHandle positions = {};
        daxa::gpu::BufferHandle uvs = {};
    };

	void setCameraVP(daxa::gpu::CommandListHandle& cmd, glm::mat4 const& vp) {
		cmd->copyHostToBuffer({
			.src = (void*)&vp,
			.size = sizeof(decltype(vp)),
			.dst = globalDataBufffer,
		});
	}

    void render(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawMesh>& draws) {
		if (draws.empty()) return;
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

		cmd->bindPipeline(pipeline);

        cmd->bindSet(0, globalSet);
		
		std::array framebuffer{
			daxa::gpu::RenderAttachmentInfo{
				.image = renderCTX.swapchainImage.getImageHandle(),
				.clearValue = { .color = VkClearColorValue{.float32 = { 1.f, 1.f, 1.f, 1.f } } },
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
            thisDrawSet->bindImage(0, draw.albedo, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            cmd->bindSet(1, thisDrawSet);
            cmd->bindIndexBuffer(draw.indices);
            cmd->bindVertexBuffer(0, draw.positions);
            cmd->bindVertexBuffer(1, draw.uvs);
			cmd->pushConstant(VK_SHADER_STAGE_VERTEX_BIT, index);
            cmd->drawIndexed(draw.indexCount, 1, 0, 0, 0);
			index += 1;
        }

		cmd->endRendering();
    }

private:
    glm::mat4 vp = {};
    daxa::gpu::PipelineHandle pipeline = {};
    daxa::gpu::BindingSetAllocatorHandle globalSetAlloc = {};
    daxa::gpu::BindingSetHandle globalSet = {};
    daxa::gpu::BufferHandle globalDataBufffer = {};
	daxa::gpu::BufferHandle transformsBuffer = {};
    daxa::gpu::BindingSetAllocatorHandle perDrawSetAlloc = {};
};