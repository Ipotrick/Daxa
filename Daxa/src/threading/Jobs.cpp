#include "Jobs.hpp"

namespace daxa{

	void Jobs::initialize()
	{
		std::unique_lock lock(mtx);
		assert(!bWorkerRunning);
		bWorkerRunning = true;
		threads.reserve(threadCount);
		for (u32 id = 0; id < threadCount; ++id) {
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

	void Jobs::wait(Handle handle)
	{
		std::unique_lock lock(mtx);
		DAXA_ASSERT(isHandleValid(handle));
		DAXA_ASSERT(bWorkerRunning);
		auto& batch = jobBatches[handle.index].batch.value();
		batch.bWaitedFor = true;
		DAXA_ASSERT(batch.bOrphaned == false);

		clientCV.wait(lock,
			[&]() -> bool {
				return batch.unfinishedJobs == 0;
			}
		);
		deleteJobBatch(handle.index);
	}

	bool Jobs::finished(Handle handle)
	{
		std::unique_lock lock(mtx);
		DAXA_ASSERT(isHandleValid(handle));
		DAXA_ASSERT(bWorkerRunning);
		DAXA_ASSERT(jobBatches[handle.index].batch.value().bOrphaned == false);
		auto& batch = jobBatches[handle.index].batch.value();

		if (batch.unfinishedJobs == 0) {
			deleteJobBatch(handle.index);
			return true;
		}
		return false;
	}

	void Jobs::orphan(Handle handle)
	{
		std::unique_lock lock(mtx);
		DAXA_ASSERT(isHandleValid(handle));
		DAXA_ASSERT(bWorkerRunning);
		DAXA_ASSERT(!jobBatches[handle.index].batch.value().bOrphaned);
		jobBatches[handle.index].batch.value().bOrphaned = true;
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

	void Jobs::workerFunction(const u32 id)
	{
		std::unique_lock lock(mtx);
		for (;;) {
			workerCV.wait(lock,
				[&]() {
					return !jobQueue.empty() || !bWorkerRunning;
				}
			);
			if (!bWorkerRunning) return;

			auto [handle, job, _] = jobQueue.back();
			jobQueue.pop_back();

			lock.unlock();
			job->execute(id);
			lock.lock();

			auto& batch = jobBatches[handle.index].batch.value();
			batch.unfinishedJobs -= 1;
			if (batch.unfinishedJobs == 0) {
				if (batch.bOrphaned) {
					deleteJobBatch(handle.index);
				}
				else if (batch.bWaitedFor) {
					clientCV.notify_one();
				}
			}
		}
	}
}
