#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <mutex>

namespace daxa {
    namespace gpu {

		class GraveyardRessource {
		public:
			virtual ~GraveyardRessource() {}
		};

		struct ZombieList {
			std::vector<std::shared_ptr<GraveyardRessource>> zombies = {};
		};

        struct Graveyard {
            std::mutex mtx = {};
            std::vector<std::shared_ptr<ZombieList>> activeZombieLists = {};
        };
    }
}