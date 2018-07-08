#ifndef JOB_MANAGER_H
#define JOB_MANAGER_H

#include "job_worker.h"

class JobManager
{
public:
    JobManager(size_t nWorkers, int default_worker_id = 0)
    {
        for(size_t i = 0; i < nWorkers; ++i)
        {
            _workers.emplace_back(
                new JobWorker(this, default_worker_id)
            );
            _threads.emplace_back([this, i](){
                _workers[i]->Run();
            });
        }
    }
    ~JobManager()
    {
        for(auto& w : _workers)
        {
            w->Stop();
        }
        for(auto& t : _threads)
        {
            t.join();
        }
    }
    JobManager& Submit(Job* j)
    {
        _queue.enqueue(j);
        return *this;
    }
    JobManager& SubmitAndReset(Job* j)
    {
        j->Reset();
        _queue.enqueue(j);
        return *this;
    }
    void Wait(Job* j){}

    Job* GetJob(worker_id_t worker_id)
    {
        Job* j = 0;
        if(!_queue.try_dequeue(j))
            return 0;
        if(!j) return 0;
        if(!j->ReadyToRun())
        {
            _queue.enqueue(j);
            return 0;
        }
        if((j->GetAffinity() != 0 && j->GetAffinity() != worker_id))
        {
            _queue.enqueue(j);
            return 0;
        }
        return j;
    }
private:
    moodycamel::ConcurrentQueue<Job*> _queue;
    std::vector<JobWorker*> _workers;
    std::vector<std::thread> _threads;
};

inline Job* JobWorker::GetJob()
{
    return _manager->GetJob(_worker_id);
}

#endif
