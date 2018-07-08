#ifndef JOB_H
#define JOB_H

#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <mutex>
#include <cassert>
#include <atomic>

class Job;
typedef void (*job_fn_t)(Job&);
typedef int thread_id_t;
typedef int worker_id_t;

#define JOB_PAYLOAD_SIZE 64
class Job
{
public:
    Job() : task(0), parent(0), dependency(0), affinity(0), open_work_items(0) {}
    Job(job_fn_t task, Job* parent = 0, Job* dependency = 0, worker_id_t affinity = 0)
    : task(task), parent(parent), dependency(dependency), affinity(affinity), open_work_items(1)
    {
        SetParent(parent);
    }

    void Reset()
    {
        if(open_work_items == 0)
        {
            open_work_items++;
            SetParent(parent);
        }
    }

    void SetParent(Job* parent) {
        if(!parent) return;
        this->parent = parent;
        parent->open_work_items++;
    }
    Job* GetParent() {
        return parent;
    }
    Job* GetDependency() {
        return dependency;
    }
    void SetAffinity(worker_id_t worker) {
        this->affinity = worker;
    }
    worker_id_t GetAffinity() { 
        return affinity;
    }
    template<typename T>
    void SetData(const T& data){
        assert(sizeof(T) <= JOB_PAYLOAD_SIZE);
        (*(T*)payload) = data;
    }
    template<typename T>
    T& GetData(){
        assert(sizeof(T) <= JOB_PAYLOAD_SIZE);
        return *(T*)payload;
    }

    job_fn_t GetTask()
    {
        return task;
    }

    bool ReadyToRun() { 
        bool dependency_finished = true;
        if(dependency) dependency_finished = dependency->Finished();
        return open_work_items == 1 && dependency_finished;
    }
    bool Finished() { return open_work_items == 0; }

    void Run() {
        task(*this);
        Finish();
    }

    bool operator<(const Job& other) const {
        return open_work_items < other.open_work_items;
    }

private:
    void Finish()
    {
        open_work_items--;
        if(Finished() && parent)
            parent->open_work_items--;
    }

    job_fn_t task;
    Job* parent;
    Job* dependency;
    worker_id_t affinity;
    std::atomic_size_t open_work_items;
public:
    unsigned char payload[JOB_PAYLOAD_SIZE];
};

class JobItem
{
public:
    JobItem()
    : job(0)
    {}
    JobItem(Job* j)
    : job(j)
    {}
    JobItem(job_fn_t task, Job* parent = 0, Job* dependency = 0, worker_id_t affinity = 0)
    {
        job = new Job(task, parent, dependency, affinity);
    }
    JobItem(JobItem&& other)
    {
        job = other.job;
        other.job = 0;
    }
    ~JobItem()
    {
        if(job) delete job;
    }
    JobItem& operator=(Job* j)
    {
        if(job) delete job;
        job = j;
    }
    operator Job*()
    {
        return job;
    }
    Job* Get() { return job; }
private:
    Job* job;
};

#endif
