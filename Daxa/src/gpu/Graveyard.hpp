#pragma once

#include "../DaxaCore.hpp"

#include <memory>
#include <mutex>
#include <vector>

namespace daxa {
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