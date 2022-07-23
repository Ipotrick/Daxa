#include "impl_core.hpp"

namespace daxa
{
    Handle::Handle(std::shared_ptr<void> a_impl) : impl{std::move(a_impl)} {}
    
    auto Access::operator | (Access const & other) -> Access
    {
        return Access{ .stage = this->stage | other.stage, .type = this->type | other.type };
    }

    auto Access::operator & (Access const & other) -> Access
    {
        return Access{ .stage = this->stage & other.stage, .type = this->type & other.type };
    }

    auto to_string(Access const & access) -> std::string_view
    {
        switch(access.stage) {
        case AccessConsts::NONE.stage: return std::string_view{"NONE"};

        case AccessConsts::TOP_OF_PIPE_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"TOP_OF_PIPE_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"TOP_OF_PIPE_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"TOP_OF_PIPE_READ_WRITE"};
            }
        case AccessConsts::DRAW_INDIRECT_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"DRAW_INDIRECT_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"DRAW_INDIRECT_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"DRAW_INDIRECT_READ_WRITE"};
            }
        case AccessConsts::VERTEX_SHADER_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"VERTEX_SHADER_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"VERTEX_SHADER_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"VERTEX_SHADER_READ_WRITE"};
            }
        case AccessConsts::TESSELLATION_CONTROL_SHADER_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"TESSELLATION_CONTROL_SHADER_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"TESSELLATION_CONTROL_SHADER_READ_WRITE"};
            }
        case AccessConsts::TESSELLATION_EVALUATION_SHADER_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"TESSELLATION_EVALUATION_SHADER_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"TESSELLATION_EVALUATION_SHADER_READ_WRITE"};
            }
        case AccessConsts::GEOMETRY_SHADER_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"GEOMETRY_SHADER_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"GEOMETRY_SHADER_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"GEOMETRY_SHADER_READ_WRITE"};
            }
        case AccessConsts::FRAGMENT_SHADER_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"FRAGMENT_SHADER_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"FRAGMENT_SHADER_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"FRAGMENT_SHADER_READ_WRITE"};
            }
        case AccessConsts::EARLY_FRAGMENT_TESTS_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"EARLY_FRAGMENT_TESTS_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"EARLY_FRAGMENT_TESTS_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"EARLY_FRAGMENT_TESTS_READ_WRITE"};
            }
        case AccessConsts::LATE_FRAGMENT_TESTS_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"LATE_FRAGMENT_TESTS_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"LATE_FRAGMENT_TESTS_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"LATE_FRAGMENT_TESTS_READ_WRITE"};
            }
        case AccessConsts::COLOR_ATTACHMENT_OUTPUT_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"COLOR_ATTACHMENT_OUTPUT_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"COLOR_ATTACHMENT_OUTPUT_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"COLOR_ATTACHMENT_OUTPUT_READ_WRITE"};
            }
        case AccessConsts::COMPUTE_SHADER_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"COMPUTE_SHADER_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"COMPUTE_SHADER_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"COMPUTE_SHADER_READ_WRITE"};
            }
        case AccessConsts::TRANSFER_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"TRANSFER_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"TRANSFER_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"TRANSFER_READ_WRITE"};
            }
        case AccessConsts::BOTTOM_OF_PIPE_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"BOTTOM_OF_PIPE_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"BOTTOM_OF_PIPE_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"BOTTOM_OF_PIPE_READ_WRITE"};
            }
        case AccessConsts::HOST_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"HOST_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"HOST_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"HOST_READ_WRITE"};
            }
        case AccessConsts::ALL_GRAPHICS_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"ALL_GRAPHICS_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"ALL_GRAPHICS_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"ALL_GRAPHICS_READ_WRITE"};
            }
        case AccessConsts::READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"READ_WRITE"};
            }
        case AccessConsts::COPY_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"COPY_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"COPY_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"COPY_READ_WRITE"};
            }
        case AccessConsts::RESOLVE_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"RESOLVE_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"RESOLVE_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"RESOLVE_READ_WRITE"};
            }
        case AccessConsts::BLIT_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"BLIT_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"BLIT_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"BLIT_READ_WRITE"};
            }
        case AccessConsts::CLEAR_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"CLEAR_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"CLEAR_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"CLEAR_READ_WRITE"};
            }
        case AccessConsts::INDEX_INPUT_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"INDEX_INPUT_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"INDEX_INPUT_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"INDEX_INPUT_READ_WRITE"};
            }
        case AccessConsts::PRE_RASTERIZATION_SHADERS_READ.stage:
            switch(access.type)
            {
                case AccessTypeFlagBits::READ: return std::string_view{"PRE_RASTERIZATION_SHADERS_READ"};
                case AccessTypeFlagBits::WRITE: return std::string_view{"PRE_RASTERIZATION_SHADERS_WRITE"};
                case AccessTypeFlagBits::READ_WRITE: return std::string_view{"PRE_RASTERIZATION_SHADERS_READ_WRITE"};
            }
        default: return std::string_view{"unknown or composite access"}; break;
        }
    }
} // namespace daxa
