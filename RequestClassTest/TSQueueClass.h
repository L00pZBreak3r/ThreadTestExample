#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>

// Thread-safe queue
template <typename T>
class TSQueue
{
private:
	// Underlying queue
	std::queue<T> m_queue;

	// mutex for thread synchronization
	std::mutex m_mutex;

	// Condition variable for signaling
	std::condition_variable m_cond;

	bool mLockEmpty;

public:
	TSQueue(bool aLockEmpty) : mLockEmpty(aLockEmpty) {}
	TSQueue(const TSQueue<T>&) = delete;
	TSQueue(TSQueue<T>&&) = delete;
	TSQueue<T>& operator=(const TSQueue<T>&) = delete;

	// Pushes an element to the queue
	void push(T item)
	{

		// Acquire lock
		std::unique_lock<std::mutex> lock(m_mutex);

		// Add item
		m_queue.push(item);

		// Notify one thread that
		// is waiting
		m_cond.notify_one();
	}

	// Pops an element off the queue
	bool pop(T* aOut)
	{

		// acquire lock
		std::unique_lock<std::mutex> lock(m_mutex);

		// wait until queue is not empty
		if (mLockEmpty)
			m_cond.wait(lock,
					[this]() { return !m_queue.empty(); });

		// retrieve item
		bool aResult = !m_queue.empty();
		if (aResult)
		{
			T item = m_queue.front();
			m_queue.pop();
			if (aOut)
				*aOut = item;
		}

		// return item
		return aResult;
	}
};
