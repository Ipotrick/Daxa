#pragma once

#include <mutex>
#include <chrono>
#include <cinttypes>
#include <random>
#include <unordered_map>
#include <iostream>
#include <iomanip>

struct UUID {
	uint64_t high{ 0 };
	uint64_t low{ 0 };
	void invalidate() { high = 0ull; low = 0ull; }
	bool isValid() const { return high != 0 && low != 0; }

	static UUID invalid()
	{
		return UUID();
	}

	bool operator==(const UUID& rhs) const { return this->high == rhs.high && this->low == rhs.low; }
	bool operator<(const UUID& rhs) const { return this->high < rhs.high || this->low < rhs.low; }
};

namespace std {
	template<>
	struct hash<UUID> {
		size_t operator()(const UUID& uuid) const
		{
			return uuid.low;
		}
	};
}

std::ostream& operator<<(std::ostream& os, UUID id);

UUID generateUUID();