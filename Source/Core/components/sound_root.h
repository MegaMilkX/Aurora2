#ifndef SOUND_ROOT_H
#define SOUND_ROOT_H

#include "../component.h"

class SoundEmitter;
class SoundListener;
class SoundRoot : public Component
{
    CLONEABLE
    RTTR_ENABLE(Component)
public:
    void Update();
    void OnInit();
    
    void AddEmitter(SoundEmitter* emit);
    void SetListener(SoundListener* listener);
private:
    SoundListener* listener;
};

#endif
