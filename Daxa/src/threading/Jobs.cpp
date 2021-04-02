#include "Jobs.hpp"

namespace daxa{

	void Jobs::initialize()
	{
		std::unique_lock lock(mtx);
		assert(!bWorkerRunning);
		bWorkerRunning = true;
		threads.reserve(threadCount);
		for (u32 id = 1; id < threadCount+1; ++id) {
			threads.push_back(std::thread(workerFunction, id));
		}
	}

	void Jobs::cleanup()
	{
		std::unique_lock lock(mtx);
		assert(bWorkerRunning); 
		bWorkerRunning = false;
		lock.unlock();
		workerCV.notify_all();
		for (auto& worker : threads) {
			worker.join();
		}
		threads.clear();
		jobQueue.clear();
		jobBatches.clear();
		freeList.clear();
	}

	void Jobs::yield(u32 minpriority, u32 yieldCount)
	{
		std::unique_lock lock(mtx);
		for (i32 i = 0; i < yieldCount; i++) {
			if (jobQueue.empty()) return;

			auto& [handle, job, jobPrio] = jobQueue.back();

			if (jobPrio < minpriority) return;

			jobQueue.pop_back();

			processJob(lock, handle, job);
		}
	}

	bool Jobs::isFinished(Handle handle)
	{
		std::unique_lock lock(mtx);
		DAXA_ASSERT(isHandleValid(handle));
		DAXA_ASSERT(bWorkerRunning);
		DAXA_ASSERT(jobBatches[handle.index].batch.value().bOrphaned == false);
		return unlockedIsFinished(handle);
	}

	void Jobs::orphan(Handle handle)
	{
		std::unique_lock lock(mtx);
		DAXA_ASSERT(isHandleValid(handle));
		DAXA_ASSERT(bWorkerRunning);
		DAXA_ASSERT(!jobBatches[handle.index].batch.value().bOrphaned);
		jobBatches[handle.index].batch.value().bOrphaned = true;
	}

	uz Jobs::maxWorkerIndex() { return threadCount; }

	bool Jobs::unlockedIsFinished(Handle handle)
	{
		auto& batch = jobBatches[handle.index].batch.value();

		if (batch.unfinishedJobs == 0) {
			deleteJobBatch(handle.index);
			return true;
		}
		return false;
	}

	void Jobs::deleteJobBatch(u32 index)
	{
		auto& batch = jobBatches[index].batch.value();
		batch.destructor(batch.mem);
		delete batch.mem;
		jobBatches[index].batch.reset();
		jobBatches[index].version += 1;
	}

	bool Jobs::isHandleValid(Handle handle)
	{
		return handle.index < jobBatches.size() && jobBatches[handle.index].batch.has_value() && jobBatches[handle.index].version == handle.version;
	}

	void Jobs::workerFunction(u32 workerIndex)
	{
		WORKER_INDEX = workerIndex;
		std::unique_lock lock(mtx);

		const auto waitingCondition =
			[]() -> bool {
				return !jobQueue.empty() || !bWorkerRunning;
			};

		for (;;) {
			workerCV.wait(lock, waitingCondition);
			if (!bWorkerRunning) return;

			auto [handle, job, _] = jobQueue.back();
			jobQueue.pop_back();

			processJob(lock, handle, job);
		}
	}

	void Jobs::wait(Handle handle)
	{
		std::unique_lock lock(mtx);
		DAXA_ASSERT(isHandleValid(handle));
		DAXA_ASSERT(bWorkerRunning);
		auto& batch = jobBatches[handle.index].batch.value();
		batch.bWaitedFor = true;
		DAXA_ASSERT(batch.bOrphaned == false);
		lock.unlock();

		waitingWorkerFunction(handle);
	}

	void Jobs::waitingWorkerFunction(Handle awaitedJob)
	{
		std::unique_lock lock(mtx);

		const auto waitingCondition =
			[=]() -> bool {
				return unlockedIsFinished(awaitedJob) || !jobQueue.empty();
			};

		for (;;) {
			clientCV.wait(lock, waitingCondition);
			if (!isHandleValid(awaitedJob))  return;

			auto [handle, job, _] = jobQueue.back();
			jobQueue.pop_back();

			processJob(lock, handle, job);
		}
	}

	void Jobs::processJob(std::unique_lock<std::mutex>& lock, Handle handle, IJob* job)
	{
		assert(lock.owns_lock());
		lock.unlock();
		job->execute();
		lock.lock();

		auto& batch = jobBatches[handle.index].batch.value();
		batch.unfinishedJobs -= 1;
		if (batch.unfinishedJobs == 0) {
			if (batch.bOrphaned) {
				deleteJobBatch(handle.index);
			}
			else if (batch.bWaitedFor) {
				clientCV.notify_all();
			}
		}
	}
}
