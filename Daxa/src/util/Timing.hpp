#pragma once

#include "../DaxaCore.hpp"

#include <chrono>
#include <iostream>

namespace daxa {
	class StopWatch {
	public:
		void start()
		{
			startTimePoint = std::chrono::high_resolution_clock::now();
			bCounting = true;
		}

		u64 stop()
		{
			if (bCounting) {
				auto endTimePoint = std::chrono::high_resolution_clock::now();
				mics += std::chrono::duration_cast<std::chrono::microseconds>(endTimePoint - startTimePoint).count();
				bCounting = false;
			}
			return mics;
		}

		void clear()
		{
			mics = 0;
		}

		u64 getMics() const
		{
			return mics;
		}

		f32 getSecs() const
		{
			return f32(mics) * 0.000001f;
		}
	private:
		bool bCounting{ false };
		std::chrono::time_point<std::chrono::high_resolution_clock> startTimePoint;
		u64 mics{ 0 };
	};

	class RaiiWatch {
	public:
		RaiiWatch(u64* ret) : ret{ ret }
		{
			start();
		}
		~RaiiWatch()
		{
			stop();
		}

		void start()
		{
			startTimePoint = std::chrono::high_resolution_clock::now();
			bCounting = true;
		}

		u64 stop()
		{
			if (bCounting) {
				auto endTimePoint = std::chrono::high_resolution_clock::now();
				*ret += std::chrono::duration_cast<std::chrono::microseconds>(endTimePoint - startTimePoint).count();
				bCounting = false;
			}
			return *ret;
		}
	private:
		bool bCounting{ false };
		std::chrono::time_point<std::chrono::high_resolution_clock> startTimePoint;
		u64* ret{ nullptr };
	};

	class LogWatch 	{
	public:
		LogWatch(std::string_view message) : message{ message }
		{
			start();
		}
		~LogWatch()
		{
			stop();
		}

		void start()
		{
			startTimePoint = std::chrono::high_resolution_clock::now();
			bCounting = true;
		}

		u64 stop()
		{
			if (bCounting) {
				auto endTimePoint = std::chrono::high_resolution_clock::now();
				mics += std::chrono::duration_cast<std::chrono::microseconds>(endTimePoint - startTimePoint).count();
				bCounting = false;
				std::cout << message << " time taken: " << mics << "mics " << f32(mics) * 0.000001f << "secs" << std::endl;
			}
			return mics;
		}

	private:
		bool bCounting{ false };
		std::chrono::time_point<std::chrono::high_resolution_clock> startTimePoint;
		u64 mics{ 0 };
		std::string_view message;
	};
}
