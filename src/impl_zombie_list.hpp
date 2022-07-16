#include "impl_core.hpp"

namespace daxa
{
    struct ImplDevice;

    template <typename ZombieT>
    concept Zombifiable = requires(ZombieT z, typename ZombieT::InfoT z_info)
    {
        z.reset();
        z.initialize(z_info);
    };

    template <Zombifiable ZombieT>
    struct ZombieList
    {
        std::mutex mtx = {};
        std::vector<std::shared_ptr<ZombieT>> zombies = {};

        void try_zombiefy(std::shared_ptr<void> & void_potential_zombie)
        {
            if (void_potential_zombie.use_count() == 1)
            {
                std::shared_ptr<ZombieT> potential_zombie = std::static_pointer_cast<ZombieT>(void_potential_zombie);
                potential_zombie->reset();
                std::unique_lock lock{mtx};
                zombies.push_back(potential_zombie);
            }
        }

        auto revive_zombie_or_create_new(std::shared_ptr<ImplDevice> & device_impl, auto const & info) -> std::shared_ptr<ZombieT>
        {
            std::shared_ptr<ZombieT> ret = {};

            {
                std::unique_lock lock{mtx};
                if (!zombies.empty())
                {
                    ret = zombies.back();
                    zombies.pop_back();
                }
            }

            if (!ret)
            {
                ret = std::make_shared<ZombieT>(device_impl);
            }

            ret->initialize(info);

            return ret;
        }

        void clear()
        {
            zombies.clear();
        }
    };
} // namespace daxa
