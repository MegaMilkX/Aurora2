#ifndef TASK_GRAPH_H
#define TASK_GRAPH_H

#include <vector>
#include <iostream>
#include <type_traits>

namespace task_graph
{
    
typedef int typeindex;
template<typename T>
struct TypeInfo{
    static typeindex Index()    {
        static typeindex id = _NewId();
        return id;
    }
private:
};

template<typename T>
typeindex GetTypeIndex(T value){
    return TypeInfo<T>::Index();
}

inline typeindex _NewId(){
    static typeindex id;
    return ++id;
}

template<typename T>
class task_data_storage
{
public:
    static T& Get() { return data; }
private:
    static T data;
};
template<typename T>
T task_data_storage<T>::data;

class task_wrap_base
{
public:
    task_wrap_base()
    : first_call(true) {}
    virtual ~task_wrap_base() {}
    virtual void run() = 0;
    bool operator==(const task_wrap_base& other) const
    {
        if(func_type == other.func_type)
            return compare_same_type_wrapper(&other);
        else
            return false;
    }
    bool does_run_once() { return once; }
    void reset_once_flag() { first_call = true; }
    void print()
    {
        std::cout << "Inputs:";
        for(typeindex type : inputs)
        {
            std::cout << " " << type;
        }
        std::cout << std::endl;
        std::cout << "Outputs:";
        for(typeindex type : outputs)
        {
            std::cout << " " << type;
        }
        std::cout << std::endl;
    }
    typeindex get_func_type() { return func_type; }
    std::vector<typeindex>& get_inputs() { return inputs; }
    std::vector<typeindex>& get_outputs() { return outputs; }
protected:
    bool once;
    bool first_call;
    typeindex func_type;
    std::vector<typeindex> inputs;
    std::vector<typeindex> outputs;
    
    virtual bool compare_same_type_wrapper(const task_wrap_base* other) const = 0;
};

template<typename Arg1, typename Arg2 = void, typename Arg3 = void>
class task_wrap : public task_wrap_base
{
public:
    task_wrap(void(*f)(Arg1, Arg2, Arg3), bool once = false)
    : func(f)
    {
        this->once = once;
        func_type = TypeInfo<decltype(f)>::Index();
        typeindex a1 = TypeInfo<std::remove_cv<typename std::remove_reference<Arg1>::type>::type>::Index();
        typeindex a2 = TypeInfo<std::remove_cv<typename std::remove_reference<Arg2>::type>::type>::Index();
        typeindex a3 = TypeInfo<std::remove_cv<typename std::remove_reference<Arg3>::type>::type>::Index();
        
        std::is_const<typename std::remove_reference<Arg1>::type>::value ?
            inputs.push_back(a1) : outputs.push_back(a1);
        std::is_const<typename std::remove_reference<Arg2>::type>::value ?
            inputs.push_back(a2) : outputs.push_back(a2);
        std::is_const<typename std::remove_reference<Arg3>::type>::value ?
            inputs.push_back(a3) : outputs.push_back(a3);
    }
    
    virtual void run()
    {
        if(once && !first_call)
            return;
        Arg1& a1 = task_data_storage<std::remove_cv<typename std::remove_reference<Arg1>::type>::type>::Get();
        Arg2& a2 = task_data_storage<std::remove_cv<typename std::remove_reference<Arg2>::type>::type>::Get();
        Arg3& a3 = task_data_storage<std::remove_cv<typename std::remove_reference<Arg3>::type>::type>::Get();
        func(a1, a2, a3);
        first_call = false;
    }
private:
    void(*func)(Arg1, Arg2, Arg3);
    
    virtual bool compare_same_type_wrapper(const task_wrap_base* other) const
    {
        auto o = (const task_wrap<Arg1, Arg2, Arg3>*)other;
        return func == o->func;
    }
};

template<typename Arg1, typename Arg2>
class task_wrap<Arg1, Arg2, void> : public task_wrap_base
{
public:
    task_wrap(void(*f)(Arg1, Arg2), bool once = false)
    : func(f)
    {
        this->once = once;
        func_type = TypeInfo<decltype(f)>::Index();
        typeindex a1 = TypeInfo<std::remove_cv<typename std::remove_reference<Arg1>::type>::type>::Index();
        typeindex a2 = TypeInfo<std::remove_cv<typename std::remove_reference<Arg2>::type>::type>::Index();
        std::is_const<typename std::remove_reference<Arg1>::type>::value ?
            inputs.push_back(a1) : outputs.push_back(a1);
        std::is_const<typename std::remove_reference<Arg2>::type>::value ?
            inputs.push_back(a2) : outputs.push_back(a2);
    }
    
    virtual void run()
    {
        if(once && !first_call)
            return;
        Arg1& a1 = task_data_storage<std::remove_cv<typename std::remove_reference<Arg1>::type>::type>::Get();
        Arg2& a2 = task_data_storage<std::remove_cv<typename std::remove_reference<Arg2>::type>::type>::Get();
        func(a1, a2);
        first_call = false;
    }
    
private:
    void (*func)(Arg1, Arg2);
    
    virtual bool compare_same_type_wrapper(const task_wrap_base* other) const
    {
        auto o = (const task_wrap<Arg1, Arg2, void>*)other;
        return func == o->func;
    }
};

template<typename Arg1>
class task_wrap<Arg1, void, void> : public task_wrap_base
{
public:
    task_wrap(void(*f)(Arg1), bool once = false)
    : func(f)
    {
        this->once = once;
        func_type = TypeInfo<decltype(f)>::Index();
        typeindex a1 = TypeInfo<std::remove_cv<typename std::remove_reference<Arg1>::type>::type>::Index();
        std::is_const<typename std::remove_reference<Arg1>::type>::value ?
            inputs.push_back(a1) : outputs.push_back(a1);
    }
    
    virtual void run()
    {
        if(once && !first_call)
            return;
        Arg1& a1 = task_data_storage<std::remove_cv<typename std::remove_reference<Arg1>::type>::type>::Get();
        func(a1);
        first_call = false;
    }
    
private:
    void (*func)(Arg1);
    
    virtual bool compare_same_type_wrapper(const task_wrap_base* other) const
    {
        auto o = (const task_wrap<Arg1, void>*)other;
        return func == o->func;
    }
};

template<typename... Args>
task_wrap_base* once(void(*fn)(Args... args))
{
    return new task_wrap<Args...>(fn, true);
}    

class graph
{
public:
    graph()
    {}
    ~graph()
    {
        clear_tasks();
    }
    
    template<typename... Args>
    void operator+=(void(*fn)(Args... args)){
        task_wrap_base* t = new task_wrap<Args...>(fn);
        *this += t;        
    }
    
    template<typename... Args>
    void reset_once_flag(void(*fn)(Args... args)){
        for(unsigned i = 0; i < tasks.size(); ++i)
            if(task_wrap<Args...>(fn) == *tasks[i])
                tasks[i]->reset_once_flag();
    }
    
    void operator+=(task_wrap_base* t){
        if(exists(t))
            return;
        tasks.push_back(t);
        sort();
    }
    
    template<typename T>
    void set_data(const T& d)
    {
        T& data = task_data_storage<T>::Get();
        data = d;
    }

    template<typename T>
    T& get_data()
    {
        return task_data_storage<T>::Get();
    }
    
    void sort()
    {
        std::vector<task_wrap_base*> tmp_tasks = tasks;
        tasks.clear();
        
        int sorted_tasks_count;
        do
        {
            sorted_tasks_count = 0;
            for(unsigned i = 0; i < tmp_tasks.size(); ++i)
            {
                task_wrap_base* task = tmp_tasks[i];
                if(!task)
                    continue;
                tmp_tasks[i] = 0;
                if(is_any_input_connected(task, tmp_tasks))
                {
                    tmp_tasks[i] = task;
                }
                else
                {
                    tasks.push_back(task);
                    sorted_tasks_count++;
                }
            }
        }while(sorted_tasks_count != 0);
        for(unsigned i = 0; i < tmp_tasks.size(); ++i)
        {
            if(tmp_tasks[i] != 0)
            {
                std::cout << "Task graph is not acyclic" << std::endl;
                break;
            }
        }
    }
    
    void run()
    {
        for(unsigned i = 0; i < tasks.size(); ++i)
        {
            tasks[i]->run();
        }
    }
private:
    void clear_tasks()
    {
        for(task_wrap_base* t : tasks)
            if(t) delete t;
        tasks.clear();
    }
    
    bool exists(task_wrap_base* t)
    {
        for(unsigned i = 0; i < tasks.size(); ++i)
            if(*t == *tasks[i])
                return true;
        return false;
    }
    bool is_any_input_connected(task_wrap_base* task, const std::vector<task_wrap_base*> other)
    {
        std::vector<typeindex>& inputs = task->get_inputs();
        for(typeindex in : inputs)
        {
            for(unsigned i = 0; i < other.size(); ++i)
            {
                if(!other[i])
                    continue;
                std::vector<typeindex>& outputs = other[i]->get_outputs();
                for(typeindex out : outputs)
                {
                    if(in == out)
                        return true;
                }
            }
        }
        return false;
    }

    std::vector<task_wrap_base*> tasks;
};

}

#endif
