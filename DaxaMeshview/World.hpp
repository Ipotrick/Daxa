#pragma once

#include "Daxa.hpp"

struct World {
    World(daxa::gpu::DeviceHandle const& device)
        : imageCache{ device }
    {

    }

    
    daxa::ImageCache imageCache;
    daxa::EntityComponentManager ecm = {};
};