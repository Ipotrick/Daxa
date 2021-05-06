#include "UUID.hpp"

std::ostream& operator<<(std::ostream& os, UUID id)
{
	return os << std::setw(20) << std::setfill('0') << id.high << "::" << std::setw(20) << std::setfill('0') << id.low;
}

UUID generateUUID()
{
	static bool initialized{ false };
	static std::mt19937_64 randomGenerator;
	if (!initialized) {
		randomGenerator = std::mt19937_64(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	}

	UUID newUUID;
	newUUID.high = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	do {
		newUUID.low = randomGenerator();
	} 
	while (newUUID.low == 0);
	return newUUID;
}