#ifndef FRAME_GRAPH_V2_H
#define FRAME_GRAPH_V2_H

#include <util/job_graph/job_manager.h>
#include <util/job_graph/job_pool.h>

enum JOB_AFFINITY
{
    AFFINITY_DEFAULT = 1,
    AFFINITY_MAIN_THREAD = 2
};

class FrameGraph
{
public:
    FrameGraph()
    : jobManager(std::thread::hardware_concurrency() - 2, AFFINITY_DEFAULT),
    jobWorkerForeground(&jobManager, AFFINITY_MAIN_THREAD),
    jobPool(64 * 1000)
    {
        Reset();
    }
    
    Job* Add(job_fn_t task, Job* parent = 0, Job* dependency = 0, worker_id_t affinity = 0)
    {
        if(parent == 0) parent = job_frameEnd;
        Job* job = jobPool.CreateJob(
            task,
            parent,
            dependency,
            affinity
        );
        jobs.emplace_back(job);
        return job;
    }

    void Submit(Job* job)
    {
        jobManager.Submit(job);
    }

    void Run()
    {
        for(auto j : jobs)
        {
            jobManager.Submit(j);
        }
        jobManager.Submit(job_frameEnd);
        jobWorkerForeground.Wait(job_frameEnd);
        jobPool.Free();
        jobs.clear();
        Reset();
    }

    Job* FrameEnd() { return job_frameEnd; }
private:
    void Reset()
    {
        job_frameEnd = jobPool.CreateJob([](Job& job){});
    }

    std::vector<Job*> jobs;

    JobPool jobPool;
    JobManager jobManager;
    JobWorker jobWorkerForeground;
    Job* job_frameEnd;
};

#endif
