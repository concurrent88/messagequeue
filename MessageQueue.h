#pragma once

#include <deque>
#include <mutex>
#include <functional>
#include <condition_variable>

namespace raj
{
	using Job = std::function<bool()>;

	class MessageQueue
	{
	public:
		void enqueue(const Job& job, bool front = false);
		void process();
		bool process_once(std::chrono::seconds timeout = std::chrono::seconds(60));

		void stop(bool drain = false);

	private:
		std::deque<Job> _jobs;

		std::mutex _lock;
		std::condition_variable _condition;
	};
}