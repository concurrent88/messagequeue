#include "MessageQueue.h"

using namespace raj;

void MessageQueue::enqueue(const Job& job, bool front)
{
	{
		std::lock_guard<std::mutex> guard{ _lock };
		if (front)
			_jobs.emplace_front(job);
		else
			_jobs.emplace_back(job);
	}

	_condition.notify_one();
}

void MessageQueue::process()
{
	while(true)
	{
		Job job;
		{
			std::unique_lock<std::mutex> guard{ _lock };
			_condition.wait(guard, [this]() { return !_jobs.empty(); });

			job = _jobs.front();
			_jobs.pop_front();
		}

		if (job && !job())
		{
			return;
		}
	}
}

bool MessageQueue::process_once(std::chrono::seconds timeout)
{
	Job job;
	{
		std::unique_lock<std::mutex> guard{ _lock };
		_condition.wait_for(guard, timeout, [this]() { return !_jobs.empty(); });

		job = _jobs.front();
		_jobs.pop_front();
	}

	if (job)
	{
		return job();
	}
	else
	{
		return false;
	}
}

void MessageQueue::stop(bool drain)
{
	{
		std::lock_guard<std::mutex> guard{ _lock };
		if (drain)
			_jobs.emplace_front([]() { return false; });
		else
			_jobs.emplace_back([]() { return false; });
	}

	_condition.notify_one();
}