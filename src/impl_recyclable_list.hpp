#include "impl_core.hpp"

namespace daxa
{
    struct ImplDevice;

    template <typename RecyclableT>
    concept Zombifiable = requires(RecyclableT r, typename RecyclableT::InfoT r_info)
    {
        r.reset();
        r.initialize(r_info);
    };

    template <Zombifiable RecyclableT>
    struct RecyclableList
    {
#if DAXA_THREADSAFETY
        std::mutex mtx = {};
#endif
        std::vector<std::unique_ptr<RecyclableT>> recyclables = {};

        auto recycle_or_create_new(ManagedWeakPtr device_impl, auto const & info) -> std::unique_ptr<RecyclableT>
        {
            std::unique_ptr<RecyclableT> ret = {};

            {
#if DAXA_THREADSAFETY
                std::unique_lock lock{mtx};
#endif
                if (!recyclables.empty())
                {
                    ret = std::move(recyclables.back());
                    recyclables.pop_back();
                }
            }

            if (!ret)
            {
                ret = std::make_unique<RecyclableT>(device_impl);
            }

            ret->initialize(info);

            return ret;
        }

        void clear()
        {
            recyclables.clear();
        }
    };
} // namespace daxa
