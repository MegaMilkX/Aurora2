#ifndef JOB_POOL_H
#define JOB_POOL_H

#include "job.h"
#include <util/critical_section.h>

class JobPool
{
public:
    JobPool(size_t nReserve)
    : _next_job(0), _jobs{nReserve}
    {
    }

    Job* CreateJob(job_fn_t task, Job* parent = 0, Job* dependency = 0, worker_id_t affinity = 0)
    {
        Job* j = Acquire();
        new (j) Job(task, parent, dependency, affinity);
        return j;
    }
    void Free()
    {
        _next_job = 0;
    }
private:
    Job* Acquire()
    {
        assert(_next_job < _jobs.size());
        return &_jobs[_next_job++];
    }
    std::atomic_size_t _next_job;
    std::vector<Job> _jobs;
};

#endif
