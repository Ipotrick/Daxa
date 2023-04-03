#pragma once

#include <0_common/window.hpp>
#include <iostream>
#include <thread>

#include <daxa/utils/pipeline_manager.hpp>
#include <daxa/utils/task_list.hpp>

#define APPNAME "Daxa API Sample TaskList"
#define APPNAME_PREFIX(x) ("[" APPNAME "] " x)

using namespace daxa::types;

#include "shaders/shared.inl"

#include <daxa/utils/imgui.hpp>
#include <imgui_impl_glfw.h>

struct AppContext
{
    daxa::Context daxa_ctx = daxa::create_context({
        .enable_validation = false,
    });
    daxa::Device device = daxa_ctx.create_device({
        .name = APPNAME_PREFIX("device"),
    });
};
