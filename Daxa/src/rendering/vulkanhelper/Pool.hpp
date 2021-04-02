#pragma once

#include <vector>
#include <array>
#include <functional>

#include "../Vulkan.hpp"

namespace daxa {
	namespace vkh {
		template<typename T>
		class Pool {
		public:
            Pool() = default;
            Pool(std::function<T(void)> creator, std::function<void(T)> destroyer, std::function<void(T)> resetter) :
                creator{ std::move(creator) }, destroyer{ std::move(destroyer) }, resetter{ std::move(resetter) }
            { }

            Pool(Pool&& other)
            {
                this->creator = std::move(other.creator);
                this->destroyer = std::move(other.destroyer);
                this->resetter = std::move(other.resetter);
                this->pool = std::move(other.pool);
                this->zombies = std::move(other.zombies);
            }

            Pool& operator=(Pool&& other)
            {
                if (&other == this) return *this;
                return *new (this) Pool(std::move(other));
            }

            ~Pool()
            {
                for (auto& el : pool) {
                    destroyer(el);
                }
                for (auto& list : zombies) {
                    for (auto& el : list) {
                        destroyer(el);
                    }
                }
                pool.clear();
            }

            auto flush()
            {
                for (auto& el : zombies[1]) {
                    resetter(el);
                }
                pool.insert(pool.end(), zombies[1].begin(), zombies[1].end());
                zombies[1].clear();
                std::swap(zombies[0], zombies[1]);
            }

            T get()
            {
                if (pool.size() == 0) {
                    pool.push_back(creator());
                }

                auto el = pool.back();
                pool.pop_back();
                zombies[0].push_back(el);
                return el;
            }
		private:
            std::function<T(void)> creator;
            std::function<void(T)> destroyer;
            std::function<void(T)> resetter;
			std::vector<T> pool;
			std::array<std::vector<T>, 2> zombies{ std::vector<T>{}, std::vector<T>{} };
		};
	}
}
