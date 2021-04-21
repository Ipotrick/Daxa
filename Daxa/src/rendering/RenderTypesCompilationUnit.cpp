#include "SimpleMesh.hpp"

#include <tiny_obj_loader.hpp>
#include <iostream>

namespace daxa {
	std::optional<SimpleMesh> daxa::loadMeshFromObj(const char* filename)
	{
		//attrib will contain the vertex arrays of the file
		tinyobj::attrib_t attrib;
		//shapes contains the info for each separate object in the file
		std::vector<tinyobj::shape_t> shapes;
		//materials contains the information about the material of each shape, but we won't use it.
		std::vector<tinyobj::material_t> materials;

		//error and warning output from the load function
		std::string warn;
		std::string err;



		//load the OBJ file
		tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename, nullptr);
		//make sure to output the warnings to the console, in case there are issues with the file
		if (!warn.empty()) {
			std::cout << "WARN: " << warn << std::endl;
		}
		//if we have any error, print it to the console, and break the mesh loading. 
		//This happens if the file can't be found or is malformed
		if (!err.empty()) {
			std::cerr << err << std::endl;
			return {};
		}

		SimpleMesh mesh;

		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

				//hardcode loading to triangles
				int fv = 3;

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

					//vertex position
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					//vertex normal
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

					//copy it into our vertex
					Vertex new_vert;
					new_vert.position.x = vx;
					new_vert.position.y = vy;
					new_vert.position.z = vz;

					new_vert.normal.x = nx;
					new_vert.normal.y = ny;
					new_vert.normal.z = nz;

					//we are setting the vertex color as the vertex normal. This is just for display purposes
					new_vert.color = new_vert.normal;

					mesh.vertices.push_back(new_vert);
				}
				index_offset += fv;
			}
		}

		return mesh;
	}
}
#include "Image.hpp"
#include "Buffer.hpp"

#include "stb_image.hpp"

namespace daxa {
    Image::Image(Image&& other) noexcept
    {
        std::swap(this->image, other.image);
        std::swap(this->view, other.view);
        std::swap(this->info, other.info);
        std::swap(this->viewInfo, other.viewInfo);
        std::swap(this->allocation, other.allocation);
        std::swap(this->allocator, other.allocator);
    }

    Image::operator bool() const {
        return image.operator bool();
    }

    Image& Image::operator=(Image&& other) noexcept
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
        info = vk::ImageCreateInfo{};
        viewInfo = vk::ImageViewCreateInfo{};
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
        vk::ImageViewCreateInfo viewCreateInfo,
        const VmaAllocationCreateInfo& allocInfo,
        vk::Device device,
        VmaAllocator allocator)
    {
        Image img;
        img.allocator = allocator;
        img.info = createInfo;
        vmaCreateImage(img.allocator, (VkImageCreateInfo*)&createInfo, &allocInfo, (VkImage*)&img.image, &img.allocation, nullptr);
        viewCreateInfo.image = img.image;
        img.viewInfo = viewCreateInfo;
        img.view = device.createImageViewUnique(viewCreateInfo);
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

        Buffer stagingBuffer = createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, VMA_MEMORY_USAGE_CPU_ONLY, allocator);

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

        Image newImage;

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
        VulkanContext::mainTransferQueue.submit(vk::SubmitInfo{ .pCommandBuffers = &cmd }, fence);
        device.waitForFences(fence, true, 9999999999);


        auto viewCI = vk::ImageViewCreateInfo{
            .image = newImage.image,
            .viewType = vk::ImageViewType::e2D,
            .format = newImage.info.format,
            .subresourceRange = vk::ImageSubresourceRange {
                .aspectMask = vk::ImageAspectFlagBits::eColor,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        };
        newImage.viewInfo = viewCI;
        newImage.view = device.createImageViewUnique(viewCI);

        return std::move(newImage);
    }
}