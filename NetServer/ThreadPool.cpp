// Author: MoonChasing

#include <deque>
#include <unistd.h>
#include "ThreadPool.h"

ThreadPool::ThreadPool(int threadnum)
    : started_(false),
      threadnum_(threadnum),
      threadVec_(),
      taskQue_(),
      mutex_(),
      con_var()

{
}

ThreadPool::~ThreadPool()
{
    std::cout << "~ThreadPool " << std::this_thread::get_id() << std::endl;
    stop();
    for (int i = 0; i < threadnum_; i++)
        threadVec_[i]->join();
    for (int i = 0; i < threadnum_; i++)
        delete threadVec_[i];
    threadVec_.clear();
}

void ThreadPool::start()
{
    if (threadnum_ > 0)
    {
        started_ = true;
        for (int i = 0; i < threadnum_; i++)
        {
            std::thread *pThread = new std::thread(&ThreadPool::ThreadFunc, this);
            threadVec_.push_back(pThread);
        }
    }
}

void ThreadPool::stop()
{
    started_ = false;
    con_var.notify_all();
}

void ThreadPool::addTask(Task task)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        taskQue_.push(task);
    }
    con_var.notify_one();
}

void ThreadPool::ThreadFunc()
{
    std::thread::id tid = std::this_thread::get_id();
    std::stringstream ss;
    ss << tid;
    std::cout << "Worker thread is running: " << tid << std::endl;
    Task task;
    while (started_)
    {
        task = nullptr;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            while (taskQue_.empty() && started_)
                con_var.wait(lock);
            if (!started_)
                break;

            task = taskQue_.front();
            taskQue_.pop();
        }
        if(task)
            task();
    }
}