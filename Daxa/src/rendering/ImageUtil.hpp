#pragma once

#include "../DaxaCore.hpp"

#include <unordered_map>
#include <filesystem>
#include <fstream>

#include "stb_image.h"

#include "../gpu/Device.hpp"

namespace daxa {
    /**
     * @brief generated mipmaps for a 2d texture for layer 0. 
     * The incoming image layout at level 0 must be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
     * All image level layouts will be set to dstImageLayoutsAllLevels in the function.
     * 
     * @param cmdList command list the mip generation commands are recorded to.
     * @param img to generate mipmaps  for. The layout of level 0 must be VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL.
     * @param dstImageLayoutsAllLevels the layout of all image levels after the mip generation.
     */
    void generateMipLevels(CommandListHandle& cmdList, ImageViewHandle& img, VkImageSubresourceLayers layers, VkImageLayout postImageLayerLayouts);
}