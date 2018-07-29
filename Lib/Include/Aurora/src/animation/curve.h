#pragma once

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <math.h>
#include <float.h>

#include "../math/math.h"

namespace Au{

template<typename Ret>
inline void Call(Ret(*fn)(), float* values)
{ (*fn)(); }

template<typename Ret, typename Arg0>
inline void Call(Ret(*fn)(Arg0), float* values)
{ (*fn)(values[0]); }

template<typename Ret, typename Arg0, typename Arg1>
inline void Call(Ret(*fn)(Arg0, Arg1), float* values)
{ (*fn)(values[0], values[1]); }

template<typename Ret, typename Arg0, typename Arg1, typename Arg2>
inline void Call(Ret(*fn)(Arg0, Arg1, Arg2), float* values)
{ (*fn)(values[0], values[1], values[2]); }

template<typename Ret, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
inline void Call(Ret(*fn)(Arg0, Arg1, Arg2, Arg3), float* values)
{ (*fn)(values[0], values[1], values[2], values[3]); }

template<typename Ret, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
inline void Call(Ret(*fn)(Arg0, Arg1, Arg2, Arg3, Arg4), float* values)
{ (*fn)(values[0], values[1], values[2], values[3], values[4]); }

template<typename Ret, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
inline void Call(Ret(*fn)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5), float* values)
{ (*fn)(values[0], values[1], values[2], values[3], values[4], values[5]); }



template<typename Class, typename Ret>
inline void CallMember(Ret(Class::*fn)(), Class* _this, float* values)
{ (_this->*fn)(); }

template<typename Class, typename Ret, typename Arg0>
inline void CallMember(Ret(Class::*fn)(Arg0), Class* _this, float* values)
{ (_this->*fn)(values[0]); }

template<typename Class, typename Ret, typename Arg0, typename Arg1>
inline void CallMember(Ret(Class::*fn)(Arg0, Arg1), Class* _this, float* values)
{ (_this->*fn)(values[0], values[1]); }

template<typename Class, typename Ret, typename Arg0, typename Arg1, typename Arg2>
inline void CallMember(Ret(Class::*fn)(Arg0, Arg1, Arg2), Class* _this, float* values)
{ (_this->*fn)(values[0], values[1], values[2]); }

template<typename Class, typename Ret, typename Arg0, typename Arg1, typename Arg2, typename Arg3>
inline void CallMember(Ret(Class::*fn)(Arg0, Arg1, Arg2, Arg3), Class* _this, float* values)
{ (_this->*fn)(values[0], values[1], values[2], values[3]); }

template<typename Class, typename Ret, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
inline void CallMember(Ret(Class::*fn)(Arg0, Arg1, Arg2, Arg3, Arg4), Class* _this, float* values)
{ (_this->*fn)(values[0], values[1], values[2], values[3], values[4]); }

template<typename Class, typename Ret, typename Arg0, typename Arg1, typename Arg2, typename Arg3, typename Arg4, typename Arg5>
inline void CallMember(Ret(Class::*fn)(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5), Class* _this, float* values)
{ (_this->*fn)(values[0], values[1], values[2], values[3], values[4], values[5]); }

struct IFunctor
{
    virtual ~IFunctor() {}
    virtual void operator()(float* values)
    {
        
    }
};

template<typename Ret, typename... Args>
struct Functor : public IFunctor{
    Functor(Ret(*fn)(Args...)):fn(fn){}
    virtual void operator()(float* values){
        Call<Ret, Args...>(fn, values);
    }
private:
    Ret(*fn)(Args...);
};

template<typename Class, typename Ret, typename... Args>
struct FunctorMember : public IFunctor{
    FunctorMember(Ret(Class::*fn)(Args...), Class* _this):fn(fn), _this(_this) {}
    virtual void operator()(float* values){
        CallMember<Class, Ret, Args...>(fn, _this, values);
    }
private:
    Ret(Class::*fn)(Args...);
    Class* _this;
};

struct Keyframe
{
    Keyframe(): frame(0) {}
    Keyframe(float frame): frame(frame) {}
    void operator=(float value)
    { this->value = value; }
    bool operator<(const Keyframe& other)
    { return frame < other.frame; }
    float frame;
    float value;
};

class Curve
{
public:
    Curve(): length(0.0f), prevTime(0.0f) {}
    Curve(const std::string& name)
    : length(0.0), name(name), delta(0.0f) {}
    ~Curve() {  }
    
    Curve& operator[](const std::string& name)
    {
        for(unsigned i = 0; i < curves.size(); ++i)
        {
            if(curves[i].Name() == name)
                return curves[i];
        }
        
        curves.push_back(Curve(name));
        curveValues.resize(curves.size());
        return curves.back();
    }
    
    Keyframe& operator[](float frame)
    {
        for(unsigned i = 0; i < keyframes.size(); ++i)
        {
            if(fabsf(keyframes[i].frame - frame) < FLT_EPSILON)
            {
                return keyframes[i];
            }
        }
        
        keyframes.push_back(Keyframe(frame));
        std::sort(keyframes.begin(), keyframes.end());
        return operator[](frame);
    }

    float ValueAt(float time)
    {
        if(keyframes.empty())
            return 0.0f;
        Keyframe* k0 = 0, *k1 = 0;
        for(int i = keyframes.size() - 1; i >= 0; --i)
        {
            k0 = &keyframes[i];
            if(i == keyframes.size() - 1)
                k1 = k0;
            else
                k1 = &keyframes[i + 1];
            if(k0->frame <= time)
                break;
        }
        if(k0 == 0)
            return 0.0f;
        if(k0 == k1)
            return k0->value;
        float a = k0->value;
        float b = k1->value;
        float t = (time - k0->frame) / (k1->frame - k0->frame);
        return a + t * (b - a);
    }
    
    float Evaluate(float time)
    {   
        if(curves.empty())
        {
            float prevValue = ValueAt(prevTime);
            value = ValueAt(time);

            if(time < prevTime)
            {
                float d0 = keyframes.back().value - prevValue;
                float d1 = value - keyframes.front().value;
                delta = d0 + d1;
            }
            else
            {
                delta = value - prevValue;
            }
            prevTime = time;
        }
        else
        {
            unsigned v = 0;
            for(unsigned i = 0; i < curves.size(); ++i)
                curveValues[v++] = curves[i].Evaluate(time);
        }
        
        return value;
    }
    
    unsigned CurveCount() { return curves.size(); }
    std::string GetCurveName(unsigned id) 
    {
        return curves[id].Name();
    }
    Curve* GetCurve(unsigned id) 
    { 
        return &curves[id];
    }
    Curve* GetCurve(const std::string& name) 
    {
        for(unsigned i = 0; i < curves.size(); ++i)
        {
            if(curves[i].Name() == name)
                return &curves[i];
        }
        return 0;
    }
    
    void Print()
    {
        std::cout << name << ": ";
        if(curves.empty())
        {
            std::cout << value << std::endl;
            return;
        }

        for(unsigned i = 0; i < curves.size(); ++i)
        {
            curves[i].Print();
        }
    }
    
    void Length(float len) { length = len; }
    float Length() { return length; }
    
    std::string& Name() { return name; }
    
    float value;
    float delta;
private:
    float prevTime;
    std::vector<Keyframe> keyframes;
    std::vector<float> curveValues;
    std::vector<Curve> curves;
    float length;
    std::string name;
};

}