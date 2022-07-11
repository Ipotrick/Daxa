#include "impl_core.hpp"

namespace daxa 
{
    Handle::Handle(std::shared_ptr<void> impl) : impl{ std::move(impl) } {}
}
