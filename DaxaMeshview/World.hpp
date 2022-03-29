#pragma once

#include "Daxa.hpp"

struct World {
    World(daxa::DeviceHandle const& device)
        : imageCache{ device }
    {

    }

    
    daxa::ImageCache imageCache;
    daxa::EntityComponentManager ecm = {};
};