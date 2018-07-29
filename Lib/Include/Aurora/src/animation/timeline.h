#ifndef TIMELINE_H
#define TIMELINE_H

#include <algorithm>
#include <vector>

namespace Au{

template<typename T>
struct Keyframe
{
    Keyframe()
    : time(0)
    {}
    Keyframe(float time)
    : time(time)
    {}
    Keyframe(float time, T value)
    : time(time), value(value) 
    {}
    
    float           time;
    T               value;
    
    bool operator<(const Keyframe& other)
    {
        return time < other.time;
    }
    
    Keyframe& operator=(const T& value)
    {
        this->value = value;
        return *this;
    }
};
    
template<typename T>
class InterpolatorFlat
{
public:  
    T operator()(const Keyframe<T>& first, const Keyframe<T>& second, float time)
    {
        return first.value;
    }
};

template<typename T, typename INTERPOLATOR = InterpolatorFlat<T>>
class Timeline
{
public:
    Timeline()
    : max_time(0)
    {}

    void SetKey(float time, T value)
    {
        (*this)[time] = value;
    }
    
    void ClearKey(float time)
    {
        for(unsigned int i = 0; i < timeline.size(); ++i)
        {
            if(timeline[i].time == time)
            {
                if(i == timeline.size() - 1)
                    if(i != 0)
                        max_time = timeline[i - 1].time;
                    else
                        max_time = 0;
                timeline.erase(timeline.begin() + i);
                break;
            }
        }
    }
    
    Keyframe<T>& operator[](float time)
    {
        for(unsigned int i = 0; i < timeline.size(); ++i)
            if(timeline[i].time == time)
                return timeline[i];
        
        timeline.push_back(Keyframe<T>(time));
        std::sort(timeline.begin(), timeline.end());
        if(max_time < time)
            max_time = time;
        
        for(unsigned int i = 0; i < timeline.size(); ++i)
            if(timeline[i].time == time)
                return timeline[i];
        
        return timeline[0];
    }
    
    T Evaluate(float time)
    {
        if(timeline.empty())
            return T();
        
        Keyframe<T> first;
        Keyframe<T> second;
        
        float time_overflow = (int)(time / max_time) * max_time;
        time = time - time_overflow;
        
        for(unsigned int i = timeline.size() - 1; i >= 0; --i)
        {
            if(timeline[i].time <= time)
            {
                first = timeline[i];
                if(i == timeline.size() - 1)
                    second = timeline[0];
                else
                    second = timeline[i+1];
                break;
            }
        }
        
        return INTERPOLATOR()(first, second, time);
    }
private:
    std::vector<Keyframe<T>> timeline;
    float max_time;
};

}

#endif
