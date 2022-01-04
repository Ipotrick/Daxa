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
			
			void main()
			{
				vtf_uv = uv;
				gl_Position = globals.vp * vec4(position, 1.0f);
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

        this->globalSet = globalSetAlloc->getSet();
        globalSet->bindBuffer(0, this->globalDataBufffer);
    }

    struct DrawMesh {
        glm::mat4 transform = {};
        daxa::gpu::ImageHandle albedo = {};
        daxa::gpu::BufferHandle indices = {};
        u32 indexCount = {};
        daxa::gpu::BufferHandle vertices = {};
    };

    void render(RenderContext& renderCTX, daxa::gpu::CommandListHandle& cmd, std::vector<DrawMesh>& draws) {
        cmd->bindSet(0, globalSet);

        cmd->bindPipeline(pipeline);
        for (auto& draw : draws) {
            auto thisDrawSet = perDrawSetAlloc->getSet();
            thisDrawSet->bindImage(0, draw.albedo, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
            cmd->bindSet(1, thisDrawSet);
            cmd->bindIndexBuffer(draw.indices);
            cmd->bindVertexBuffer(0, draw.vertices);
            cmd->drawIndexed(draw.indexCount, 1, 0, 0, 0);
        }
    }

    void cleanup() {
        perDrawSetAlloc = {};
        globalDataBufffer = {};
        globalSet = {};
        globalSetAlloc = {};
        pipeline = {};
    }

private:
    glm::mat4 vp = {};
    daxa::gpu::PipelineHandle pipeline = {};
    daxa::gpu::BindingSetAllocatorHandle globalSetAlloc = {};
    daxa::gpu::BindingSetHandle globalSet = {};
    daxa::gpu::BufferHandle globalDataBufffer = {};
    daxa::gpu::BindingSetAllocatorHandle perDrawSetAlloc = {};
};