#include "util.hpp"

namespace daxa {
    std::size_t sizeofFormat(VkFormat format) {
        if (format == VkFormat::VK_FORMAT_UNDEFINED)
            return -1;
        if (format == VkFormat::VK_FORMAT_R4G4_UNORM_PACK8)
            return 1;
        if (format >= VkFormat::VK_FORMAT_R4G4B4A4_UNORM_PACK16 && format <= VkFormat::VK_FORMAT_A1R5G5B5_UNORM_PACK16)
            return 2;
        if (format >= VkFormat::VK_FORMAT_R8_UNORM && format <= VkFormat::VK_FORMAT_R8_SRGB)
            return 1;
        if (format >= VkFormat::VK_FORMAT_R8G8_UNORM && format <= VkFormat::VK_FORMAT_R8G8_SRGB)
            return 2;
        if (format >= VkFormat::VK_FORMAT_R8G8B8_UNORM && format <= VkFormat::VK_FORMAT_R8G8B8_SRGB)
            return 3;
        if (format >= VkFormat::VK_FORMAT_R8G8B8_UNORM && format <= VkFormat::VK_FORMAT_A2B10G10R10_SINT_PACK32)
            return 4;
        if (format >= VkFormat::VK_FORMAT_R16_UNORM && format <= VkFormat::VK_FORMAT_R16_SFLOAT)
            return 2;
        if (format >= VkFormat::VK_FORMAT_R16G16_UNORM && format <= VkFormat::VK_FORMAT_R16G16_SFLOAT)
            return 4;
        if (format >= VkFormat::VK_FORMAT_R16G16B16_UNORM && format <= VkFormat::VK_FORMAT_R16G16B16_SFLOAT)
            return 6;
        if (format >= VkFormat::VK_FORMAT_R16G16B16A16_UNORM && format <= VkFormat::VK_FORMAT_R16G16B16A16_SFLOAT)
            return 8;
        if (format >= VkFormat::VK_FORMAT_R32_UINT && format <= VkFormat::VK_FORMAT_R32_SFLOAT)
            return 4;
        if (format >= VkFormat::VK_FORMAT_R32G32_UINT && format <= VkFormat::VK_FORMAT_R32G32_SFLOAT)
            return 8;
        if (format >= VkFormat::VK_FORMAT_R32G32B32_UINT && format <= VkFormat::VK_FORMAT_R32G32B32_SFLOAT)
            return 12;
        if (format >= VkFormat::VK_FORMAT_R32G32B32A32_UINT && format <= VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT)
            return 16;
        if (format >= VkFormat::VK_FORMAT_R64_UINT && format <= VkFormat::VK_FORMAT_R64_SFLOAT)
            return 8;
        if (format >= VkFormat::VK_FORMAT_R64G64_UINT && format <= VkFormat::VK_FORMAT_R64G64_SFLOAT)
            return 16;
        if (format >= VkFormat::VK_FORMAT_R64G64B64_UINT && format <= VkFormat::VK_FORMAT_R64G64B64_SFLOAT)
            return 24;
        if (format >= VkFormat::VK_FORMAT_R64G64B64A64_UINT && format <= VkFormat::VK_FORMAT_R64G64B64A64_SFLOAT)
            return 32;
        if (format == VkFormat::VK_FORMAT_B10G11R11_UFLOAT_PACK32 || format == VkFormat::VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)
            return 32;
        if (format == VkFormat::VK_FORMAT_D16_UNORM)
            return 16;
        if (format == VkFormat::VK_FORMAT_X8_D24_UNORM_PACK32 || format == VkFormat::VK_FORMAT_D32_SFLOAT)
            return 32;
        if (format == VkFormat::VK_FORMAT_S8_UINT)
            return 8;
        return -1;
    }
}