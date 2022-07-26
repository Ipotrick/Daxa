#include <daxa/daxa.hpp>
#include <iostream>

#include <daxa/utils/task_list.hpp>

struct AppContext
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = true,
    });
    daxa::Device device = daxa_ctx.create_default_device();
};

namespace tests
{
    using namespace daxa::types;

    void simplest()
    {
        AppContext app = {};
        auto task_list = daxa::TaskList({.device = app.device, .debug_name = "TaskList task list"});

        // auto task_image = task_list.create_task_iamge({...});
        // auto [task_image1, task_image2] = task_list.split({...});
        // auto finalized_task_image = task_list.merge(task_image1, task_image2);
        // TaskImageId = task_list.create_task_image({...});
    }

    void execution()
    {
        AppContext app = {};
        auto task_list = daxa::TaskList({.device = app.device, .debug_name = "TaskList task list"});

        // This is pointless, but done to show how the task list executes
        task_list.add_task({
            .task = [&](daxa::TaskInterface &)
            {
                std::cout << "Hello, ";
            },
            .debug_name = "task 1",
        });
        task_list.add_task({
            .task = [&](daxa::TaskInterface &)
            {
                std::cout << "World!" << std::endl;
            },
            .debug_name = "task 2",
        });

        task_list.compile();

        task_list.execute();
    }

    void image_upload()
    {
        AppContext app = {};
        auto task_list = daxa::TaskList({.device = app.device, .debug_name = "TaskList task list"});

        auto task_image = task_list.create_task_image({
            .fetch_callback = [](){ return daxa::ImageId{}; },
        });

        auto upload_buffer = task_list.create_task_buffer({
            .fetch_callback = [](){ return daxa::BufferId{}; },
        });

        task_list.add_task({
            .resources = {
                .buffers = {{upload_buffer, daxa::TaskBufferAccess::TRANSFER_READ}},
                .images = {{task_image, daxa::TaskImageAccess::TRANSFER_WRITE}},
            },
            .task = [](daxa::TaskInterface &){},
        });

        task_list.compile();

        task_list.execute();
    }

    void compute()
    {
        AppContext app = {};
        auto task_list = daxa::TaskList({.device = app.device, .debug_name = "TaskList task list"});

        // std::array<f32, 32> data;
        // for (usize i = 0; i < data.size(); ++i)
        //     data[i] = static_cast<f32>(i);

        // // print the data first
        // for (usize i = 0; i < data.size(); ++i)
        //     std::cout << data[i] << ", ";
        // std::cout << std::endl;

        // auto pipeline_compiler = app.device.create_pipeline_compiler({
        //     .root_paths = {"include"}, // for daxa/daxa.hlsl
        //     .debug_name = "TaskList Pipeline Compiler",
        // });

        // struct Push
        // {
        //     daxa::BufferId data_buffer_id;
        // };
        // // clang-format off
        // auto pipeline = pipeline_compiler.create_compute_pipeline({
        //     .shader_info = {
        //         .source = daxa::ShaderCode{.string = R"(
        //             #include "daxa/daxa.hlsl"
        //             struct DataBuffer
        //             {
        //                 float data[32];
        //                 void compute(uint tid)
        //                 {
        //                     data[tid] = data[tid] * 2.0f;
        //                 }
        //             };
        //             DAXA_DEFINE_GET_STRUCTURED_BUFFER(DataBuffer);
        //             struct Push
        //             {
        //                 daxa::BufferId data_buffer_id;
        //             };
        //             [[vk::push_constant]] Push push;
        //             [numthreads(32, 1, 1)] void main(uint tid : SV_DISPATCHTHREADID)
        //             {
        //                 StructuredBuffer<DataBuffer> data_buffer = daxa::get_StructuredBuffer<DataBuffer>(push.data_buffer_id);
        //                 data_buffer.compute(tid);
        //             }
        //         )"},
        //     },
        //     .push_constant_size = sizeof(Push),
        // }).value();
        // // clang-format on

        // auto data_buffer = app.device.create_buffer({.size = sizeof(data)});
        // auto staging_buffer = app.device.create_buffer({
        //     .memory_flags = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
        //     .size = sizeof(data),
        // });

        // // upload data
        // task_list.add_task({
        //     .resources = {
        //         .buffers = {
        //             { staging_buffer, daxa::TaskBufferAccess::TRANSFER_WRITE },
        //         },
        //     },
        //     .task = [&](daxa::TaskInterface &)
        //     {
        //         auto & mapped_data = *app.device.map_memory_as<decltype(data)>(staging_buffer);
        //         mapped_data = data;
        //         app.device.unmap_memory(staging_buffer);
        //         // transfer staging_buffer to data_buffer

        //     },
        // });

        // // run GPU algorithm
        // task_list.add_task({
        //     .task = [&](daxa::TaskInterface & inter)
        //     {
        //         auto cmd_list = inter.get_command_list();
        //         cmd_list.set_pipeline(pipeline);
        //         cmd_list.push_constant(Push{
        //             .data_buffer_id = data_buffer,
        //         });
        //         cmd_list.dispatch(1, 1, 1);
        //     },
        // });

        // // download data
        // task_list.add_task({
        //     .task = [&](daxa::TaskInterface &)
        //     {
        //         // transfer data_buffer to staging_buffer
        //         data = *app.device.map_memory_as<decltype(data)>(staging_buffer);
        //         app.device.unmap_memory(staging_buffer);
        //     },
        // });

        // task_list.compile();
        // task_list.execute();

        // // output the transformed data!
        // for (usize i = 0; i < data.size(); ++i)
        //     std::cout << data[i] << ", ";
        // std::cout << std::endl;

        // app.device.destroy_buffer(staging_buffer);
        // app.device.destroy_buffer(data_buffer);
    }
} // namespace tests

int main()
{
    // tests::simplest();
    tests::image_upload();
    // tests::execution();
}
