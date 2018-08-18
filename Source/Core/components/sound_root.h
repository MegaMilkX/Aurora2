#ifndef SOUND_ROOT_H
#define SOUND_ROOT_H

#include "../scene_object.h"

class SoundEmitter;
class SoundListener;
class SoundRoot : public SceneObject::Component
{
    CLONEABLE(SoundRoot)
    RTTR_ENABLE(SceneObject::Component)
public:
    void Update();
    void OnInit();
    
    void AddEmitter(SoundEmitter* emit);
    void SetListener(SoundListener* listener);
private:
    SoundListener* listener;
};

#endif
