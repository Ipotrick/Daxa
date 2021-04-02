#include <fstream>
#include <ios>
#include <iostream>
#include <vector>


#include "Initialization.hpp"

namespace daxa {
    namespace vkh {
        VkFence makeFence(VkDevice device)
        {
            VkFenceCreateInfo fenceCreateInfo = {};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.pNext = nullptr;

            //we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            VkFence fence;
            vkCreateFence(device, &fenceCreateInfo, nullptr, &fence);
            return fence;
        }

        VkSemaphore makeSemaphore(VkDevice device)
        {
            VkSemaphoreCreateInfo semaphoreCreateInfo = {};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            semaphoreCreateInfo.pNext = nullptr;
            semaphoreCreateInfo.flags = 0;

            VkSemaphore sem;
            vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &sem);
            return sem;
        }

        VkRenderPass makeRenderPass(VkRenderPassCreateInfo info, VkDevice device)
        {
            VkRenderPass renderpass;
            vkCreateRenderPass(device, &info, nullptr, &renderpass);
            return renderpass;
        }
    }
}


#include "CommandBuffer.hpp"

namespace daxa {
    namespace vkh {

        VkCommandBuffer makeCommandBuffer(VkCommandPool pool, VkDevice device)
        {
            VkCommandBufferAllocateInfo cmdAllocInfo = {};
            cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmdAllocInfo.pNext = nullptr;
            cmdAllocInfo.commandPool = pool;                        // commands will be made from our _commandPool
            cmdAllocInfo.commandBufferCount = 1;                    // we will allocate 1 command buffer
            cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;   // command level is Primary

            VkCommandBuffer buffer;
            vkAllocateCommandBuffers(device, &cmdAllocInfo, &buffer);
            return buffer;
        }

        VkCommandBufferBeginInfo makeCmdBeginInfo(VkCommandBufferUsageFlags usage)
        {
            VkCommandBufferBeginInfo cmdBeginInfo = {};
            cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmdBeginInfo.pNext = nullptr;
            cmdBeginInfo.pInheritanceInfo = nullptr;
            cmdBeginInfo.flags = usage;
            return cmdBeginInfo;
        }

        CommandBuffer::CommandBuffer(VkCommandPool pool, VkDevice device) :
            pool{ pool }, device{ device }
        {
            buffer = makeCommandBuffer(pool, device);
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

        CommandBuffer::~CommandBuffer()
        {
            if (buffer) {
                vkFreeCommandBuffers(device, pool, 1, &buffer);
            }
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
    namespace vkh {
        VkCommandPool makeCommandPool(
            u32 queueFamilyIndex,
            VkCommandPoolCreateFlagBits flags,
            VkDevice device)
        {
            VkCommandPoolCreateInfo cmdPoolCreateInfo = {};
            cmdPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            cmdPoolCreateInfo.pNext = nullptr;
            cmdPoolCreateInfo.flags = flags;   // we can reset individual command buffers from this pool
            cmdPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

            VkCommandPool pool;
            vkCreateCommandPool(daxa::vkh::mainDevice, &cmdPoolCreateInfo, nullptr, &pool);
            return pool;
        }

        CommandPool::CommandPool(
            u32 queueFamilyIndex,
            VkCommandPoolCreateFlagBits flags,
            VkDevice device
        ) :
            queueFamilyIndex{ queueFamilyIndex },
            device{ device },
            pool{ makeCommandPool(queueFamilyIndex, flags, device) },
            bufferPool{
                [=]() { return makeCommandBuffer(pool, device); },
                [=](VkCommandBuffer buffer) { /* gets freed anyway */ },
                [=](VkCommandBuffer buffer) { vkResetCommandBuffer(buffer, 0); }
            }
        {
        }

        CommandPool::CommandPool(CommandPool&& other) noexcept
        {
            this->queueFamilyIndex = other.queueFamilyIndex;
            this->device = other.device;
            this->pool = other.pool;
            this->bufferPool = std::move(other.bufferPool);
            other.queueFamilyIndex = 0xFFFFFFFF;
            other.device = VK_NULL_HANDLE;
            other.pool = VK_NULL_HANDLE;
        }

        CommandPool::~CommandPool()
        {
            if (pool)
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

        VkCommandBuffer CommandPool::getBuffer()
        {
            return bufferPool.get();
        }

        void CommandPool::flush()
        {
            bufferPool.flush();
        }
    }
}


#include "Shader.hpp"

namespace daxa {
    namespace vkh {

        ShaderModule::ShaderModule(std::string filePath, VkDevice device) :
            device { device }
        {
            auto opt = loadShaderModule(filePath, device);
            if (opt) {
                shader = opt.value();
            }
        }

        ShaderModule::ShaderModule(ShaderModule&& other) noexcept
        {
            this->destructionQ = other.destructionQ;
            this->device = other.device;
            this->shader = other.shader;
            other.destructionQ = nullptr;
            other.device = VK_NULL_HANDLE;
            other.shader = VK_NULL_HANDLE;
        }

        ShaderModule::~ShaderModule()
        {
            if (shader) {
                vkDestroyShaderModule(device, shader, nullptr);
            }
        }

        ShaderModule::operator const VkShaderModule& ()
        {
            assert(shader);
            return shader;
        }

        const VkShaderModule& ShaderModule::get() const
        {
            assert(shader);
            return shader;
        }

        ShaderModule::operator bool() const
        {
            return shader != VK_NULL_HANDLE;
        }

        bool ShaderModule::valid() const
        {
            return shader != VK_NULL_HANDLE;
        }

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
    namespace vkh {

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


        void BetterPipelineBuilder::setVertexInfo(const VertexDescription& vd)
        {
            VkPipelineVertexInputStateCreateInfo vertexInputStateCreate = {};
            vertexInputStateCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            vertexInputStateCreate.pVertexAttributeDescriptions = vd.attributes.data();
            vertexInputStateCreate.vertexAttributeDescriptionCount = vd.attributes.size();
            vertexInputStateCreate.pVertexBindingDescriptions = vd.bindings.data();
            vertexInputStateCreate.vertexBindingDescriptionCount = vd.bindings.size();
            vertexInputInfo = vertexInputStateCreate;
        }

        VkPipeline BetterPipelineBuilder::build(VkRenderPass pass, VkDevice device)
        {
            if (!viewport) {
                std::cout << "error: viewport was not specified in pipeline builder!\n";
                exit(-1);
            }
            if (!pipelineLayout) {
                std::cout << "error: pipelineLayout was not specified in pipeline builder!\n";
                exit(-1);
            }
            if (!vertexInputInfo) {
                std::cout << "error: vertexInputInfo was not specified in pipeline builder!\n";
                exit(-1);
            }
            if (!scissor) {
                VkRect2D s;
                s.offset = { i32(viewport.value().x), i32(viewport.value().y) };
                s.extent = { u32(viewport.value().width), u32(viewport.value().height) };
                this->scissor = s;
            }
            //make viewport state from our stored viewport and scissor.
            //at the moment we won't support multiple viewports or scissors
            VkPipelineViewportStateCreateInfo viewportState = {};
            viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
            viewportState.pNext = nullptr;

            viewportState.viewportCount = 1;
            viewportState.pViewports = &viewport.value();
            viewportState.scissorCount = 1;
            viewportState.pScissors = &scissor.value();

            if (!colorBlendAttachment) {
                colorBlendAttachment = vkh::makeColorBlendSAttachmentState();
            }

            //setup dummy color blending. We aren't using transparent objects yet
            //the blending is just "no blend", but we do write to the color attachment
            VkPipelineColorBlendStateCreateInfo colorBlending = {};
            colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
            colorBlending.pNext = nullptr;

            colorBlending.logicOpEnable = VK_FALSE;
            colorBlending.logicOp = VK_LOGIC_OP_COPY;
            colorBlending.attachmentCount = 1;
            colorBlending.pAttachments = &colorBlendAttachment.value();

            if (!inputAssembly) {
                inputAssembly = vkh::makeInputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
            }
            if (!rasterizer) {
                rasterizer = vkh::makeRasterisationStateCreateInfo(VK_POLYGON_MODE_FILL);
            }
            if (!multisampling) {
                multisampling = vkh::makeMultisampleStateCreateInfo();
            }

            //we now use all of the info structs we have been writing into into this one to create the pipeline
            VkGraphicsPipelineCreateInfo pipelineInfo = {};
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipelineInfo.pNext = nullptr;

            pipelineInfo.stageCount = _shaderStages.size();
            pipelineInfo.pStages = _shaderStages.data();
            pipelineInfo.pVertexInputState = &vertexInputInfo.value();
            pipelineInfo.pInputAssemblyState = &inputAssembly.value();
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer.value();
            pipelineInfo.pMultisampleState = &multisampling.value();
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.layout = pipelineLayout.value();
            pipelineInfo.renderPass = pass;
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

            //it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
            VkPipeline newPipeline;
            if (vkCreateGraphicsPipelines(
                device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
                std::cout << "failed to create pipline\n";
                exit(-1);
                return VK_NULL_HANDLE; // failed to create graphics pipeline
            }
            else {
                return newPipeline;
            }
        }
    }
}


#include "Vertex.hpp"

namespace daxa {
    namespace vkh {
        VkBufferCreateInfo makeVertexBufferCreateInfo(uz size)
        {
            VkBufferCreateInfo bufferInfo = {};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = size;
            bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            return bufferInfo;
        }

        VertexDiscriptionBuilder& VertexDiscriptionBuilder::beginBinding(u32 stride, VkVertexInputRate inputRate)
        {
            offset = 0;
            location = 0;
            VkVertexInputBindingDescription binding = {};
            binding.binding = bindings.size();
            binding.stride = stride;
            binding.inputRate = inputRate;
            bindings.push_back(binding);
            return *this;
        }

        VertexDiscriptionBuilder& VertexDiscriptionBuilder::setAttribute(VkFormat format)
        {
            switch (format) {
            case VK_FORMAT_R32_SFLOAT:
            case VK_FORMAT_R32_UINT:
            case VK_FORMAT_R32_SINT:
                offset += sizeof(u32);
                break;
            case VK_FORMAT_R32G32_SFLOAT:
            case VK_FORMAT_R32G32_UINT:
            case VK_FORMAT_R32G32_SINT:
                offset += sizeof(u32) * 2;
                break;
            case VK_FORMAT_R32G32B32_SFLOAT:
            case VK_FORMAT_R32G32B32_UINT:
            case VK_FORMAT_R32G32B32_SINT:
                offset += sizeof(u32) * 3;
                break;
            case VK_FORMAT_R32G32B32A32_SFLOAT:
            case VK_FORMAT_R32G32B32A32_UINT:
            case VK_FORMAT_R32G32B32A32_SINT:
                offset += sizeof(u32) * 4;
                break;
            default:
                assert(false);
            }

            VkVertexInputAttributeDescription attribute = {};
            attribute.binding = bindings.size() - 1;
            attribute.location = location;
            attribute.format = format;
            attribute.offset = offset;
            attributes.push_back(attribute);
            location += 1;
            return *this;
        }

        VertexDiscriptionBuilder& VertexDiscriptionBuilder::stageCreateFlags(VkPipelineVertexInputStateCreateFlags flags)
        {
            this->flags = flags;
            return *this;
        }

        VertexDescription VertexDiscriptionBuilder::build()
        {
            assert(bindings.size() > 0);
            return VertexDescription{
                bindings,
                attributes,
                flags
            };
        }
    }
}
