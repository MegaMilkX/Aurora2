#ifndef ANIMATOR_H
#define ANIMATOR_H

#include "../component.h"
#include <animation.h>

enum AnimationBlendMode {
    ANIM_OVERWRITE,
    ANIM_BLEND,
    ANIM_ADD
};

struct AnimatorLayer {
    std::shared_ptr<Animation> animation;
    AnimationBlendMode blendMode;
    float weight;
};

class Animator : public Component {
    RTTR_ENABLE(Component)
public:
    std::vector<AnimatorLayer> layers;
    
    void Refresh() {
        
    }
};

#endif
