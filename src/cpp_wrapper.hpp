#pragma once

#include "impl_core.hpp"

#include <daxa/c/instance.h>
#include <daxa/instance.hpp>

namespace daxa
{
    struct ImplInstance final : ManagedSharedState
    {
        InstanceInfo info;
        daxa_Instance instance;

        explicit ImplInstance(daxa_Instance a_instance, InstanceInfo a_info);
        virtual ~ImplInstance() override final;
    };
} // namespace daxa
