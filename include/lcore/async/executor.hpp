/**
 * @file executor.hpp
 * @author liyanes@outlook.com
 * @brief Task executor
 * @version 0.1
 * @date 2024-08-31
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "base.hpp"
#include "lcore/container.hpp"
#include "task.hpp"
#include <thread>

LCORE_ASYNC_NAMESPACE_BEGIN

class Executor: public AbstractClass {
    virtual void Schedule(Task<void>&& task) = 0;
    virtual void Run() = 0;
    virtual void Stop() = 0;
};

template <typename TaskType = Task<void>>
class DefaultExecutor: public Executor {
private:
    List<TaskType> tasks;
    bool started = false;
public:
    void Schedule(TaskType&& task) {
        tasks.push_back(std::move(task));
    }
    void Run(){
        started = true;
        while (started && !tasks.empty()){
            bool allsuspend = true;
            for (auto iter = tasks.begin(); iter != tasks.end();){
                auto &task = *iter;
                if (task.done()){
                    allsuspend = false;
                    iter = tasks.erase(iter);
                }else{
                    task.resume();
                    iter++;
                }
            }
            if (allsuspend){
                std::this_thread::yield();
            }
        }
    }
    void Stop(){
        started = false;
    }
};

LCORE_ASYNC_NAMESPACE_END
