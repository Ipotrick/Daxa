#pragma once

#include <any>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <optional>
#include <memory>
#include <algorithm>

#include "../DaxaCore.hpp"

namespace daxa {
	/**
	 * abstact Interface class for jobs.
	 */
	class IJob {
	public:
		virtual void execute(const uint32_t threadId) = 0;
	};

	template<typename T>
	void destructorOf(void* el)
	{
		reinterpret_cast<T*>(el)->~T();
	}

	template<typename T>
	/**
	 * Concept for a job class that derives the IJob interface.
	 */
	concept CJob = std::is_base_of_v<IJob, T>;

	class Jobs {
	public:

		struct Handle {
			u32 index{ 0xFFFFFFFF };
			u32 version{ 0xFFFFFFFF };
		};

		static void initialize();

		static void cleanup();

		template<CJob T>
		static Handle schedule(T&& job, f32 priority = 0.0f)
		{
			std::unique_lock lock(mtx);
			Handle handle;
			if (freeList.empty()) {
				jobBatches.push_back(JobBatchSlot{ JobBatch{}, 0 });
				handle.index = static_cast<u32>(jobBatches.size()) - 1;
				handle.version = 0;
			}
			else {
				handle.index = freeList.back();
				freeList.pop_back();
				handle.version = jobBatches[handle.index].version;
				jobBatches[handle.index].batch.emplace(JobBatch{});
			}
			JobBatch& jg = jobBatches[handle.index].batch.value();
			jg.unfinishedJobs = 1;
			jg.mem = new T(std::move(job));
			jg.destructor = destructorOf<T>;
			auto place = std::upper_bound(
				jobQueue.begin(), 
				jobQueue.end(), 
				QueuedJob{ .priority = priority }, 
				[](const QueuedJob& a, const QueuedJob& b) { return a.priority < b.priority; }
			);
			jobQueue.insert(place, { handle, reinterpret_cast<T*>(jg.mem), priority });
			workerCV.notify_one();
			return handle;
		}

		template<CJob T>
		static Handle schedule(std::vector<T>&& jobs, f32 priority = 0.0f)
		{
			std::unique_lock lock(mtx);
			Handle handle;
			if (freeList.empty()) {
				jobBatches.push_back(JobBatchSlot{ JobBatch{}, 0 });
				handle.index = static_cast<u32>(jobBatches.size()) - 1;
				handle.version = 0;
			}
			else {
				handle.index = freeList.back();
				freeList.pop_back();
				handle.version = jobBatches[handle.index].version;
				jobBatches[handle.index].batch.emplace(JobBatch{});
			}
			JobBatch& jg = jobBatches[handle.index].batch.value();
			jg.unfinishedJobs = jobs.size();
			jg.mem = new std::vector<T>(std::move(jobs));
			jg.destructor = destructorOf<T>;

			static std::vector<QueuedJob> tempQueuedJobsBuffer;
			tempQueuedJobsBuffer.clear();
			tempQueuedJobsBuffer.reserve(jobs.size());
			for (auto& job : *reinterpret_cast<std::vector<T>*>(jg.mem)) {
				tempQueuedJobsBuffer.push_back({ handle, &job, priority });
			}
			auto insertionPlace = std::upper_bound(
				jobQueue.begin(), 
				jobQueue.end(), 
				QueuedJob{.priority = priority}, 
				[](const QueuedJob& a, const QueuedJob& b) { return a.priority < b.priority; }
			);
			jobQueue.insert(insertionPlace, tempQueuedJobsBuffer.begin(), tempQueuedJobsBuffer.end());

			workerCV.notify_all();
			return handle;
		}

		static void wait(Handle handle);

		static bool finished(Handle handle);

		static void orphan(Handle handle);

	private:

		static void deleteJobBatch(u32 index);

		static bool isHandleValid(Handle handle);

		static void workerFunction(const u32 workerId);

		inline static bool bWorkerRunning{ false };
		inline static const size_t threadCount{ std::max(std::thread::hardware_concurrency() - 1, 1u) };
		inline static std::vector<std::thread> threads;
		inline static std::mutex mtx;
		inline static std::condition_variable workerCV;
		inline static std::condition_variable clientCV;

		struct JobBatch {
			void* mem{ nullptr };
			void (*destructor)(void*) { nullptr };

			u64 unfinishedJobs{ 0 };
			bool bOrphaned{ false };
			bool bWaitedFor{ false };
		};

		struct JobBatchSlot {
			std::optional<JobBatch> batch;
			u32 version{ 0 };
		};

		struct QueuedJob {
			Handle handle;
			IJob* job{ nullptr };
			f32 priority{ 0.0f };
		};

		inline static std::deque<QueuedJob> jobQueue;
		inline static std::vector<JobBatchSlot> jobBatches;
		inline static std::vector<u32> freeList;
	};
}
