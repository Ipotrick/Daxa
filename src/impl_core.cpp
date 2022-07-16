#include "impl_core.hpp"

namespace daxa
{
    Handle::Handle(std::shared_ptr<void> a_impl) : impl{std::move(a_impl)} {}
} // namespace daxa
