#ifndef CURVE_H
#define CURVE_H

#include <util/gfxm.h>
#include <vector>

struct keyframe
{
    keyframe(): time(0) {}
    keyframe(float time): time(time) {}
    void operator=(float value)
    { this->value = value; }
    bool operator<(const keyframe& other)
    { return time < other.time; }
    float time;
    float value;
};

class curve
{
public:
    float at(float time, float def = 0.0f)
    {
        if(keyframes.empty())
            return def;
        keyframe* k0 = 0, *k1 = 0;
        for(int i = keyframes.size() - 1; i >= 0; --i)
        {
            k0 = &keyframes[i];
            if(i == keyframes.size() - 1)
                k1 = k0;
            else
                k1 = &keyframes[i + 1];
            if(k0->time <= time)
                break;
        }
        if(k0 == 0)
            return def;
        if(k0 == k1)
            return k0->value;
        float a = k0->value;
        float b = k1->value;
        float t = (time - k0->time) / (k1->time - k0->time);
        return a + t * (b - a);
    }

    float delta(float from, float to)
    {
        if(keyframes.empty())
            return 0.0f;
        float d = 0.0f;
        float prevValue = at(from);
        float value = at(to);

        if(to < from)
        {
            float d0 = keyframes.back().value - prevValue;
            float d1 = value - keyframes.front().value;
            d = d0 + d1;
        }
        else
        {
            d = value - prevValue;
        }
        return d;
    }

    keyframe& operator[](float time)
    {
        for(unsigned i = 0; i < keyframes.size(); ++i)
        {
            if(fabsf(keyframes[i].time - time) < FLT_EPSILON)
            {
                return keyframes[i];
            }
        }
        
        keyframes.push_back(keyframe(time));
        std::sort(keyframes.begin(), keyframes.end());
        return operator[](time);
    }

    bool empty() { return keyframes.empty(); }
private:
    std::vector<keyframe> keyframes;
};

struct curve2
{
    gfxm::vec2 at(float t, const gfxm::vec2& def)
    {
        return gfxm::vec2(x.at(t, def.x), y.at(t, def.y));
    }
    gfxm::vec2 delta(float from, float to)
    {
        return gfxm::vec2(
            x.delta(from, to), 
            y.delta(from, to)
        );
    }
    bool empty() { return x.empty() || y.empty(); }
    curve x, y;
};

struct curve3
{
    gfxm::vec3 at(float t, const gfxm::vec3& def)
    {
        return gfxm::vec3(
            x.at(t, def.x), 
            y.at(t, def.y), 
            z.at(t, def.z));
    }
    gfxm::vec3 delta(float from, float to)
    {
        return gfxm::vec3(
            x.delta(from, to),
            y.delta(from, to),
            z.delta(from, to)
        );
    }
    bool empty() { return x.empty() || y.empty() || x.empty(); }
    curve x, y, z;
};

struct curve4
{
    gfxm::vec4 at(float t, const gfxm::vec4& def)
    {
        return gfxm::vec4(
            x.at(t, def.x), 
            y.at(t, def.y), 
            z.at(t, def.z), 
            w.at(t, def.w));
    }
    gfxm::vec4 delta(float from, float to)
    {
        return gfxm::vec4(
            x.delta(from, to),
            y.delta(from, to),
            z.delta(from, to),
            w.delta(from, to)
        );
    }
    bool empty() { return x.empty() || y.empty() || x.empty() || w.empty(); }
    curve x, y, z, w;
};

struct curveq
{
public:
    gfxm::quat at(float t, const gfxm::quat& def)
    {
        return gfxm::normalize(
            gfxm::quat(
                x.at(t, def.x), 
                y.at(t, def.y), 
                z.at(t, def.z), 
                w.at(t, def.w)
            )
        );
    }
    bool empty() { return x.empty() || y.empty() || x.empty() || w.empty(); }
    curve x, y, z, w;
};

#endif
