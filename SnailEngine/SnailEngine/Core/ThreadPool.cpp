#include "stdafx.h"
#include "ThreadPool.h"

#include <iterator>

namespace Snail
{
ThreadPool::TaskHandle::TaskHandle(int taskId)
    : taskId{taskId}
{ }

bool ThreadPool::TaskHandle::operator<(const TaskHandle& handle) const
{
    return taskId < handle.taskId;
}

ThreadPool::Task::Task(TaskHandle handle, const std::function<void()>& func, bool waitable)
    : handle{handle}
    , task{func}
    , waitable{waitable}
{}

ThreadPool::Task::Task(const std::function<void()>& func, bool waitable)
    : handle{}
    , task{ func }
    , waitable{ waitable }
{}

ThreadPool::TaskHandle ThreadPool::TaskHandle::operator++()
{
    return ++taskId;
}

ThreadPool::TaskHandle ThreadPool::TaskHandle::operator++(int)
{
    return TaskHandle{taskId++};
}

void ThreadPool::Run()
{
    Task task;
    while (!stop)
    {
        {
            // Wait for a task to be available
            std::unique_lock waitLock{queueMutex};
            queueCondition.wait(waitLock, [this] { return !taskQueue.empty() || stop; });
            if (stop)
                return;

            task = taskQueue.front();
            taskQueue.pop();
        }

        task.task();

        if (task.waitable)
        {
            {
                std::unique_lock waitLock{ doneMutex };
                doneTasks.insert(task.handle);
            }
            doneCondition.notify_all();
        }
    }
}

ThreadPool::ThreadPool()
{
    auto threadCount = std::thread::hardware_concurrency();
    threadCount = std::max(threadCount, 1U);

    pool.reserve(threadCount);
    std::generate_n(
        std::back_inserter(pool),
        threadCount,
        [this]
        {
            return std::thread(std::bind(&ThreadPool::Run, this));
        }
    );
}

ThreadPool::~ThreadPool()
{
    stop = true;
    queueCondition.notify_all();
    for (auto& thread : pool)
        thread.join();
}

void ThreadPool::AddTask(const std::function<void()>& func)
{
    {
        std::unique_lock waitLock{ queueMutex };
        taskQueue.push({ func, false });
    }
    queueCondition.notify_one();
}

ThreadPool::TaskHandle ThreadPool::AddWaitableTask(const std::function<void()>& fn)
{
    TaskHandle handle;
    {
        std::unique_lock waitLock{ queueMutex };
        handle = currentHandle++;
        taskQueue.push({handle, fn, true});
    }
    queueCondition.notify_one();
    return handle;
}

void ThreadPool::WaitFor(TaskHandle handle)
{
    std::unique_lock waitLock{doneMutex};
    doneCondition.wait(waitLock, [&] { return doneTasks.contains(handle); });
    doneTasks.erase(handle);
}
}
