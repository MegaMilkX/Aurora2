#ifndef SOUND_EMITTER_H
#define SOUND_EMITTER_H

#include "../scene_object.h"
#include <component.h>
#include "sound_root.h"
#include <sound_clip.h>

#undef GetObject

class SoundEmitter : public SceneObject::Component
{
    CLONEABLE(SoundEmitter)
    RTTR_ENABLE(SceneObject::Component)
public:
    SoundEmitter()
    : clip(0)
    {}

    void SetClip(const std::string& name)
    {
        clip = asset<SoundClip>::get(name);
        if(!clip)
            return;
        emitter = GameState::GetAudioMixer()->CreateEmitter(clip->GetBuffer());
        emitter->Play(1);
    }

    void OnInit()
    {
        soundRoot = GetObject()->Root()->GetComponent<SoundRoot>();
        soundRoot->AddEmitter(this);
    }
protected:
    SoundRoot* soundRoot;
    asset<SoundClip> clip;
    AudioEmitter* emitter;
};

class SoundListener : public SceneObject::Component
{
    CLONEABLE(SoundListener)
    RTTR_ENABLE(SceneObject::Component)
public:
    void OnInit()
    {
        soundRoot = GetObject()->Root()->GetComponent<SoundRoot>();
        soundRoot->SetListener(this);
    }
protected:
    SoundRoot* soundRoot;
};

#endif
