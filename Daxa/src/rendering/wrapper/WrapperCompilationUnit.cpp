#include <fstream>
#include <ios>
#include <iostream>
#include <vector>

#include "CommandBuffer.hpp"

namespace daxa {
    namespace vk {
        CommandBuffer::CommandBuffer(VkCommandPool pool, VkDevice device) :
            pool{ pool }, device{ device }
        {
            VkCommandBufferAllocateInfo cmdAllocInfo = {};
            cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmdAllocInfo.pNext = nullptr;
            cmdAllocInfo.commandPool = pool;                        // commands will be made from our _commandPool
            cmdAllocInfo.commandBufferCount = 1;                    // we will allocate 1 command buffer
            cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;   // command level is Primary

            vkAllocateCommandBuffers(device, &cmdAllocInfo, &buffer);
        }

        CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept
        {
            this->device = other.device;
            this->pool = other.pool;
            this->buffer = other.buffer;
            other.pool = VK_NULL_HANDLE;
            other.device = VK_NULL_HANDLE;
            other.buffer = VK_NULL_HANDLE;
        }

        CommandBuffer::operator const VkCommandBuffer&()
        {
            assert(buffer != VK_NULL_HANDLE);   // USE AFTER MOVE
            return buffer;
        }
        const VkCommandBuffer& CommandBuffer::get()
        {
            assert(buffer != VK_NULL_HANDLE);   // USE AFTER MOVE
            return buffer;
        }
    }
}


#include "CommandPool.hpp"

namespace daxa {
    namespace vk {
        CommandPool::CommandPool(
            u32 queueFamilyIndex,
            VkCommandPoolCreateFlagBits flags,
            VkDevice device
        ) :
            queueFamilyIndex{ queueFamilyIndex },
            device{ device }
        {
            VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
            cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            cmdPoolCreateInfo.pNext = nullptr;
            cmdPoolCreateInfo.flags = flags;   // we can reset individual command buffers from this pool
            cmdPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

            vkCreateCommandPool(daxa::vk::mainDevice, &cmdPoolCreateInfo, nullptr, &pool);
        }

        CommandPool::CommandPool(CommandPool&& other) noexcept
        {
            this->queueFamilyIndex = other.queueFamilyIndex;
            this->device = other.device;
            this->pool = other.pool;
            other.queueFamilyIndex = 0xFFFFFFFF;
            other.device = VK_NULL_HANDLE;
            other.pool = VK_NULL_HANDLE;
        }

        CommandPool::~CommandPool()
        {
            if (pool != VK_NULL_HANDLE)
                vkDestroyCommandPool(device, pool, nullptr);
        }

        CommandPool::operator const VkCommandPool&()
        {
            assert(pool != VK_NULL_HANDLE); // USE AFTER MOVE
            return pool;
        }
        const VkCommandPool& CommandPool::get()
        {
            assert(pool != VK_NULL_HANDLE); // USE AFTER MOVE
            return pool;
        }
    }
}

#include "Fence.hpp"

namespace daxa {
    namespace vk {
        Fence::Fence(VkDevice device) :
            device{ device }
        {
            VkFenceCreateInfo fenceCreateInfo = {};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.pNext = nullptr;

            //we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);
        }

        Fence::Fence(Fence&& other) noexcept
        {
            this->device = other.device;
            this->fence = other.fence;
            other.device = VK_NULL_HANDLE;
            other.fence = VK_NULL_HANDLE;
        }

        Fence::~Fence()
        {
            if (fence != VK_NULL_HANDLE)
                vkDestroyFence(device, fence, nullptr);
        }

        Fence::operator const VkFence& ()
        {
            assert(fence != VK_NULL_HANDLE);
            return fence;
        }

        const VkFence& Fence::get()
        {
            assert(fence != VK_NULL_HANDLE);
            return fence;
        }
    }
}


#include "Semaphore.hpp"

namespace daxa {
    namespace vk {
        Semaphore::Semaphore(VkDevice device) :
            device{ device }
        {
            //for the semaphores we don't need any flags
            VkSemaphoreCreateInfo semaphoreCreateInfo = {};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semaphoreCreateInfo.pNext = nullptr;
            semaphoreCreateInfo.flags = 0;

            vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &semaphore);
        }

        Semaphore::Semaphore(Semaphore&& other) noexcept
        {
            this->device = other.device;
            this->semaphore = other.semaphore;
            other.device = VK_NULL_HANDLE;
            other.semaphore = VK_NULL_HANDLE;
        }

        Semaphore::~Semaphore()
        {
            if (semaphore != VK_NULL_HANDLE)
                vkDestroySemaphore(device, semaphore, nullptr);
        }

        Semaphore::operator const VkSemaphore&()
        {
            assert(semaphore != VK_NULL_HANDLE);
            return semaphore;
        }

        const VkSemaphore& Semaphore::get()
        {
            assert(semaphore != VK_NULL_HANDLE);
            return semaphore;
        }
    }
}


#include "Shader.hpp"

namespace daxa {
    namespace vk {
        std::optional<VkShaderModule> loadShaderModule(std::string filePath, VkDevice device)
        {
            //open the file. With cursor at the end
            std::ifstream file{ filePath, std::ios::ate | std::ios::binary };

            if (!file.is_open()) {
                return {};
            }

            uz fileSize = (uz)file.tellg();

            //spirv expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file
            std::vector<u32> buffer(fileSize / sizeof(u32));

            //put file cursor at beggining
            file.seekg(0);

            //load the entire file into the buffer
            file.read((char*)buffer.data(), fileSize);

            //now that the file is loaded into the buffer, we can close it
            file.close();


            //create a new shader module, using the buffer we loaded
            VkShaderModuleCreateInfo createInfo = {};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.pNext = nullptr;

            //codeSize has to be in bytes, so multply the ints in the buffer by size of int to know the real size of the buffer
            createInfo.codeSize = buffer.size() * sizeof(u32);
            createInfo.pCode = buffer.data();

            //check that the creation goes well.
            VkShaderModule shaderModule;
            if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                return {};
            }
            return shaderModule;
        }

        VkPipelineShaderStageCreateInfo makeShaderStageCreateInfo(VkShaderStageFlagBits stage, VkShaderModule shaderModule)
        {
            VkPipelineShaderStageCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            info.pNext = nullptr;

            //shader stage
            info.stage = stage;
            //module containing the code for this shader stage
            info.module = shaderModule;
            //the entry point of the shader
            info.pName = "main";
            return info;
        }
    }
}


#include "Pipeline.hpp"

namespace daxa {
    namespace vk {

        VkPipelineVertexInputStateCreateInfo makeVertexInputStageCreateInfo()
        {
            VkPipelineVertexInputStateCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            info.pNext = nullptr;

            //no vertex bindings or attributes
            info.vertexBindingDescriptionCount = 0;
            info.vertexAttributeDescriptionCount = 0;
            return info;
        }

        VkPipelineInputAssemblyStateCreateInfo makeInputAssemblyStateCreateInfo(VkPrimitiveTopology topology)
        {
            VkPipelineInputAssemblyStateCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            info.pNext = nullptr;

            info.topology = topology;
            //we are not going to use primitive restart on the entire tutorial so leave it on false
            info.primitiveRestartEnable = VK_FALSE;
            return info;
        }

        VkPipelineRasterizationStateCreateInfo makeRasterisationStateCreateInfo(VkPolygonMode polygonMode)
        {
            VkPipelineRasterizationStateCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
            info.pNext = nullptr;

            info.depthClampEnable = VK_FALSE;
            //discards all primitives before the rasterization stage if enabled which we don't want
            info.rasterizerDiscardEnable = VK_FALSE;

            info.polygonMode = polygonMode;
            info.lineWidth = 1.0f;
            //no backface cull
            info.cullMode = VK_CULL_MODE_NONE;
            info.frontFace = VK_FRONT_FACE_CLOCKWISE;
            //no depth bias
            info.depthBiasEnable = VK_FALSE;
            info.depthBiasConstantFactor = 0.0f;
            info.depthBiasClamp = 0.0f;
            info.depthBiasSlopeFactor = 0.0f;

            return info;
        }

        VkPipelineMultisampleStateCreateInfo makeMultisampleStateCreateInfo()
        {
            VkPipelineMultisampleStateCreateInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
            info.pNext = nullptr;

            info.sampleShadingEnable = VK_FALSE;
            //multisampling defaulted to no multisampling (1 sample per pixel)
            info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
            info.minSampleShading = 1.0f;
            info.pSampleMask = nullptr;
            info.alphaToCoverageEnable = VK_FALSE;
            info.alphaToOneEnable = VK_FALSE;
            return info;
        }

        VkPipelineColorBlendAttachmentState makeColorBlendSAttachmentState()
        {
            VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_FALSE;
            return colorBlendAttachment;
        }

        VkPipelineLayoutCreateInfo makeLayoutCreateInfo()
        {
            VkPipelineLayoutCreateInfo info{};
            info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            info.pNext = nullptr;

            //empty defaults
            info.flags = 0;
            info.setLayoutCount = 0;
            info.pSetLayouts = nullptr;
            info.pushConstantRangeCount = 0;
            info.pPushConstantRanges = nullptr;
            return info;
        }

        VkPipeline PipelineBuilder::build(VkRenderPass pass, VkDevice device) {
            //make viewport state from our stored viewport and scissor.
            //at the moment we won't support multiple viewports or scissors
            VkPipelineViewportStateCreateInfo viewportState = {};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.pNext = nullptr;

            viewportState.viewportCount = 1;
            viewportState.pViewports = &_viewport;
            viewportState.scissorCount = 1;
            viewportState.pScissors = &_scissor;

            //setup dummy color blending. We aren't using transparent objects yet
            //the blending is just "no blend", but we do write to the color attachment
            VkPipelineColorBlendStateCreateInfo colorBlending = {};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.pNext = nullptr;

            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = VK_LOGIC_OP_COPY;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &_colorBlendAttachment;





            //we now use all of the info structs we have been writing into into this one to create the pipeline
            VkGraphicsPipelineCreateInfo pipelineInfo = {};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.pNext = nullptr;

            pipelineInfo.stageCount = _shaderStages.size();
            pipelineInfo.pStages = _shaderStages.data();
            pipelineInfo.pVertexInputState = &_vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &_inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &_rasterizer;
            pipelineInfo.pMultisampleState = &_multisampling;
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.layout = _pipelineLayout;
            pipelineInfo.renderPass = pass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            //it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
            VkPipeline newPipeline;
            if (vkCreateGraphicsPipelines(
                device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
                std::cout << "failed to create pipline\n";
                return VK_NULL_HANDLE; // failed to create graphics pipeline
            }
            else 	{
                return newPipeline;
            }
        }
    }
}
