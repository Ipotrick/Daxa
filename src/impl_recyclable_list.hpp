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
#if defined(DAXA_ENABLE_THREADSAFETY)
        std::mutex mtx = {};
#endif
        std::vector<std::shared_ptr<RecyclableT>> recyclables = {};

        auto recycle_or_create_new(std::shared_ptr<ImplDevice> & device_impl, auto const & info) -> std::shared_ptr<RecyclableT>
        {
            std::shared_ptr<RecyclableT> ret = {};

            {
#if defined(DAXA_ENABLE_THREADSAFETY)
                std::unique_lock lock{mtx};
#endif
                if (!recyclables.empty())
                {
                    ret = recyclables.back();
                    recyclables.pop_back();
                }
            }

            if (!ret)
            {
                ret = std::make_shared<RecyclableT>(device_impl);
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
