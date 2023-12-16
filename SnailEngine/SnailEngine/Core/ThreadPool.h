#pragma once
#include <mutex>
#include <queue>
#include <set>
#include <thread>

namespace Snail
{
class ThreadPool
{
public:
    class TaskHandle
    {
        friend ThreadPool;

        int taskId;

        TaskHandle() = default;
        TaskHandle(int taskId);

        TaskHandle operator++();
        TaskHandle operator++(int);
    public:
        bool operator<(const TaskHandle& handle) const;
    };
private:

    struct Task
    {
        TaskHandle handle{};
        std::function<void()> task{};
        bool waitable = false;

        Task() = default;
        Task(TaskHandle, const std::function<void()>&, bool);
        Task(const std::function<void()>&, bool);
    };

    std::mutex queueMutex;
    std::condition_variable queueCondition;

    std::mutex doneMutex;
    std::condition_variable doneCondition;

    std::atomic_bool stop = false;

    TaskHandle currentHandle{0};

    std::queue<Task> taskQueue;

    // Done tasks is only cleared when someone waits for the task
    std::set<TaskHandle> doneTasks;
    std::vector<std::thread> pool;

    void Run();
public:

    ThreadPool();
    ~ThreadPool();
    void AddTask(const std::function<void()>& func);
    TaskHandle AddWaitableTask(const std::function<void()>& func);
    void WaitFor(TaskHandle handle);
};
}
