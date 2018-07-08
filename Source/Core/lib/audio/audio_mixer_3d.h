#ifndef AUDIO_MIXER_3D_H
#define AUDIO_MIXER_3D_H


#include <iostream>
#include <vector>

#include <stdint.h>

#include <xaudio2.h>
#pragma comment(lib, "xaudio2.lib")

#include <util/gfxm.h>

inline size_t FixIndexOverflow(size_t index, size_t size)
{
    return index % size;
}

class AudioBuffer
{
public:
    enum RETCODE
    {
        RET_OK,
        RET_INVALID_PARAMETERS,
        RET_INVALID_BIT_PER_SAMPLE_VALUE
    };

    AudioBuffer(int sampleRate, int bitPerSample, int nChannels)
    : data(0), bytes(0), sampleRate(sampleRate), bitPerSample(bitPerSample), nChannels(nChannels)
    {
        
    }
    ~AudioBuffer()
    {
        if(data)
            free(data);
    }
    int Upload(void* data, size_t len, int srcSampleRate, int srcBitPerSample, int srcNChannels)
    {
        this->bytes = len;
        this->data = malloc(len);
        memcpy(this->data, data, len);
        sampleRate = srcSampleRate;
        bitPerSample = srcBitPerSample;
        nChannels = srcNChannels;

        return RET_OK;
    }
    
    void ConvertSample(char* src, char* dest, int fromBps, int toBps)
    {
        for(int i = 0; i < fromBps && i < toBps; ++i)
        {
            dest[toBps - i - 1] = src[fromBps - i - 1];
        }
    }
    
    size_t Size() { return bytes; }
    
    size_t CopyData(void* data, size_t bytes, size_t cursor, bool circular, float* volMod)
    {
        short* clip = (short*)data;
        
        size_t szAudio1 = this->bytes - (size_t)cursor;
        if(szAudio1 > bytes) szAudio1 = bytes;
        size_t szAudio2 = bytes - szAudio1;
        
        char* clip1 = (char*)this->data + (size_t)cursor;
        char* clip2 = (char*)this->data;
        
        CopyClip(clip1, szAudio1, nChannels, clip, 2, cursor, volMod);
        CopyClip(clip2, szAudio2, nChannels, clip + (szAudio1 / 2), 2, cursor, volMod);
        
        return szAudio2;
    }
    
    void CopyClip(char* clip, size_t szClip, int srcNChannels, short* tgtClip, int tgtNChannels, size_t cursor, float* volMod)
    {
        for(unsigned i = 0; i < szClip / 2 ; i += 2)
        {
            int sample = 0;
            for(int srcChan = 0; srcChan < srcNChannels; ++srcChan)
            {
                sample += ((short*)clip)[i + srcChan];
            }
            sample /= srcNChannels;
            for(int tgtChan = 0; tgtChan < tgtNChannels; ++tgtChan)
            {
                tgtClip[i + tgtChan] += (short)((short)sample * volMod[tgtChan]);
            }
        }
    }
    
    int SampleRate() { return sampleRate; }
private:
    void* data;
    size_t bytes;
    int sampleRate;
    int bitPerSample;
    int nChannels;
};

struct AudioEmitter
{
    AudioEmitter()
    : buffer(0),
    cursor(0),
    stopped(true),
    discardOnStop(false)
    {}
    
    AudioBuffer* buffer = 0;
    size_t cursor = 0;
    gfxm::vec3 position;
    bool looping;
    bool stopped;
    bool discardOnStop;
    
    void Play(bool loop = false)
    {
        looping = loop;
        stopped = false;
        cursor = 0;
    }
    
    void PlayAndDiscard()
    {
        looping = false;
        stopped = false;
        cursor = 0;
        discardOnStop = true;
    }
    
    bool IsStopped() { return stopped; }
    bool IsLooped() { return looping; }
    
    void Mix(char* data,
        size_t tgtBytes,
        int tgtSampleRate,
        int tgtBitPerSample,
        int tgtNChannels, 
        gfxm::vec3& leftEarPos, 
        gfxm::vec3& rightEarPos)
    {
        if(!buffer)
            return;
        if(stopped)
            return;
        
        float leftDist = gfxm::length(leftEarPos - position);
        float rightDist = gfxm::length(rightEarPos - position);
        float volMod[2];
        volMod[0] = (10.0f - leftDist) * 0.1f;
        if(volMod[0] < 0.0f) volMod[0] = 0.0f;
        volMod[0] *= volMod[0];
        volMod[1] = (10.0f - rightDist) * 0.1f;
        if(volMod[1] < 0.0f) volMod[1] = 0.0f;
        volMod[1] *= volMod[1];
        
        size_t overflow = 
            buffer->CopyData(
                data, 
                tgtBytes, 
                cursor,
                looping,
                volMod
            );
        if(!looping && overflow)
            stopped = true;
    }
    
    void Advance(int adv)
    {
        if(!buffer)
            return;
        size_t bufSize = buffer->Size();
        cursor += adv;
        if(cursor >= bufSize)
            cursor = cursor - bufSize;
    }
};

class AudioMixer3D
{
public:
    int Init(int sampleRate, int bitPerSample, int maxSounds = 512);
    void Cleanup();
    
    void EarDistance(float meters) { earDistance = meters; }
    float EarDistance() { return earDistance; }
    
    void SetListenerTransform(float* mat4)
    {
        memcpy(&listenerTransform, mat4, sizeof(listenerTransform));
    }
    
    void Update();
    
    void Play(AudioBuffer* buf, gfxm::vec3& pos = gfxm::vec3())
    {
        AudioEmitter* emit = CreateEmitter(buf, pos);
        emit->PlayAndDiscard();
    }
    
    AudioBuffer* CreateBuffer()
    {
        AudioBuffer* buf = new AudioBuffer(sampleRate, bitPerSample, nChannels);
        buffers.push_back(buf);
        return buf;
    }
    void DestroyBuffer(AudioBuffer* buf)
    {
        for(unsigned i = 0; i < buffers.size(); ++i)
            if(buffers[i] == buf)
            {
                delete buf;
                buffers.erase(buffers.begin() + i);
            }
    }
    
    AudioEmitter* CreateEmitter(AudioBuffer* buf = 0, const gfxm::vec3& pos = gfxm::vec3(0.0f, 0.0f, 0.0f))
    {
        AudioEmitter* emitter = 0;
        for(unsigned i = 0; i < freeEmitterSlots.size(); ++i)
        {
            if(freeEmitterSlots[i])
            {
                freeEmitterSlots[i] = false;
                emitter = &(emitters[i]);
                emitter->buffer = buf;
                emitter->position = pos;
                break;
            }
        }
        return emitter;
    }
    void DestroyEmitter(AudioEmitter* obj)
    {
        if(!obj)
            return;
        size_t begin = (size_t)&emitters[0];
        size_t current = (size_t)obj;
        size_t offset = current - begin;
        size_t index = offset / sizeof(AudioEmitter);
        freeEmitterSlots[index] = true;
    }
    
private:
    IXAudio2SourceVoice* pSourceVoice;

    int maxSounds;
    
    int sampleRate;
    int bitPerSample;
    int nChannels;

    float earDistance = 0.215f;
    std::vector<AudioBuffer*> buffers;
    std::vector<AudioEmitter> emitters;
    std::vector<bool> freeEmitterSlots;

    short buffer[88200];
    UINT64 prevSamplesPlayed = 0;
    size_t prevCursor = 0;
    
    gfxm::mat4 listenerTransform;
};

inline int AudioMixer3D::Init(int sampleRate, int bitPerSample, int maxSounds)
{
    ZeroMemory(buffer, sizeof(buffer));
    this->sampleRate = sampleRate;
    this->bitPerSample = bitPerSample;
    this->nChannels = 2;
    this->maxSounds = maxSounds;
    
    int blockAlign = (bitPerSample * nChannels) / 8;
    WAVEFORMATEX wfx = { 
        WAVE_FORMAT_PCM, 
        (WORD)nChannels, 
        (DWORD)sampleRate, 
        (DWORD)(sampleRate * blockAlign), 
        (WORD)blockAlign, 
        (WORD)bitPerSample, 
        0 
    };
    
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    if(FAILED(hr))
    {
        std::cout << "Failed to init COM: " << hr << std::endl;
        return 1;
    }
    
#if(_WIN32_WINNT < 0x602)
#ifdef _DEBUG
    HMODULE xAudioDll = LoadLibraryExW(L"XAudioD2_7.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
#else
    HMODULE xAudioDll = LoadLibraryExW(L"XAudio2_7.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
#endif
    if(!xAudioDll)
    {
        std::cout << "Failed to find XAudio2.7 dll" << std::endl;
        CoUninitialize();
        return 1;
    }
#endif
    
    UINT32 flags = 0;
#if (_WIN32_WINNT < 0x0602 /*_WIN32_WINNT_WIN8*/) && defined(_DEBUG)
    flags |= XAUDIO2_DEBUG_ENGINE;
#endif

    IXAudio2* pXAudio2;
    hr = XAudio2Create(&pXAudio2, flags);
    if(FAILED(hr))
    {
        std::cout << "Failed to init XAudio2: " << hr << std::endl;
        CoUninitialize();
        return 1;
    }
    
    IXAudio2MasteringVoice* pMasteringVoice = 0;
    if(FAILED(hr = pXAudio2->CreateMasteringVoice(&pMasteringVoice)))
    {
        std::cout << "Failed to create mastering voice: " << hr << std::endl;
        //pXAudio2.Reset();
        CoUninitialize();
        return 1;
    }
    
    if(FAILED(hr = pXAudio2->CreateSourceVoice(&pSourceVoice, &wfx, 0, 1.0f, 0)))
    {
        std::cout << "Failed to create source voice: " << hr << std::endl;
        return 1;
    }
        
    pSourceVoice->Start(0, 0);
        
    XAUDIO2_BUFFER buf = { 0 };
    buf.AudioBytes = sizeof(buffer);
    buf.pAudioData = (BYTE*)buffer;
    buf.LoopCount = XAUDIO2_LOOP_INFINITE;
    
    hr = pSourceVoice->SubmitSourceBuffer(&buf);
    
    pMasteringVoice->SetVolume(1.0f);
    
    emitters.resize(maxSounds);
    freeEmitterSlots.resize(maxSounds, true);
    
    listenerTransform = gfxm::mat4(1.0f);
    
    return 0;
}

inline void AudioMixer3D::Cleanup()
{
    
}

inline void AudioMixer3D::Update()
{
    XAUDIO2_VOICE_STATE state;
    pSourceVoice->GetState(&state);
    size_t cursor = (size_t)state.SamplesPlayed * (bitPerSample / 8) * nChannels;
    size_t cursorDelta = cursor - prevCursor;
    size_t cursorBuffer = cursor % sizeof(buffer);
    if(cursor == prevCursor)
    {
        return;
    }
    
    char tempBuffer[22050];
    ZeroMemory(tempBuffer, sizeof(tempBuffer));
    for(int i = 0; i < maxSounds; ++i)
    {
        if(freeEmitterSlots[i])
            continue;
        
        if(emitters[i].stopped && emitters[i].discardOnStop)
        {
            freeEmitterSlots[i] = true;
            continue;
        }
        
        gfxm::vec4 lpos = listenerTransform * gfxm::vec4(-0.1075f, 0.0f, 0.0f, 1.0f);
        gfxm::vec4 rpos = listenerTransform * gfxm::vec4(0.1075f, 0.0f, 0.0f, 1.0f);
        
        emitters[i].Mix(
            tempBuffer,
            sizeof(tempBuffer),
            sampleRate,
            bitPerSample,
            nChannels,
            gfxm::vec3(lpos.x, lpos.y, lpos.z),
            gfxm::vec3(rpos.x, rpos.y, rpos.z)
        );
        emitters[i].Advance(cursorDelta);
    }
    
    char* buf1 = (char*)buffer + cursorBuffer;
    char* buf2 = (char*)buffer;
    
    size_t szBuf1 = sizeof(buffer) - cursorBuffer;
    if(szBuf1 > sizeof(tempBuffer)) szBuf1 = sizeof(tempBuffer);
    size_t szBuf2 = sizeof(tempBuffer) - szBuf1;
    
    memcpy(buf1, tempBuffer, szBuf1);
    memcpy(buf2, tempBuffer + szBuf1, szBuf2);
    
    prevCursor = cursor;
}


#endif
