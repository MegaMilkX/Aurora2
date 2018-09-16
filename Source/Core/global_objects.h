#ifndef GLOBAL_OBJECTS_H
#define GLOBAL_OBJECTS_H

#include <scene_serializer.h>

inline SceneSerializer& GlobalSceneSerializer() {
    static SceneSerializer serializer;
    return serializer;
}

#endif
