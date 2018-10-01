#ifndef FBX_SCENE_2_H
#define FBX_SCENE_2_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <fstream>
#include <functional>

#include "fbx_node.h"
#include "fbx_object_container.h"

#include "fbx_log.h"

#include "fbx_properties.h"
#include "fbx_model.h"
#include "fbx_pose.h"
#include "fbx_deformer_skin.h"
#include "fbx_deformer_cluster.h"
#include "fbx_deformer_blend_shape.h"
#include "fbx_animation_stack.h"
#include "fbx_animation_layer.h"
#include "fbx_animation_curve_node.h"
#include "fbx_animation_curve.h"
#include "fbx_geometry.h"
#include "fbx_texture.h"
#include "fbx_material.h"
#include "fbx_node_attribute.h"

namespace Fbx {

typedef uint8_t Byte;
typedef uint32_t Word;
typedef uint64_t DoubleWord;

class Scene {
public:
    bool ReadMem(const char* data, size_t size);
    bool ReadFile(const std::string& filename);
private:
    ObjectContainer objectContainer;

    void Finalize(Node& node);
};

}

#endif
