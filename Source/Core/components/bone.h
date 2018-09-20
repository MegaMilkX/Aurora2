#ifndef BONE_H
#define BONE_H

#include <scene_object.h>
#include <transform.h>

class Bone : public SceneObject::Component {
  CLONEABLE(Bone)
  RTTR_ENABLE(SceneObject::Component)
public:
  
  virtual void OnInit() {}
};
STATIC_RUN(Bone)
{
    rttr::registration::class_<Bone>("Bone")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr);
}

#endif
