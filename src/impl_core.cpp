#include "impl_core.hpp"

namespace daxa
{
    Handle::Handle(std::shared_ptr<void> a_impl) : impl{std::move(a_impl)} {}
    
    auto Access::operator | (Access const & other) -> Access
    {
        return Access{ .stages = this->stages | other.stages, .type = this->type | other.type };
    }

    auto Access::operator & (Access const & other) -> Access
    {
        return Access{ .stages = this->stages & other.stages, .type = this->type & other.type };
    }

    auto to_string(AccessTypeFlags flags) -> std::string_view
    {
        switch (flags)
        {
        case AccessTypeFlagBits::NONE: return "NONE";
        case AccessTypeFlagBits::READ: return "READ";
        case AccessTypeFlagBits::WRITE: return "WRITE";
        case AccessTypeFlagBits::READ_WRITE: return "READ_WRITE";
        default: DAXA_DBG_ASSERT_TRUE_M(false, "invalid AccessTypeFlags");
        }
        return "invalid AccessTypeFlags";
    }

    auto to_string(ImageLayout layout) -> std::string_view
    {
        switch (layout)
        {
        case ImageLayout::UNDEFINED: return "UNDEFINED";
        case ImageLayout::GENERAL: return "GENERAL";
        case ImageLayout::COLOR_ATTACHMENT_OPTIMAL: return "COLOR_ATTACHMENT_OPTIMAL";
        case ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL: return "DEPTH_STENCIL_ATTACHMENT_OPTIMAL";
        case ImageLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL: return "DEPTH_STENCIL_READ_ONLY_OPTIMAL";
        case ImageLayout::SHADER_READ_ONLY_OPTIMAL: return "SHADER_READ_ONLY_OPTIMAL";
        case ImageLayout::TRANSFER_SRC_OPTIMAL: return "TRANSFER_SRC_OPTIMAL";
        case ImageLayout::TRANSFER_DST_OPTIMAL: return "TRANSFER_DST_OPTIMAL";
        //PREINITIALIZED = 8,
        case ImageLayout::DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL: return "DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL";
        case ImageLayout::DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL: return "DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL";
        case ImageLayout::DEPTH_ATTACHMENT_OPTIMAL: return "DEPTH_ATTACHMENT_OPTIMAL";
        case ImageLayout::DEPTH_READ_ONLY_OPTIMAL: return "DEPTH_READ_ONLY_OPTIMAL";
        case ImageLayout::STENCIL_ATTACHMENT_OPTIMAL: return "STENCIL_ATTACHMENT_OPTIMAL";
        case ImageLayout::STENCIL_READ_ONLY_OPTIMAL: return "STENCIL_READ_ONLY_OPTIMAL";
        //READ_ONLY_OPTIMAL = 1000314000,
        case ImageLayout::ATTACHMENT_OPTIMAL: return "ATTACHMENT_OPTIMAL";
        case ImageLayout::PRESENT_SRC: return "PRESENT_SRC";
        //SHARED_PRESENT = 1000111000,
        //FRAGMENT_DENSITY_MAP_OPTIMAL_EXT = 1000218000,
        //FRAGMENT_SHADING_RATE_ATTACHMENT_OPTIMAL = 1000164003,
        default: DAXA_DBG_ASSERT_TRUE_M(false, "invalid ImageLayout");
        }
    }

    auto to_string(PipelineStageFlags flags) -> std::string
    {
        if (flags == PipelineStageFlagBits::NONE)
        {
            return "NONE";
        }

        std::string ret = {};
        
        if (flags & PipelineStageFlagBits::TOP_OF_PIPE)
        {
            ret += "TOP_OF_PIPE";
        }
        if (flags & PipelineStageFlagBits::DRAW_INDIRECT)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "DRAW_INDIRECT";
        }
        if (flags & PipelineStageFlagBits::VERTEX_SHADER)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "VERTEX_SHADER";
        }
        if (flags & PipelineStageFlagBits::TESSELLATION_CONTROL_SHADER)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "TESSELLATION_CONTROL_SHADER";
        }
        if (flags & PipelineStageFlagBits::TESSELLATION_EVALUATION_SHADER)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "TESSELLATION_EVALUATION_SHADER";
        }
        if (flags & PipelineStageFlagBits::GEOMETRY_SHADER)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "GEOMETRY_SHADER";
        }
        if (flags & PipelineStageFlagBits::FRAGMENT_SHADER)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "FRAGMENT_SHADER";
        }
        if (flags & PipelineStageFlagBits::EARLY_FRAGMENT_TESTS)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "EARLY_FRAGMENT_TESTS";
        }
        if (flags & PipelineStageFlagBits::LATE_FRAGMENT_TESTS)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "LATE_FRAGMENT_TESTS";
        }
        if (flags & PipelineStageFlagBits::COLOR_ATTACHMENT_OUTPUT)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "COLOR_ATTACHMENT_OUTPUT";
        }
        if (flags & PipelineStageFlagBits::COMPUTE_SHADER)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "COMPUTE_SHADER";
        }
        if (flags & PipelineStageFlagBits::TRANSFER)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "TRANSFER";
        }
        if (flags & PipelineStageFlagBits::BOTTOM_OF_PIPE)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "BOTTOM_OF_PIPE";
        }
        if (flags & PipelineStageFlagBits::HOST)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "HOST";
        }
        if (flags & PipelineStageFlagBits::ALL_GRAPHICS)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "ALL_GRAPHICS";
        }
        if (flags & PipelineStageFlagBits::ALL_COMMANDS)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "ALL_COMMANDS";
        }
        if (flags & PipelineStageFlagBits::COPY)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "COPY";
        }
        if (flags & PipelineStageFlagBits::RESOLVE)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "RESOLVE";
        }
        if (flags & PipelineStageFlagBits::BLIT)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "BLIT";
        }
        if (flags & PipelineStageFlagBits::CLEAR)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "CLEAR";
        }
        if (flags & PipelineStageFlagBits::INDEX_INPUT)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "INDEX_INPUT";
        }
        if (flags & PipelineStageFlagBits::PRE_RASTERIZATION_SHADERS)
        {
            if (ret.size() != 0) 
            {
                ret += " | ";
            }
            ret += "PRE_RASTERIZATION_SHADERS";
        }
        return ret;
    }

    auto to_string(Access access) -> std::string
    {
        std::string ret = {};
        ret += "{ stages: ";
        ret += to_string(access.stages);
        ret += ", accessType: ";
        ret += to_string(access.type);
        ret += " }";
        return ret;
    }
} // namespace daxa
