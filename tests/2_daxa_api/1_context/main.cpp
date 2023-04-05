#include <daxa/daxa.hpp>
#include <iostream>

namespace tests
{
    void simplest()
    {
        auto daxa_ctx = daxa::create_context({});
    }

    void no_validation()
    {
        auto daxa_ctx = daxa::create_context({
            .enable_validation = false,
        });
    }

    void custom_validation_callback()
    {
        auto daxa_ctx = daxa::create_context({
            .enable_validation = false,
            .validation_callback = [](daxa::MsgSeverity severity, daxa::MsgType type, std::string_view msg)
            {
                std::cout << msg << std::endl;
                // maybe call the callback provided by daxa!
                daxa::default_validation_callback(severity, type, msg);
            },
        });
    }
} // namespace tests

auto main() -> int
{
    tests::simplest();
    tests::no_validation();
    tests::custom_validation_callback();
}
