#ifndef BONE_H
#define BONE_H

#include <component.h>
#include <transform.h>

class Bone : public Component {
  CLONEABLE(Bone)
  RTTR_ENABLE(Component)
public:
  
  virtual void OnInit() {}
};
STATIC_RUN(Bone)
{
    rttr::registration::class_<Bone>("Bone")
        .constructor<>()(rttr::policy::ctor::as_raw_ptr);
}

#endif
