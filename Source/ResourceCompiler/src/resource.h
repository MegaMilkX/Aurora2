#ifndef RESOURCE_H
#define RESOURCE_H

#include <string>

class ResData
{
public:
    template<typename T>
    T Build();
};

class Resource
{
public:
    void Init();

    ResData Get(const std::string name);
};

class ResCache
{
public:
    template<typename T>
    T* Get(const std::string& name)
    {

    }
private:

};

class SceneObject
{

};

#endif

int main()
{
    Resource resources;
    resources.Init();

    ResData res = resources.Get("General.Player");

    SceneObject so = res.Build<SceneObject>();

    ResCache cache;
    SceneObject* player = cache.Get<SceneObject>("General.Player");

    return 0;
}