#pragma once

namespace daxa
{
    using ProfileEnterFn = void (*)(char const *);
    using ProfileLeaveFn = void (*)();

    inline ProfileEnterFn profile_enter_fn = nullptr;
    inline ProfileLeaveFn profile_leave_fn = nullptr;

    inline void set_profiling_callbacks(ProfileEnterFn enter, ProfileLeaveFn leave)
    {
        profile_enter_fn = enter;
        profile_leave_fn = leave;
    }

    struct ProfileScope
    {
        explicit ProfileScope(char const * name)
        {
            if (profile_enter_fn != nullptr)
            {
                profile_enter_fn(name);
            }
        }
        ~ProfileScope()
        {
            if (profile_leave_fn != nullptr)
            {
                profile_leave_fn();
            }
        }
        ProfileScope(ProfileScope const &) = delete;
        ProfileScope(ProfileScope &&) = delete;
        auto operator=(ProfileScope const &) -> ProfileScope & = delete;
        auto operator=(ProfileScope &&) -> ProfileScope & = delete;
    };
} // namespace daxa

#define _DAXA_PROFILE_CAT2(a, b) a##b
#define _DAXA_PROFILE_CAT(a, b) _DAXA_PROFILE_CAT2(a, b)
#define DAXA_PROFILE_SCOPE(name) \
    ::daxa::ProfileScope const _DAXA_PROFILE_CAT(_daxa_profile_scope_, __LINE__) { name }
