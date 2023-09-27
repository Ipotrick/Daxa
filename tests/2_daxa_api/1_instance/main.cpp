#include <daxa/daxa.hpp>
#include <iostream>

namespace tests
{
    void simplest()
    {
        auto instance = daxa::create_instance({});
    }
} // namespace tests

auto main() -> int
{
    tests::simplest();
}
