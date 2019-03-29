// Author: MoonChasing
// ThreadPoll类：简易线程池实现，表示 worker 线程，执行通用任务线程

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>

class ThreadPool
{
public:
    typedef std::function<void()> Task;
    ThreadPool(int threadnum = 0);
    ~ThreadPool();

    void start();
    void stop();
    void addTask(Task task);
    void ThreadFunc();
    int getThreadNum() {return threadnum_;}

private:
    bool started_;
    int threadnum_;
    std::vector<std::thread *> threadVec_;
    std::queue<Task> taskQue_;
    std::mutex mutex_;
    std::condition_variable con_var;
};

#endif