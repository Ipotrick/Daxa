#include <daxa/daxa.hpp>

int main()
{
    auto daxa_ctx = daxa::create_context({.enable_validation = false});
    auto device = daxa_ctx.create_device({});

    
}
