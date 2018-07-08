#ifndef SOUND_ROOT_H
#define SOUND_ROOT_H

#include "../scene_object.h"
#include <component.h>

class SoundEmitter;
class SoundListener;
class SoundRoot : public SceneObject::Component
{
public:
    void Update();
    void OnInit();
    
    void AddEmitter(SoundEmitter* emit);
    void SetListener(SoundListener* listener);
private:
    SoundListener* listener;
};
COMPONENT(SoundRoot)

#endif
