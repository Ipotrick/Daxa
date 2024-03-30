#pragma once

// Disable msvc warning on alignment padding.
#if defined(_MSC_VER)
#pragma warning(disable : 4324)
#endif

#if !DAXA_BUILT_WITH_UTILS_TASK_GRAPH
#error "[package management error] You must build Daxa with the DAXA_ENABLE_UTILS_TASK_GRAPH CMake option enabled, or request the utils-task-graph feature in vcpkg"
#endif

#include <array>
#include <string_view>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <span>
#include <memory>
#include <vector>

#include <daxa/core.hpp>
#include <daxa/device.hpp>
#include <daxa/utils/mem.hpp>

namespace daxa
{
    enum struct TaskBufferAccess
    {
        NONE,
        GRAPHICS_SHADER_READ,
        GRAPHICS_SHADER_WRITE,
        GRAPHICS_SHADER_READ_WRITE,
        GRAPHICS_SHADER_READ_WRITE_CONCURRENT,
        COMPUTE_SHADER_READ,
        COMPUTE_SHADER_WRITE,
        COMPUTE_SHADER_READ_WRITE,
        COMPUTE_SHADER_READ_WRITE_CONCURRENT,
        RAY_TRACING_SHADER_READ,
        RAY_TRACING_SHADER_WRITE,
        RAY_TRACING_SHADER_READ_WRITE,
        RAY_TRACING_SHADER_READ_WRITE_CONCURRENT,
        TASK_SHADER_READ,
        TASK_SHADER_WRITE,
        TASK_SHADER_READ_WRITE,
        TASK_SHADER_READ_WRITE_CONCURRENT,
        MESH_SHADER_READ,
        MESH_SHADER_WRITE,
        MESH_SHADER_READ_WRITE,
        MESH_SHADER_READ_WRITE_CONCURRENT,
        VERTEX_SHADER_READ,
        VERTEX_SHADER_WRITE,
        VERTEX_SHADER_READ_WRITE,
        VERTEX_SHADER_READ_WRITE_CONCURRENT,
        TESSELLATION_CONTROL_SHADER_READ,
        TESSELLATION_CONTROL_SHADER_WRITE,
        TESSELLATION_CONTROL_SHADER_READ_WRITE,
        TESSELLATION_CONTROL_SHADER_READ_WRITE_CONCURRENT,
        TESSELLATION_EVALUATION_SHADER_READ,
        TESSELLATION_EVALUATION_SHADER_WRITE,
        TESSELLATION_EVALUATION_SHADER_READ_WRITE,
        TESSELLATION_EVALUATION_SHADER_READ_WRITE_CONCURRENT,
        GEOMETRY_SHADER_READ,
        GEOMETRY_SHADER_WRITE,
        GEOMETRY_SHADER_READ_WRITE,
        GEOMETRY_SHADER_READ_WRITE_CONCURRENT,
        FRAGMENT_SHADER_READ,
        FRAGMENT_SHADER_WRITE,
        FRAGMENT_SHADER_READ_WRITE,
        FRAGMENT_SHADER_READ_WRITE_CONCURRENT,
        INDEX_READ,
        DRAW_INDIRECT_INFO_READ,
        TRANSFER_READ,
        TRANSFER_WRITE,
        HOST_TRANSFER_READ,
        HOST_TRANSFER_WRITE,
        MAX_ENUM = 0x7fffffff,
    };

    auto to_string(TaskBufferAccess const & usage) -> std::string_view;

    enum struct TaskImageAccess
    {
        NONE,
        GRAPHICS_SHADER_SAMPLED,
        GRAPHICS_SHADER_STORAGE_WRITE_ONLY,
        GRAPHICS_SHADER_STORAGE_READ_ONLY,
        GRAPHICS_SHADER_STORAGE_READ_WRITE,
        GRAPHICS_SHADER_STORAGE_READ_WRITE_CONCURRENT,
        COMPUTE_SHADER_SAMPLED,
        COMPUTE_SHADER_STORAGE_WRITE_ONLY,
        COMPUTE_SHADER_STORAGE_READ_ONLY,
        COMPUTE_SHADER_STORAGE_READ_WRITE,
        COMPUTE_SHADER_STORAGE_READ_WRITE_CONCURRENT,
        RAY_TRACING_SHADER_SAMPLED,
        RAY_TRACING_SHADER_STORAGE_WRITE_ONLY,
        RAY_TRACING_SHADER_STORAGE_READ_ONLY,
        RAY_TRACING_SHADER_STORAGE_READ_WRITE,
        RAY_TRACING_SHADER_STORAGE_READ_WRITE_CONCURRENT,
        TASK_SHADER_SAMPLED,
        TASK_SHADER_STORAGE_WRITE_ONLY,
        TASK_SHADER_STORAGE_READ_ONLY,
        TASK_SHADER_STORAGE_READ_WRITE,
        TASK_SHADER_STORAGE_READ_WRITE_CONCURRENT,
        MESH_SHADER_SAMPLED,
        MESH_SHADER_STORAGE_WRITE_ONLY,
        MESH_SHADER_STORAGE_READ_ONLY,
        MESH_SHADER_STORAGE_READ_WRITE,
        MESH_SHADER_STORAGE_READ_WRITE_CONCURRENT,
        VERTEX_SHADER_SAMPLED,
        VERTEX_SHADER_STORAGE_WRITE_ONLY,
        VERTEX_SHADER_STORAGE_READ_ONLY,
        VERTEX_SHADER_STORAGE_READ_WRITE,
        VERTEX_SHADER_STORAGE_READ_WRITE_CONCURRENT,
        TESSELLATION_CONTROL_SHADER_SAMPLED,
        TESSELLATION_CONTROL_SHADER_STORAGE_WRITE_ONLY,
        TESSELLATION_CONTROL_SHADER_STORAGE_READ_ONLY,
        TESSELLATION_CONTROL_SHADER_STORAGE_READ_WRITE,
        TESSELLATION_CONTROL_SHADER_STORAGE_READ_WRITE_CONCURRENT,
        TESSELLATION_EVALUATION_SHADER_SAMPLED,
        TESSELLATION_EVALUATION_SHADER_STORAGE_WRITE_ONLY,
        TESSELLATION_EVALUATION_SHADER_STORAGE_READ_ONLY,
        TESSELLATION_EVALUATION_SHADER_STORAGE_READ_WRITE,
        TESSELLATION_EVALUATION_SHADER_STORAGE_READ_WRITE_CONCURRENT,
        GEOMETRY_SHADER_SAMPLED,
        GEOMETRY_SHADER_STORAGE_WRITE_ONLY,
        GEOMETRY_SHADER_STORAGE_READ_ONLY,
        GEOMETRY_SHADER_STORAGE_READ_WRITE,
        GEOMETRY_SHADER_STORAGE_READ_WRITE_CONCURRENT,
        FRAGMENT_SHADER_SAMPLED,
        FRAGMENT_SHADER_STORAGE_WRITE_ONLY,
        FRAGMENT_SHADER_STORAGE_READ_ONLY,
        FRAGMENT_SHADER_STORAGE_READ_WRITE,
        FRAGMENT_SHADER_STORAGE_READ_WRITE_CONCURRENT,
        TRANSFER_READ,
        TRANSFER_WRITE,
        COLOR_ATTACHMENT,
        DEPTH_ATTACHMENT,
        STENCIL_ATTACHMENT,
        DEPTH_STENCIL_ATTACHMENT,
        DEPTH_ATTACHMENT_READ,
        STENCIL_ATTACHMENT_READ,
        DEPTH_STENCIL_ATTACHMENT_READ,
        RESOLVE_WRITE,
        PRESENT,
        MAX_ENUM = 0x7fffffff,
    };

    auto to_string(TaskImageAccess const & usage) -> std::string_view;

    using TaskResourceIndex = u32;

    struct DAXA_EXPORT_CXX TaskGPUResourceView
    {
        TaskResourceIndex task_graph_index = {};
        TaskResourceIndex index = {};

        auto is_empty() const -> bool;
        auto is_persistent() const -> bool;
        auto is_null() const -> bool;

        auto operator<=>(TaskGPUResourceView const & other) const = default;
    };

    auto to_string(TaskGPUResourceView const & id) -> std::string;

    struct TaskBufferView : public TaskGPUResourceView
    {
    };

    static constexpr inline TaskBufferView NullTaskBuffer = []()
    {
        TaskBufferView ret = {};
        ret.task_graph_index = std::numeric_limits<u32>::max();
        ret.index = std::numeric_limits<u32>::max();
        return ret;
    }();

    struct TaskImageView : public TaskGPUResourceView
    {
        daxa::ImageMipArraySlice slice = {};
        auto view(daxa::ImageMipArraySlice const & new_slice) const -> TaskImageView
        {
            auto ret = *this;
            ret.slice = new_slice;
            return ret;
        }
        auto operator<=>(TaskGPUResourceView const & other) const = delete;
        auto operator<=>(TaskImageView const & other) const = default;
    };

    static constexpr inline TaskImageView NullTaskImage = []()
    {
        TaskImageView ret = {};
        ret.task_graph_index = std::numeric_limits<u32>::max();
        ret.index = std::numeric_limits<u32>::max();
        return ret;
    }();

    struct ImageSliceState
    {
        Access latest_access = {};
        ImageLayout latest_layout = {};
        ImageMipArraySlice slice = {};
    };

    enum struct TaskResourceUseType : u32
    {
        NONE = 0,
        BUFFER = 1,
        IMAGE = 2,
        CONSTANT = 3,
        MAX_ENUM = 0xffffffff,
    };

    enum struct TaskHeadImageArrayType : u8
    {
        RUNTIME_IMAGES,
        MIP_LEVELS,
    };

    enum struct TaskAttachmentType
    {
        UNDEFINED,
        BUFFER,
        IMAGE,
    };

    struct UndefinedAttachment
    {
    };

    struct TaskBufferAttachment
    {
        char const * name = {};
        TaskBufferAccess access = {};
        u8 shader_array_size = {};
        bool shader_as_address = {};
    };

    struct TaskImageAttachment
    {
        char const * name = {};
        TaskImageAccess access = {};
        ImageViewType view_type = ImageViewType::MAX_ENUM;
        u8 shader_array_size = {};
        bool shader_as_index = {};
        TaskHeadImageArrayType shader_array_type = {};
    };

    struct TaskBufferInlineAttachment
    {
        char const * name = {};
        TaskBufferAccess access = {};
        u8 shader_array_size = {};
        bool shader_as_address = {};
        TaskBufferView view = {};
    };

    struct TaskImageInlineAttachment
    {
        char const * name = {};
        TaskImageAccess access = {};
        ImageViewType view_type = ImageViewType::MAX_ENUM;
        u8 shader_array_size = {};
        TaskHeadImageArrayType shader_array_type = {};
        TaskImageView view = {};
    };

    struct TaskBufferAttachmentIndex
    {
        u32 value;
    };

    struct TaskImageAttachmentIndex
    {
        u32 value;
    };

    template <typename T>
    concept TaskAttachmentIndex = requires(T t) {
        std::is_same_v<T, TaskBufferAttachmentIndex> || std::is_same_v<T, TaskImageAttachmentIndex>;
    };

    struct TaskAttachment
    {
        TaskAttachmentType type = TaskAttachmentType::UNDEFINED;
        union Value
        {
            UndefinedAttachment undefined;
            TaskBufferAttachment buffer;
            TaskImageAttachment image;
        } value = {.undefined = {}};

        constexpr TaskAttachment() = default;

        constexpr TaskAttachment(TaskBufferAttachment const & buffer)
            : type{TaskAttachmentType::BUFFER}, value{.buffer = buffer}
        {
        }

        constexpr TaskAttachment(TaskImageAttachment const & image)
            : type{TaskAttachmentType::IMAGE}, value{.image = image}
        {
        }

        constexpr auto name() const -> char const *
        {
            switch (type)
            {
            case TaskAttachmentType::BUFFER: return value.buffer.name;
            case TaskAttachmentType::IMAGE: return value.image.name;
            default: return "undefined";
            }
        }

        constexpr auto shader_array_size() const -> u32
        {
            switch (type)
            {
            case TaskAttachmentType::BUFFER: return value.buffer.shader_array_size * 8;
            case TaskAttachmentType::IMAGE: return value.image.shader_array_size * (value.image.shader_as_index ? 4 : 8);
            default: return 0;
            }
        }

        constexpr auto shader_element_align() const -> u32
        {
            switch (type)
            {
            case TaskAttachmentType::BUFFER: return 8;
            case TaskAttachmentType::IMAGE: return value.image.shader_as_index ? 4 : 8;
            default: return 0;
            }
        }
    };

    struct UndefinedAttachmentRuntimeData
    {
    };

    struct TaskBufferAttachmentInfo : TaskBufferAttachment
    {
        TaskBufferView view = {};
        TaskBufferView translated_view = {};
        std::span<BufferId const> ids = {};
    };

    struct TaskImageAttachmentInfo : TaskImageAttachment
    {
        TaskImageView view = {};
        TaskImageView translated_view = {};
        ImageLayout layout = {};
        std::span<ImageId const> ids = {};
        std::span<ImageViewId const> view_ids = {};
    };

    struct TaskAttachmentInfo
    {
        TaskAttachmentType type = TaskAttachmentType::UNDEFINED;
        union Value
        {
            UndefinedAttachment undefined;
            TaskBufferAttachmentInfo buffer;
            TaskImageAttachmentInfo image;
        } value = {.undefined = {}};

        constexpr TaskAttachmentInfo() = default;

        constexpr TaskAttachmentInfo(TaskBufferAttachmentInfo const & buffer)
            : type{TaskAttachmentType::BUFFER}, value{.buffer = buffer}
        {
        }

        constexpr TaskAttachmentInfo(TaskImageAttachmentInfo const & image)
            : type{TaskAttachmentType::IMAGE}, value{.image = image}
        {
        }

        constexpr auto name() const -> char const *
        {
            switch (type)
            {
            case TaskAttachmentType::BUFFER: return value.buffer.name;
            case TaskAttachmentType::IMAGE: return value.image.name;
            default: return "undefined";
            }
        }

        constexpr auto shader_array_size() const -> u32
        {
            switch (type)
            {
            case TaskAttachmentType::BUFFER: return value.buffer.shader_array_size * 8;
            case TaskAttachmentType::IMAGE: return value.image.shader_array_size * (value.image.shader_as_index ? 4 : 8);
            default: return 0;
            }
        }

        constexpr auto shader_element_align() const -> u32
        {
            switch (type)
            {
            case TaskAttachmentType::BUFFER: return 8;
            case TaskAttachmentType::IMAGE: return value.image.shader_as_index ? 4 : 8;
            default: return 0;
            }
        }
    };

    using TaskAttachmentInfoVariant = Variant<TaskBufferAttachmentInfo, TaskImageAttachmentInfo>;

    struct DAXA_EXPORT_CXX TaskInterface
    {
        Device & device;
        CommandRecorder & recorder;
        std::span<TaskAttachmentInfo const> attachment_infos = {};
        // optional:
        TransferMemoryPool * allocator = {};
        std::span<std::byte> attachment_shader_blob = {};

        void assign_attachment_shader_blob(std::span<u8> arr) const
        {
            std::memcpy(
                arr.data(),
                attachment_shader_blob.data(),
                attachment_shader_blob.size());
        }

        TaskBufferAttachmentInfo const & get(TaskBufferAttachmentIndex index) const;
        TaskBufferAttachmentInfo const & get(TaskBufferView view) const;
        TaskImageAttachmentInfo const & get(TaskImageAttachmentIndex index) const;
        TaskImageAttachmentInfo const & get(TaskImageView view) const;
        TaskAttachmentInfo const & get(usize index) const;
    };

    using TaskViewVariant = Variant<
        std::pair<daxa::TaskBufferAttachmentIndex, daxa::TaskBufferView>,
        std::pair<daxa::TaskImageAttachmentIndex, daxa::TaskImageView>>;

    inline namespace detail
    {
        template <typename T>
        constexpr auto calculate_attachment_shader_blob_size(std::span<T const> attachments) -> u32
        {
            u32 total = 0;
            for (auto const & attach : attachments)
            {
                auto attach_size = attach.shader_array_size();
                auto const align = attach.shader_element_align();
                // up-align
                if (attach_size != 0)
                {
                    auto align_offset = total % align;
                    if (align_offset != 0)
                    {
                        total += align - align_offset;
                    }
                }
                total += attach_size;
            }
            return total;
        }
    } // namespace detail

    struct ITask
    {
        constexpr virtual ~ITask() {}
        /// TODO(pahrens): optimize:
        constexpr virtual auto attachment_shader_blob_size() const -> u32
        {
            return detail::calculate_attachment_shader_blob_size(attachments());
        };
        constexpr virtual auto attachments() -> std::span<TaskAttachmentInfo> = 0;
        constexpr virtual auto attachments() const -> std::span<TaskAttachmentInfo const> = 0;
        constexpr virtual std::string_view name() const = 0;
        virtual void callback(TaskInterface){};
    };

    template <usize N>
    struct StringLiteral
    {
        constexpr StringLiteral(char const (&str)[N])
        {
            std::copy_n(str, N - 1, value);
        }
        char value[N - 1];
        usize SIZE = N - 1;
    };

    // Used for simpler concept template constraint in add_task.
    struct IPartialTask
    {
    };

    template <usize ATTACHMENT_COUNT>
    struct AttachmentViews
    {
        AttachmentViews(std::array<daxa::TaskViewVariant, ATTACHMENT_COUNT> const & index_view_pairs)
        {
            for (TaskViewVariant const & vari : index_view_pairs)
            {
                if (auto * buf_pair = get_if<std::pair<daxa::TaskBufferAttachmentIndex, daxa::TaskBufferView>>(&vari))
                {
                    views[buf_pair->first.value] = buf_pair->second;
                }
                else
                {
                    auto const & img_pair = get<std::pair<daxa::TaskImageAttachmentIndex, daxa::TaskImageView>>(vari);
                    views[img_pair.first.value] = img_pair.second;
                }
            }
        }
        AttachmentViews() = default;
        std::array<Variant<daxa::TaskBufferView, daxa::TaskImageView>, ATTACHMENT_COUNT> views = {};
    };

    template <usize ATTACHMENT_COUNT, StringLiteral NAME>
    struct PartialTask : IPartialTask
    {
        /// NOTE: Used to add attachments and declate named constant indices to the added attachment.
        static auto add_attachment(TaskBufferAttachment const & attach) -> TaskBufferAttachmentIndex
        {
            attachment_decl_array.at(cur_attach_index) = attach;
            return TaskBufferAttachmentIndex{cur_attach_index++};
        }
        /// NOTE: Used to add attachments and declate named constant indices to the added attachment.
        static auto add_attachment(TaskImageAttachment const & attach) -> TaskImageAttachmentIndex
        {
            attachment_decl_array.at(cur_attach_index) = attach;
            return TaskImageAttachmentIndex{cur_attach_index++};
        }
        static auto name() -> std::string_view { return std::string_view{NAME.value, NAME.SIZE}; }
        static auto attachments() -> std::span<TaskAttachment const>
        {
            return attachment_decl_array;
        }
        static auto attachment(TaskBufferAttachmentIndex index) -> TaskBufferAttachment const &
        {
            return attachment_decl_array[index.value].value.buffer;
        }
        static auto attachment(TaskImageAttachmentIndex index) -> TaskImageAttachment const &
        {
            return attachment_decl_array[index.value].value.image;
        }

        static auto attachment_shader_blob_size() -> u32
        {
            return detail::calculate_attachment_shader_blob_size(attachments());
        };

        static constexpr inline usize ATTACH_COUNT = ATTACHMENT_COUNT;
        static inline std::array<TaskAttachment, ATTACHMENT_COUNT> attachment_decl_array = {};
        using AttachmentViews = daxa::AttachmentViews<ATTACHMENT_COUNT>;

      private:
        static inline u32 cur_attach_index = 0;
    };

    /*
    ⠀⠀⢀⣀⣄⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⠾⠛⠛⠷⣦⡀⠀⠀⠀⠀⠀⠀
    ⢠⣶⠛⠋⠉⡙⢷⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⣿⠐⡡⢂⠢⠈⠻⣦⡀⠀⠀⠀⠀
    ⣾⠃⠠⡀⠥⡐⡙⣧⣰⣤⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢿⡹⡜⢄⠣⢤⣩⣦⣸⣧⠀⠀⠀⠀
    ⣿⡀⢢⠑⠢⣵⡿⠛⠉⠉⠉⣷⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢿⣜⢬⣿⠛⠉⠉⠉⠻⣧⡀⠀⠀
    ⣹⣇⠢⣉⣾⡏⠀⠠⠀⢆⠡⣘⣷⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⡾⠟⠋⢉⠛⢷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⣿⡆⠱⡈⠔⠠⠄⠈⢷⡄⠀
    ⠀⢿⣦⢡⣿⠀⠌⡐⠩⡄⢊⢵⣇⣠⣀⣀⡀⠀⠀⠀⠀⣼⠟⢁⢀⠂⠆⡌⢢⢿⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣤⡀⠀⠀⠀⠀⣾⣅⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⡏⢷⡣⠜⣈⠆⡡⠂⠌⣷⠀
    ⠀⠀⢹⡞⣧⠈⡆⢡⠃⣼⣾⡟⠛⠉⠉⠉⠛⣷⡄⠀⢸⡏⠐⢨⡄⡍⠒⣬⢡⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣴⡟⠁⠀⠀⠀⠀⠑⢻⣶⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣴⣾⡖⣶⣦⠀⠀⠀⠀⠀⠀⠀⢸⡏⡜⣷⢱⢨⡆⢱⠈⡆⣿⠀
    ⠀⠀⠀⠽⣇⠎⡰⣩⡼⡟⠁⠄⡀⠠⠀⠀⠀⠈⢿⡄⡿⢄⢃⠖⡰⣉⠖⣡⡿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣷⡿⠁⣀⣠⣀⣤⣤⣤⣼⣿⣷⣦⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⢯⠋⡀⢀⠀⠉⢿⣆⠀⠀⠀⠀⠀⣸⢗⢡⣿⣂⣖⣨⡱⢊⡔⣿⠀
    ⠀⠀⠀⠀⣯⠒⠥⡾⢇⠰⡉⠔⡠⠃⡌⢐⠡⠀⣼⡟⡓⢌⢒⢪⠑⣌⡾⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣴⣿⣿⣿⣻⠭⠿⠛⠒⠓⠚⠛⠛⠿⣿⣿⣿⣿⣳⡶⢦⣄⣀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⡚⠤⣁⠢⠐⣀⠀⣿⡀⠀⠀⠀⢈⡟⣸⢟⠉⠁⠀⠉⠙⢷⣴⠇⠀
    ⠀⠀⠀⠀⢿⣩⢲⣟⢌⡒⡱⢊⠴⢡⢘⣄⣢⣽⠞⡑⢌⠂⢎⠤⢋⡞⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣶⣿⣿⣿⠿⠋⡀⢀⠠⠀⠄⠠⠂⠄⠄⡠⠀⠄⡈⠉⠛⠛⠛⡙⠺⣭⡗⣦⣄⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢿⣰⠂⡅⢣⠐⡠⢽⡇⠀⠀⢀⡾⡅⣯⢄⠊⠤⢁⠂⠄⠀⠙⣧⡀
    ⠀⠀⠀⠀⠺⣇⢾⢭⢢⠱⣡⠋⣔⣷⠋⡍⠰⢀⠊⠰⢈⠜⡠⢊⣽⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⡶⣿⡿⠛⠛⡉⡁⢄⠂⡔⢠⠂⡅⢊⢡⠘⡐⢌⣠⡑⠢⢐⠡⢊⠔⡡⢂⠅⣂⠙⡳⣎⡟⣶⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⣯⠰⢃⡜⢠⢺⡇⠀⢰⡾⠅⠃⢿⣜⠌⡒⢄⢊⡐⡁⢂⠘⣧
    ⠀⠀⠀⠀⠀⢻⣺⡇⢎⡱⢄⡓⣾⠄⢣⠈⠅⡂⠡⠑⡈⢢⠑⢢⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⡾⣫⠗⡅⣢⣥⣧⢽⠶⠟⣶⢶⣿⠆⡜⡐⠦⠱⢌⠢⡜⢏⣿⠛⣛⠳⢾⣤⡣⡜⣠⠓⡤⢩⢳⡎⡝⡷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡟⡰⢈⠆⡹⣇⣰⠿⡀⢌⠒⠤⠙⢷⣼⡠⢆⡔⢡⠂⠔⣻
    ⠀⠀⠀⠀⠀⠐⢻⣏⠦⣑⢊⠔⣿⠈⢆⡑⠂⡌⢠⠑⡈⠤⡉⢼⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⢻⢣⠞⣣⢵⣾⡿⢋⠃⢆⠬⣹⠗⡬⡑⢎⠴⣉⠎⣕⢪⡑⢎⠲⡸⢯⣅⡚⠤⡘⡙⠿⣶⣍⡒⠧⢎⠼⣑⢣⢏⢷⣆⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡿⡐⠥⠚⡄⡙⠓⠤⡑⢌⡘⠤⡉⣼⢌⣷⠢⠜⢢⠉⢆⣿
    ⠀⠀⠀⠀⠀⠀⠐⣯⣚⠤⡋⡜⢫⠩⢄⠢⡑⡠⢃⠰⡁⢆⠱⣈⡧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣾⠏⣎⢣⣾⣿⡋⢍⡰⢌⡚⣌⣾⢋⠳⡰⣉⢎⠲⣡⠚⡤⠣⡜⣌⢣⠱⣩⠙⠷⣧⠵⡨⠜⡨⠻⣿⣇⠮⣑⢎⢣⠞⣬⡙⣯⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⢻⡌⡱⢃⡜⣨⡕⢢⠑⣢⠘⢤⣹⢏⡜⣠⢣⠙⢦⡙⢦⠇
    ⠀⠀⠀⠀⠀⠀⠀⠽⣎⠖⡱⢌⠥⢊⠖⠓⠒⠿⣮⡔⡡⢎⠰⢂⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣼⡛⣆⡛⣴⣿⡟⠰⣌⠲⡌⢶⢞⡋⢦⡉⠖⣑⣢⣮⣵⣶⣷⣶⣷⣶⣶⣥⣧⣢⡙⢢⡑⢎⡡⡙⠴⣛⠛⡦⢓⢬⠚⣌⡓⢦⡹⢜⡻⣆⠀⠀⠀⠀⠀⠀⠀⠀⢰⣿⢜⡢⢱⣡⡿⠛⠛⢒⠳⢤⢋⠴⣛⠣⡔⢢⢎⡙⢦⣱⠟⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⢻⣝⡰⣉⠖⣡⠚⣈⠁⠄⠈⢻⣶⡨⢡⢃⡿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡾⢳⠍⣦⠱⣊⠏⡽⣉⢆⠳⢌⠣⢆⡙⣤⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣼⡠⢃⠝⡢⢅⠫⡔⡍⢦⠹⢤⡙⢦⠱⣋⡜⡻⣆⠀⠀⠀⠀⠀⠀⠀⠈⢻⣜⠲⣱⣿⠀⠂⡍⠰⣈⠦⡉⢖⡡⢓⡌⢣⠎⣜⣶⠏⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠠⢻⣖⡡⠞⣄⠓⡄⠣⢐⠀⠀⢻⣿⣥⡾⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡾⣍⢧⢫⠔⡫⠴⣉⠖⡡⢎⡱⢊⡱⣼⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣎⡑⢎⡱⡘⡜⢢⠝⣢⡙⣌⢳⡑⢮⠱⣹⣆⠀⠀⠀⠀⠀⠀⠀⠈⢿⡱⣿⣿⠀⢃⠌⡱⢠⢒⡉⢦⡑⢣⡜⢣⣾⠞⠁⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠻⣼⠱⣌⠓⡬⠑⡌⠠⠁⢸⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⢑⠎⡖⣩⢎⡱⢣⠜⡬⡑⢎⠔⣣⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣮⡢⡑⡜⢌⡱⢪⠔⡱⢊⢦⠹⣌⠏⣄⢻⡆⠀⠀⠀⠀⠀⠀⠀⠁⠙⢿⣿⡌⡐⢌⠰⠡⢎⠜⣢⠙⣦⡽⠟⠁⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠻⣦⣝⡰⢩⢌⠱⣈⣾⠏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⠇⣎⠹⡬⣑⠎⣔⠣⣍⠒⡭⢌⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡕⡘⠦⣡⢃⢎⡱⣉⢦⢋⡜⡎⢥⠊⣿⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹⣿⣔⣈⠒⣍⣢⣽⡴⠟⠉⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠙⠛⠚⠛⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⡿⠐⣌⢓⠲⣉⠞⡤⢓⠬⡑⢆⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣎⢒⡡⠎⢦⠱⡌⠦⡍⠖⣭⠒⡌⢸⣇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠙⠛⠋⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢼⡇⢢⠙⡜⢦⢋⡴⢡⠚⡤⢓⣼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡌⢣⠔⡌⢦⠱⢢⠕⡲⣉⠖⣁⠚⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⠁⢢⠹⣌⠳⣌⠲⣡⢋⠴⣹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡐⢎⡜⢢⡙⢆⢫⠱⣌⠳⣀⠂⣿⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠠⣿⠐⢂⠳⣌⠳⣌⠳⣄⢋⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡰⢌⠣⡜⣌⠣⡝⢤⠳⢄⠂⢹⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⠀⢣⡙⣔⠣⡜⠲⡌⢦⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⢊⠵⡘⢤⡓⢬⢣⡙⠢⠌⢸⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⣿⠈⠴⡱⢌⡳⢌⠳⡘⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣎⠲⣉⠦⡙⣆⢣⠚⡅⠊⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⣿⡈⡱⣘⢣⠜⡬⢣⢹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡰⡡⢎⡱⡌⡖⣍⠒⡡⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⡇⡒⣍⠮⡜⢆⢣⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠟⡋⠍⡠⠄⡠⢀⠂⡍⢙⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣧⡑⢮⠰⡱⢜⡢⠍⢤⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣺⣇⣱⢎⢲⣉⠮⢼⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢟⡋⠔⣡⠘⠤⡑⢨⠐⡁⢎⠠⢃⠌⡐⡙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡜⣌⢣⠕⣎⡱⣩⢘⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠩⣷⡐⣏⠦⣃⠞⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⡑⠢⠜⡰⢠⢉⠒⡌⢄⠣⡘⠄⠣⢌⠢⡑⢌⠢⡙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣯⠔⣣⢚⡴⣑⡃⢾⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣷⠸⣜⡰⢻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠋⡴⢉⠜⢢⠑⠢⢌⠒⡌⢢⠑⠤⣉⠲⡈⢆⠱⢌⠢⢡⠃⡽⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⢜⢢⠣⢖⡱⢌⡿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢻⣷⢆⡇⣻⣿⣿⣿⣿⣿⣿⣿⣿⡿⣁⠳⢨⠜⡨⢆⣉⠣⣊⠜⣈⠆⣙⡐⢢⠡⣑⢊⠒⡌⢣⢑⡊⡔⣊⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡎⡖⣹⢊⡵⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣿⢼⡘⣽⣿⣿⣿⣿⣿⣿⣿⢏⠦⣡⢋⠲⢌⡑⠦⢌⡱⡐⠎⡤⠩⢔⠨⡅⢃⠲⢌⡱⢌⡱⢢⠱⡘⢤⣉⠻⣿⣿⣿⣿⣿⣿⣿⣿⡗⣍⠖⣯⣼⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⢺⡱⣚⣿⣿⣿⣿⣿⠟⡕⢎⠲⡡⢎⡱⢊⡜⡘⢆⠲⢡⠓⣌⠓⣌⠓⣌⢃⠳⣈⠲⢌⡒⣡⢣⡙⠦⣌⠣⡍⢿⣿⣿⣿⣿⣿⣿⡹⡰⣋⢿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢿⡟⡠⢇⠼⣻⠿⣟⢣⠟⡸⢜⢣⠣⡜⡠⢇⡸⢣⠜⢣⠇⡛⣄⢛⡀⢟⡀⠟⡤⢃⠻⡄⢣⢄⢣⡘⢇⡄⢧⠛⣤⢘⡿⣿⣿⣿⢟⡣⢣⠇⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠚⣧⡑⡌⠒⡡⠚⢤⣳⡾⣱⠪⣅⠳⢬⠱⣊⠴⣃⠮⡑⢎⡱⢌⠦⣙⢢⡙⡜⡰⣉⠖⣩⠲⡌⢦⡙⠦⡜⢢⠛⡤⢓⡜⣆⠳⡜⢪⡑⢣⠋⣾⢁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⣷⣄⣁⣠⣽⡟⢧⡱⢆⡳⣌⢓⡎⡱⣌⠳⣌⢲⣉⠖⡱⢊⠖⣡⢒⡱⣌⡱⡘⣜⢢⢓⡜⣢⡙⣜⡘⣣⢝⡸⣛⢾⣌⡳⢈⠥⠘⠠⢡⡿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠙⠛⠋⠉⠛⢧⡝⣎⠵⣌⠧⡜⡱⣌⠳⣌⠶⡌⢞⡡⢏⠼⣡⢎⡱⢢⠵⡱⣌⢎⠦⣱⢡⠞⣤⠛⡴⢪⠵⣩⢞⣼⠿⢶⣤⣥⣤⡿⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⠳⣽⡘⢮⡱⢳⣌⠳⣜⢢⡝⣢⢝⡸⢲⢡⠞⣰⠣⡞⡱⣌⢎⢞⡰⢣⠞⣔⡫⣜⢣⣿⠶⠋⠀⠀⠀⠈⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠓⠯⣧⣎⠳⣬⢓⡬⡱⢎⡵⣋⡬⣛⠴⣋⠶⡱⢎⡞⡬⢳⡍⣞⣦⠷⠛⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    ⠄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢈⡙⠓⠻⠶⠽⢮⣶⣥⣷⣭⣾⣥⣯⡵⠯⠼⠗⠛⠋⣉⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
    */

    /// @brief Calculates size of a shader blob.
    /// @param t task head attachment struct.
    /// @return size of shader blob.
    consteval auto get_asb_size(auto const & as) -> usize
    {
        u32 size = 0;
        u32 total_align = 0;
        auto align_up = [](auto value, auto align) -> u32
        {
            if (value == 0)
                return 0;
            if (align == 0)
                return 0;
            return (value + align - 1u) / align * align;
        };
        for (auto const & attachment_decl : as.attachment_decl_array)
        {
            if (attachment_decl.shader_array_size() == 0)
                continue;
            if (attachment_decl.shader_element_align() == 0)
                return 0; // Error case!
            size = align_up(size, attachment_decl.shader_element_align());
            total_align = std::max(total_align, attachment_decl.shader_element_align());
            size += attachment_decl.shader_array_size();
        }
        size = align_up(size, total_align);
        return static_cast<usize>(size);
    }

    /// @brief Calculates alignment of a shader blob.
    /// @param t task head attachment struct.
    /// @return alignment of shader blob.
    consteval auto get_asb_alignment(auto const & as) -> usize
    {
        u32 total_align = 0;
        for (auto const & attachment_decl : as.attachment_decl_array)
        {
            if (attachment_decl.shader_array_size() == 0)
                continue;
            if (attachment_decl.shader_element_align() == 0)
                return 0; // Error case!
            total_align = std::max(total_align, attachment_decl.shader_element_align());
        }
        return static_cast<usize>(total_align);
    }

#define DAXA_DECL_TASK_HEAD_BEGIN(HEAD_NAME, COUNT)                                           \
    namespace HEAD_NAME                                                                       \
    {                                                                                         \
        static inline constexpr char NAME[] = #HEAD_NAME;                                     \
        static inline constexpr daxa::usize ATTACHMENT_COUNT = COUNT;                         \
        struct AttachmentsStruct                                                              \
        {                                                                                     \
            daxa::u32 actually_declared_attachment_count = {};                                \
                                                                                              \
            auto constexpr add_attachment(daxa::TaskBufferAttachment attachment)              \
                -> daxa::TaskBufferAttachmentIndex                                            \
            {                                                                                 \
                attachment_decl_array.at(actually_declared_attachment_count) = attachment;    \
                return daxa::TaskBufferAttachmentIndex{actually_declared_attachment_count++}; \
            }                                                                                 \
            auto constexpr add_attachment(daxa::TaskImageAttachment attachment)               \
                -> daxa::TaskImageAttachmentIndex                                             \
            {                                                                                 \
                attachment_decl_array.at(actually_declared_attachment_count) = attachment;    \
                return daxa::TaskImageAttachmentIndex{actually_declared_attachment_count++};  \
            }                                                                                 \
                                                                                              \
            std::array<daxa::TaskAttachment, ATTACHMENT_COUNT> attachment_decl_array = {};

#define _DAXA_HELPER_TH_BUFFER(NAME, TASK_ACCESS, ...)     \
    daxa::TaskBufferAttachmentIndex const NAME =           \
        add_attachment(daxa::TaskBufferAttachment{         \
            .name = #NAME,                                 \
            .access = daxa::TaskBufferAccess::TASK_ACCESS, \
            __VA_ARGS__});

#define _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, ...)     \
    daxa::TaskImageAttachmentIndex const NAME =           \
        add_attachment(daxa::TaskImageAttachment{         \
            .name = #NAME,                                \
            .access = daxa::TaskImageAccess::TASK_ACCESS, \
            __VA_ARGS__});

#define DAXA_DECL_TASK_HEAD_END                                                     \
    }                                                                               \
    ;                                                                               \
    static inline constexpr auto AT = AttachmentsStruct{};                          \
    struct alignas(daxa::get_asb_alignment(AT)) AttachmentShaderBlob                \
    {                                                                               \
        std::array<daxa::u8, daxa::get_asb_size(AT)> value = {};                    \
    };                                                                              \
    struct Task : public daxa::IPartialTask                                         \
    {                                                                               \
        using AttachmentViews = daxa::AttachmentViews<ATTACHMENT_COUNT>;            \
        static constexpr AttachmentsStruct AT = AttachmentsStruct{};                \
        static constexpr daxa::usize ATTACH_COUNT = ATTACHMENT_COUNT;               \
        static auto name() -> std::string_view { return std::string_view{NAME}; }   \
        static auto attachments() -> std::span<daxa::TaskAttachment const>          \
        {                                                                           \
            return AT.attachment_decl_array;                                        \
        }                                                                           \
        static auto attachment_shader_blob_size() -> daxa::u32                      \
        {                                                                           \
            return sizeof(daxa::get_asb_size(AT));                                  \
        };                                                                          \
    };                                                                              \
    static_assert(ATTACHMENT_COUNT == AT.actually_declared_attachment_count,        \
                  "Declared attachment must match attachment count in Task Head!"); \
    }                                                                               \
    ;

#define DAXA_TH_BLOB(HEAD_NAME, field_name) HEAD_NAME::AttachmentShaderBlob field_name;

#define DAXA_TH_IMAGE(TASK_ACCESS, VIEW_TYPE, NAME) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = 0)
#define DAXA_TH_IMAGE_ID(TASK_ACCESS, VIEW_TYPE, NAME) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = 1)
#define DAXA_TH_IMAGE_INDEX(TASK_ACCESS, VIEW_TYPE, NAME) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = 1, .shader_as_index = true)
#define DAXA_TH_IMAGE_ID_ARRAY(TASK_ACCESS, VIEW_TYPE, NAME, SIZE) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = SIZE)
#define DAXA_TH_IMAGE_ID_MIP_ARRAY(TASK_ACCESS, VIEW_TYPE, NAME, SIZE) _DAXA_HELPER_TH_IMAGE(NAME, TASK_ACCESS, .view_type = daxa::ImageViewType::VIEW_TYPE, .shader_array_size = SIZE, .shader_array_type = daxa::TaskHeadImageArrayType::MIP_LEVELS)
#define DAXA_TH_BUFFER(TASK_ACCESS, NAME) _DAXA_HELPER_TH_BUFFER(NAME, TASK_ACCESS, .shader_array_size = 0)
#define DAXA_TH_BUFFER_ID(TASK_ACCESS, NAME) _DAXA_HELPER_TH_BUFFER(NAME, TASK_ACCESS, .shader_array_size = 1, .shader_as_address = false)
#define DAXA_TH_BUFFER_PTR(TASK_ACCESS, PTR_TYPE, NAME) _DAXA_HELPER_TH_BUFFER(NAME, TASK_ACCESS, .shader_array_size = 1, .shader_as_address = true)
#define DAXA_TH_BUFFER_ID_ARRAY(TASK_ACCESS, NAME, SIZE) _DAXA_HELPER_TH_BUFFER(NAME, TASK_ACCESS, .shader_array_size = SIZE, .shader_as_address = false)
#define DAXA_TH_BUFFER_PTR_ARRAY(TASK_ACCESS, PTR_TYPE, NAME, SIZE) _DAXA_HELPER_TH_BUFFER(NAME, TASK_ACCESS, .shader_array_size = SIZE, .shader_as_address = false)

    template <typename BufFn, typename ImgFn>
    constexpr void for_each(std::span<TaskAttachmentInfo> attachments, BufFn && buf_fn, ImgFn && img_fn)
    {
        for (u32 index = 0; index < attachments.size(); ++index)
        {
            switch (attachments[index].type)
            {
            case TaskAttachmentType::BUFFER: buf_fn(index, attachments[index].value.buffer); break;
            case TaskAttachmentType::IMAGE: img_fn(index, attachments[index].value.image); break;
            default: break;
            }
        }
    }

    template <typename BufFn, typename ImgFn>
    constexpr void for_each(std::span<TaskAttachmentInfo const> attachments, BufFn && buf_fn, ImgFn && img_fn)
    {
        for (u32 index = 0; index < attachments.size(); ++index)
        {
            switch (attachments[index].type)
            {
            case TaskAttachmentType::BUFFER: buf_fn(index, attachments[index].value.buffer); break;
            case TaskAttachmentType::IMAGE: img_fn(index, attachments[index].value.image); break;
            default: break;
            }
        }
    }

    struct TrackedBuffers
    {
        std::span<BufferId const> buffers = {};
        Access latest_access = {};
    };

    struct TaskBufferInfo
    {
        TrackedBuffers initial_buffers = {};
        std::string name = {};
    };

    struct ImplPersistentTaskBuffer;
    struct DAXA_EXPORT_CXX TaskBuffer : ManagedPtr<TaskBuffer, ImplPersistentTaskBuffer *>
    {
        TaskBuffer() = default;
        // TaskBuffer(TaskBuffer const & tb) = default;
        TaskBuffer(TaskBufferInfo const & info);

        operator TaskBufferView() const;

        auto view() const -> TaskBufferView;
        /// THREADSAFETY:
        /// * reference MUST NOT be read after the object is destroyed.
        /// @return reference to info of object.
        auto info() const -> TaskBufferInfo const &;
        auto get_state() const -> TrackedBuffers;

        void set_buffers(TrackedBuffers const & buffers);
        void swap_buffers(TaskBuffer & other);

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };

    struct TrackedImages
    {
        std::span<ImageId const> images = {};
        // optional:
        std::span<ImageSliceState const> latest_slice_states = {};
    };

    struct TaskImageInfo
    {
        TrackedImages initial_images = {};
        bool swapchain_image = {};
        std::string name = {};
    };

    struct ImplPersistentTaskImage;
    struct DAXA_EXPORT_CXX TaskImage : ManagedPtr<TaskImage, ImplPersistentTaskImage *>
    {
        TaskImage() = default;
        // TaskImage(TaskImage const & ti) = default;
        TaskImage(TaskImageInfo const & info);

        operator TaskImageView() const;

        auto view() const -> TaskImageView;
        /// THREADSAFETY:
        /// * reference MUST NOT be read after the object is destroyed.
        /// @return reference to info of object.
        auto info() const -> TaskImageInfo const &;
        auto get_state() const -> TrackedImages;

        void set_images(TrackedImages const & images);
        void swap_images(TaskImage & other);

      protected:
        template <typename T, typename H_T>
        friend struct ManagedPtr;
        static auto inc_refcnt(ImplHandle const * object) -> u64;
        static auto dec_refcnt(ImplHandle const * object) -> u64;
    };

    using AttachmentViewPairVariant = Variant<std::pair<TaskBufferAttachment, TaskBufferView>, std::pair<TaskImageAttachment, TaskImageView>>;

    inline auto attachment_view(TaskBufferAttachmentIndex index, TaskBufferView view) -> TaskViewVariant
    {
        return std::pair<daxa::TaskBufferAttachmentIndex, daxa::TaskBufferView>(index, view);
    }

    inline auto attachment_view(TaskImageAttachmentIndex index, TaskImageView view) -> TaskViewVariant
    {
        return std::pair<daxa::TaskImageAttachmentIndex, daxa::TaskImageView>(index, view);
    }

    inline auto inl_attachment(TaskBufferAccess access, TaskBufferView view) -> TaskAttachmentInfo
    {
        TaskBufferAttachmentInfo buf = {};
        buf.name = "inline attachment";
        buf.access = access;
        buf.shader_array_size = 0;
        buf.shader_as_address = false;
        buf.view = view;
        TaskAttachmentInfo info = {};
        info.type = daxa::TaskAttachmentType::BUFFER;
        info.value.buffer = buf;
        return info;
    }
    inline auto inl_attachment(TaskImageAccess access, TaskImageView view) -> TaskAttachmentInfo
    {
        TaskImageAttachmentInfo img = {};
        img.name = "inline attachment";
        img.access = access;
        img.view_type = daxa::ImageViewType::MAX_ENUM;
        img.shader_array_size = 0;
        img.view = view;
        TaskAttachmentInfo info = {};
        info.value.image = img;
        info.type = daxa::TaskAttachmentType::IMAGE;
        return info;
    }
    inline auto inl_attachment(TaskImageAccess access, ImageViewType view_type, TaskImageView view) -> TaskAttachmentInfo
    {
        TaskImageAttachmentInfo img = {};
        img.name = "inline attachment";
        img.access = access;
        img.view_type = view_type;
        img.shader_array_size = 0;
        img.view = view;
        TaskAttachmentInfo info = {};
        info.type = daxa::TaskAttachmentType::IMAGE;
        info.value.image = img;
        return info;
    }
} // namespace daxa