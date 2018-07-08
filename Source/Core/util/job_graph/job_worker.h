#ifndef JOB_WORKER_H
#define JOB_WORKER_H

#include "job.h"
#include <util/scoped_timer.h>
#include <lib/concurrentqueue.h>

class JobManager;
class JobWorker
{
public:
    JobWorker() : _running(false), _manager(0) {}
    JobWorker(JobManager* man, worker_id_t worker_id = 0)
    : _running(false), _manager(man), _worker_id(worker_id)
    {

    }
    ~JobWorker()
    {

    }

    void Submit(Job* job) {

    }

    void Run() {
        _running = true;
        while(_running)
            RunOne();
    }
    void Stop() {
        _running = false;
    }

    void Wait(Job* waitFor) {
        while(!waitFor->Finished())
            RunOne();
    }
private:
    void RunOne()
    {
        Job* j = GetJob();
        if(j != 0)
        {
            j->Run();
        }
    }
    Job* GetJob();

    moodycamel::ConcurrentQueue<Job*> _queue;
    worker_id_t _worker_id;
    JobManager* _manager;
    std::atomic<bool> _running;
};

#endif
