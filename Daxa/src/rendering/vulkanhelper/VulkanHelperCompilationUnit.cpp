#include <fstream>
#include <ios>
#include <iostream>
#include <vector>


#include "Initialization.hpp"

namespace daxa {
    namespace vkh {
        vk::FenceCreateInfo makeDefaultFenceCI()
        {
            vk::FenceCreateInfo info{};
            info.flags |= vk::FenceCreateFlagBits::eSignaled;
            return info;
        }

        vk::SemaphoreCreateInfo makeDefaultSemaphoreCI()
        {
            return vk::SemaphoreCreateInfo();
        }

        vk::PipelineRasterizationStateCreateInfo makeDefaultPipelineRasterizationSCI()
        {
            return vk::PipelineRasterizationStateCreateInfo{
                .polygonMode = vk::PolygonMode::eFill,
                .cullMode = vk::CullModeFlagBits::eNone,
                .frontFace = vk::FrontFace::eClockwise,
                .lineWidth = 1.0f,
            };
        }

        vk::AttachmentDescription makeDefaultAttackmentDescription()
        {
            return vk::AttachmentDescription{
                .format = vk::Format::eUndefined,
                .samples = vk::SampleCountFlagBits::e1,
                .loadOp = vk::AttachmentLoadOp::eClear,
                .storeOp = vk::AttachmentStoreOp::eStore,
                .stencilLoadOp = vk::AttachmentLoadOp::eDontCare,
                .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
                .initialLayout = vk::ImageLayout::eUndefined,
                .finalLayout = vk::ImageLayout::ePresentSrcKHR,
            };
        }
    }
}


#include "CommandPool.hpp"

namespace daxa {
    namespace vkh {
        vk::UniqueCommandPool makeCommandPool(
            u32 queueFamilyIndex,
            vk::CommandPoolCreateFlagBits flags,
            vk::Device device)
        {
            vk::CommandPoolCreateInfo cmdPoolCreateInfo = {};
            cmdPoolCreateInfo.flags = flags;   // we can reset individual command buffers from this pool
            cmdPoolCreateInfo.queueFamilyIndex = queueFamilyIndex;

            return device.createCommandPoolUnique(cmdPoolCreateInfo);
        }

        CommandPool::CommandPool(
            u32 queueFamilyIndex,
            vk::CommandPoolCreateFlagBits flags,
            vk::Device device
        ) :
            queueFamilyIndex{ queueFamilyIndex },
            device{ device },
            pool{ makeCommandPool(queueFamilyIndex, flags, device) },
            bufferPool{
                [=]() { return device.allocateCommandBuffers(vk::CommandBufferAllocateInfo{.commandPool= *pool, .commandBufferCount= 1}).front(); },
                [=](vk::CommandBuffer buffer) { /* gets freed anyway */ },
                [=](vk::CommandBuffer buffer) { buffer.reset(); }
            }
        { }

        CommandPool::CommandPool(CommandPool&& other) noexcept
        {
            this->queueFamilyIndex = other.queueFamilyIndex;
            this->device = std::move(other.device);
            this->pool = std::move(other.pool);
            this->bufferPool = std::move(other.bufferPool);
            other.queueFamilyIndex = 0xFFFFFFFF;
        }

        CommandPool::operator const vk::UniqueCommandPool&()
        {
            assert(pool); // USE AFTER MOVE
            return pool;
        }

        const vk::UniqueCommandPool& CommandPool::get()
        {
            assert(pool); // USE AFTER MOVE
            return pool;
        }

        vk::CommandBuffer CommandPool::getBuffer()
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

        std::optional<vk::UniqueShaderModule> loadShaderModule(std::string filePath, vk::Device device)
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
            vk::ShaderModuleCreateInfo createInfo = {};

            //codeSize has to be in bytes, so multply the ints in the buffer by size of int to know the real size of the buffer
            createInfo.codeSize = buffer.size() * sizeof(u32);
            createInfo.pCode = buffer.data();

            //check that the creation goes well.
            vk::ShaderModule shaderModule;
            return std::move(device.createShaderModuleUnique(createInfo));
        }

        vk::PipelineShaderStageCreateInfo makeShaderStageCreateInfo(vk::ShaderStageFlagBits stage, vk::ShaderModule shaderModule)
        {
            vk::PipelineShaderStageCreateInfo info{};

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

        vk::PipelineVertexInputStateCreateInfo makeVertexInputStageCreateInfo()
        {
            vk::PipelineVertexInputStateCreateInfo info = {};

            //no vertex bindings or attributes
            info.vertexBindingDescriptionCount = 0;
            info.vertexAttributeDescriptionCount = 0;
            return info;
        }

        vk::PipelineInputAssemblyStateCreateInfo makeInputAssemblyStateCreateInfo(vk::PrimitiveTopology topology)
        {
            vk::PipelineInputAssemblyStateCreateInfo info = {};
            info.topology = topology;
            //we are not going to use primitive restart on the entire tutorial so leave it on false
            info.primitiveRestartEnable = VK_FALSE;
            return info;
        }

        vk::PipelineRasterizationStateCreateInfo makeRasterisationStateCreateInfo(vk::PolygonMode polygonMode)
        {
            vk::PipelineRasterizationStateCreateInfo info = {};

            info.depthClampEnable = VK_FALSE;
            //discards all primitives before the rasterization stage if enabled which we don't want
            info.rasterizerDiscardEnable = VK_FALSE;

            info.polygonMode = polygonMode;
            info.lineWidth = 1.0f;
            //no backface cull
            info.cullMode = vk::CullModeFlagBits::eNone;
            info.frontFace = vk::FrontFace::eClockwise;
            //no depth bias
            info.depthBiasEnable = VK_FALSE;
            info.depthBiasConstantFactor = 0.0f;
            info.depthBiasClamp = 0.0f;
            info.depthBiasSlopeFactor = 0.0f;

            return info;
        }

        vk::PipelineMultisampleStateCreateInfo makeMultisampleStateCreateInfo()
        {
            vk::PipelineMultisampleStateCreateInfo info = {};

            info.sampleShadingEnable = VK_FALSE;
            //multisampling defaulted to no multisampling (1 sample per pixel)
            info.rasterizationSamples = vk::SampleCountFlagBits::e1;
            info.minSampleShading = 1.0f;
            info.pSampleMask = nullptr;
            info.alphaToCoverageEnable = VK_FALSE;
            info.alphaToOneEnable = VK_FALSE;
            return info;
        }

        vk::PipelineColorBlendAttachmentState makeColorBlendSAttachmentState()
        {
            vk::PipelineColorBlendAttachmentState colorBlendAttachment = {};
            colorBlendAttachment.colorWriteMask =
                vk::ColorComponentFlagBits::eR |
                vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB |
                vk::ColorComponentFlagBits::eA;
            colorBlendAttachment.blendEnable = VK_FALSE;
            return colorBlendAttachment;
        }

        vk::PipelineLayoutCreateInfo makePipelineLayoutCreateInfo()
        {
            return vk::PipelineLayoutCreateInfo{};
        }

        Pipeline  daxa::vkh::PipelineBuilder::build(vk::RenderPass pass, u32 subpass, vk::Device device)
        {
            if (!vertexInput) {
                std::cout << "error: vertexInput was not specified in pipeline builder!\n";
                exit(-1);
            }

            // set state create infos:
            vk::PipelineVertexInputStateCreateInfo    pvertexInputCI = vertexInput.value();
            vk::PipelineColorBlendAttachmentState     pcolorBlendAttachmentCI = colorBlendAttachment.value_or(vkh::makeColorBlendSAttachmentState());
            vk::PipelineInputAssemblyStateCreateInfo  pinputAssemlyStateCI = inputAssembly.value_or(vkh::makeInputAssemblyStateCreateInfo(vk::PrimitiveTopology::eTriangleList ));
            vk::PipelineRasterizationStateCreateInfo  prasterizationStateCI = rasterization.value_or(vkh::makeRasterisationStateCreateInfo(vk::PolygonMode::eFill));
            vk::PipelineMultisampleStateCreateInfo    multisamplerStateCI = multisampling.value_or(vkh::makeMultisampleStateCreateInfo());
            vk::PipelineDepthStencilStateCreateInfo   pDepthStencilStateCI = depthStencil.value_or(vk::PipelineDepthStencilStateCreateInfo{});
            vk::Viewport                              pviewport = viewport.value_or(vk::Viewport{ .width = 1,.height = 1 });
            vk::Rect2D                                pscissor = scissor.value_or(vk::Rect2D{ .extent = {static_cast<u32>(pviewport.width), static_cast<u32>(pviewport.height)} });

            Pipeline pipeline;

            //build pipeline layout:
            vk::PipelineLayoutCreateInfo layoutCI{
                .pushConstantRangeCount = u32(pushConstants.size()),
                .pPushConstantRanges = pushConstants.data(),
            };
            pipeline.layout = device.createPipelineLayoutUnique(layoutCI);

            vk::PipelineViewportStateCreateInfo viewportStateCI{
                .viewportCount = 1,
                .pViewports = &pviewport,
                .scissorCount = 1,
                .pScissors = &pscissor,
            };

            //setup dummy color blending. We aren't using transparent objects yet
            //the blending is just "no blend", but we do write to the color attachment
            vk::PipelineColorBlendStateCreateInfo colorBlendingSCI{
                .logicOpEnable = VK_FALSE,
                .logicOp = vk::LogicOp::eCopy,
                .attachmentCount = 1,
                .pAttachments = &pcolorBlendAttachmentCI,
            };

            // dynamic state setup:
            if (std::find(dynamicStateEnable.begin(), dynamicStateEnable.end(), vk::DynamicState::eViewport) == dynamicStateEnable.end()) {
                dynamicStateEnable.push_back(vk::DynamicState::eViewport);
            }
            if (std::find(dynamicStateEnable.begin(), dynamicStateEnable.end(), vk::DynamicState::eScissor) == dynamicStateEnable.end()) {
                dynamicStateEnable.push_back(vk::DynamicState::eScissor);
            }

            vk::PipelineDynamicStateCreateInfo dynamicStateCI{
                .dynamicStateCount = (u32)dynamicStateEnable.size(),
                .pDynamicStates = dynamicStateEnable.data(),
            };

            //we now use all of the info structs we have been writing into into this one to create the pipeline
            vk::GraphicsPipelineCreateInfo pipelineCI{
                .stageCount = (u32)shaderStages.size(),
                .pStages = shaderStages.data(),
                .pVertexInputState = &pvertexInputCI,
                .pInputAssemblyState = &pinputAssemlyStateCI,
                .pViewportState = &viewportStateCI,
                .pRasterizationState = &prasterizationStateCI,
                .pMultisampleState = &multisamplerStateCI,
                .pDepthStencilState = &pDepthStencilStateCI,
                .pColorBlendState = &colorBlendingSCI,
                .pDynamicState = &dynamicStateCI,
                .layout = pipeline.layout.get(),
                .renderPass = pass,
                .subpass = subpass,
            };

            auto ret = device.createGraphicsPipelineUnique({}, pipelineCI);
            if (ret.result != vk::Result::eSuccess) {
                std::cerr << "error: could not compile pipeline!\n";
                exit(-1);
            }
            pipeline.pipeline = std::move(ret.value);

            return pipeline;
        }
    }
}


#include "Vertex.hpp"

namespace daxa {
    namespace vkh {

        VertexDiscriptionBuilder& VertexDiscriptionBuilder::beginBinding(u32 stride, vk::VertexInputRate inputRate)
        {
            offset = 0;
            location = 0;
            vk::VertexInputBindingDescription binding{
                .binding = (u32)bindings.size(),
                .stride = stride,
                .inputRate = inputRate,
            };
            bindings.push_back(binding);
            return *this;
        }

        VertexDiscriptionBuilder& VertexDiscriptionBuilder::setAttribute(vk::Format format)
        {
            vk::VertexInputAttributeDescription attribute{
                .location = location,
                .binding = u32(bindings.size() - 1),
                .format = format,
                .offset = offset,
            };

            attributes.push_back(attribute);

            location += 1;

            switch (format) {
            case vk::Format::eR32G32B32A32Sfloat:
            case vk::Format::eR32G32B32A32Sint:
            case vk::Format::eR32G32B32A32Uint:
                offset += sizeof(f32) * 4;
                break;
            case vk::Format::eR32G32B32Sfloat:
            case vk::Format::eR32G32B32Sint:
            case vk::Format::eR32G32B32Uint:
                offset += sizeof(f32) * 3;
                break;
            case vk::Format::eR32G32Sfloat:
            case vk::Format::eR32G32Sint:
            case vk::Format::eR32G32Uint:
                offset += sizeof(f32) * 2;
                break;
            case vk::Format::eR32Sfloat:
            case vk::Format::eR32Sint:
            case vk::Format::eR32Uint:
                offset += sizeof(f32) * 1;
                break;
            default:
                assert(false);
            }

            return *this;
        }

        VertexDiscriptionBuilder& VertexDiscriptionBuilder::stageCreateFlags(vk::PipelineVertexInputStateCreateFlags flags)
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

#include "Image.hpp"
#include "Buffer.hpp"

#include "stb_image.hpp"

namespace daxa {
    namespace vkh {

        Image::Image(Image && other) noexcept
        {
            std::swap(this->image, other.image);
            std::swap(this->allocation, other.allocation);
            std::swap(this->allocator, other.allocator);
        }

        Image& Image::operator=(Image && other) noexcept
        {
            Image::~Image();
            new(this) Image(std::move(other));
            return *this;
        }

        Image::~Image()
        {
            if (valid()) {
                vmaDestroyImage(allocator, image, allocation);
            }
            image = vk::Image{};
            allocation = {};
            allocator = {};
        }

        void Image::reset()
        {
            Image::~Image();
        }

        bool Image::valid() const
        {
            return image.operator bool();
        }

        Image makeImage(
            const vk::ImageCreateInfo& createInfo,
            const VmaAllocationCreateInfo& allocInfo,
            VmaAllocator allocator)
        {
            Image img;
            img.allocator = allocator;
            vmaCreateImage(img.allocator, (VkImageCreateInfo*)&createInfo, &allocInfo, (VkImage*)&img.image, &img.allocation, nullptr);
            return std::move(img);
        }

        Image loadImage(vk::CommandBuffer& cmd, vk::Fence fence, std::string& path, vk::Device device, VmaAllocator allocator)
        {
            i32 width;
            i32 height;
            i32 channels;

            // load image data from disc
            stbi_uc* loadedData = stbi_load(path.c_str(), &width, &height, &channels, STBI_rgb_alpha);

            if (!loadedData) {
                std::cout << "Warning: could not load file from path: " << path << std::endl;
                return {};
            }


            // move cpu data into a staging buffer, so that we can read the image data on the gpu:
            void* pixel_ptr = loadedData;
            VkDeviceSize imageSize = width * height * 4;

            vk::Format image_format = vk::Format::eR8G8B8A8Srgb;

            vkh::Buffer stagingBuffer = createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY, allocator);

            void* data;
            vmaMapMemory(allocator, stagingBuffer.allocation, &data);

            memcpy(data, pixel_ptr, static_cast<size_t>(imageSize));

            vmaUnmapMemory(allocator, stagingBuffer.allocation);

            stbi_image_free(loadedData);


            // create image:
            vk::Extent3D imageExtent;
            imageExtent.width = static_cast<u32>(width);
            imageExtent.height = static_cast<u32>(height);
            imageExtent.depth = 1;

            vk::ImageCreateInfo imgCI{
                .imageType = vk::ImageType::e2D,
                .format = image_format,
                .extent = imageExtent,
                .mipLevels = 1,
                .arrayLayers = 1,
                .usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst,
            };

            vkh::Image newImage;

            VmaAllocationCreateInfo dimg_allocinfo = {};
            dimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            vmaCreateImage(allocator, (VkImageCreateInfo*)&imgCI, &dimg_allocinfo, (VkImage*)&newImage.image, &newImage.allocation, nullptr);
            newImage.allocator = allocator;


            // change image layout to transfer dist optiomal:
            VkImageSubresourceRange range;
            range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            range.baseMipLevel = 0;
            range.levelCount = 1;
            range.baseArrayLayer = 0;
            range.layerCount = 1;

            VkImageMemoryBarrier imageBarrier_toTransfer = {};
            imageBarrier_toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

            imageBarrier_toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageBarrier_toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageBarrier_toTransfer.image = newImage.image;
            imageBarrier_toTransfer.subresourceRange = range;

            imageBarrier_toTransfer.srcAccessMask = 0;
            imageBarrier_toTransfer.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            cmd.begin({ .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit });

            vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier_toTransfer);


            //copy the buffer into the image:
            VkBufferImageCopy copyRegion = {};
            copyRegion.bufferOffset = 0;
            copyRegion.bufferRowLength = 0;
            copyRegion.bufferImageHeight = 0;

            copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            copyRegion.imageSubresource.mipLevel = 0;
            copyRegion.imageSubresource.baseArrayLayer = 0;
            copyRegion.imageSubresource.layerCount = 1;
            copyRegion.imageExtent = imageExtent;

            vkCmdCopyBufferToImage(cmd, stagingBuffer.buffer, newImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

            cmd.end();
            vkh::mainTransferQueue.submit(vk::SubmitInfo{ .pCommandBuffers = &cmd }, fence);
            device.waitForFences(fence, true, 9999999999);

            return std::move(newImage);
        }
    }
}
