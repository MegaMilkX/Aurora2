#ifndef ANIMATION_H
#define ANIMATION_H

#include <util/gfxm.h>

#include <map>
#include <asset.h>
#include <aurora/media/fbx.h>
#include <aurora/curve.h>
#include <fstream>
#include <util/animation/curve.h>
#include <util/scoped_timer.h>

struct AnimNode
{
    std::string name;
    curve3 position;
    curveq rotation;
    curve3 scale;
};

struct AnimPose
{
    std::map<std::string, gfxm::transform> poses;
};

class AnimTrack
{
public:
    class Cursor
    {
    public:
        Cursor() : track(0) {}
        Cursor(AnimTrack* t) 
        : track(t), cursor(0), prevCursor(0) 
        {}
        float GetCursor() { return cursor; }
        int Valid() { return track == 0 ? 0 : 1; }
        int EndReached(float secondsBeforeEnd = 0.0f)
        {
            if(!track) return 0;
            if(track->Looping()) return 0;
            return cursor + secondsBeforeEnd >= track->Length() ? 1 : 0;
        }
        void operator+=(float t)
        {
            Advance(t);
        }
        void SetFrame(float f)
        {
            if(!track) return;
            cursor = f;
        }
        void Advance(float t)
        {
            if(!track) return;
            prevCursor = cursor;
            cursor += t * track->FrameRate();
            float len = track->Length() - 1.0f;
            if(track->Looping())
            {
                if(cursor > len)
                {
                    if(len == 0.0f) cursor = 0.0f;
                    else cursor -= len;
                }
            }
        }
        AnimPose* GetPose(AnimPose* bind)
        {
            return GetPoseAt(cursor, bind);
        }
        AnimPose* GetPoseAt(float frame, AnimPose* bind) 
        {
            if(!track)
                return &pose;
            for(auto& kv : track->GetNodes())
            {
                gfxm::transform& bp = bind->poses[kv.first];
                gfxm::transform t;
                if(!kv.second.position.empty()) 
                    t.position(
                        kv.second.position.at(
                            frame, 
                            bp.position()
                        )
                    );
                gfxm::quat brot = bp.rotation();
                if(!kv.second.rotation.empty()) 
                    t.rotation(
                        kv.second.rotation.at(
                            frame, 
                            gfxm::vec4(brot.x, brot.y, brot.z, brot.w)
                        )
                    );
                if(!kv.second.scale.empty()) 
                    t.scale(
                        kv.second.scale.at(
                            frame, 
                            bp.scale()
                        )
                    );
                pose.poses[kv.first] = t;
            }
            AnimNode& rmn = track->GetRootMotionNode();
            posDeltaRootMotion = rmn.position.delta(prevCursor, frame);
            gfxm::quat q0 = rmn.rotation.at(prevCursor, gfxm::vec4(0, 0, 0, 1));
            gfxm::quat q1 = rmn.rotation.at(frame, gfxm::vec4(0, 0, 0, 1));

            if(frame < prevCursor)
            {
                gfxm::quat d0 = gfxm::quat(rmn.rotation.at(track->Length() - 1.0f, gfxm::vec4(0, 0, 0, 1))) * gfxm::inverse(q0);
                gfxm::quat d1 = q1 * gfxm::inverse(gfxm::quat(rmn.rotation.at(0.0f, gfxm::vec4(0,0,0,1))));
                rotDeltaRootMotion = d1 * d0;
            }
            else
            {
                rotDeltaRootMotion = q1 * gfxm::inverse(q0);
            }

            return &pose;
        }
        gfxm::vec3& GetRootMotionDeltaPosition() { return posDeltaRootMotion; }
        gfxm::quat& GetRootMotionDeltaRotation() { return rotDeltaRootMotion; }
    private:
        float cursor;
        float prevCursor;
        AnimTrack* track;
        AnimPose pose;
        gfxm::vec3 posDeltaRootMotion;
        gfxm::quat rotDeltaRootMotion;
    };

    AnimTrack(float frameRate)
    : fps(frameRate)
    {}

    Cursor GetCursor() { return Cursor(this); }

    void Looping(bool val) { looping = val; }
    bool Looping() { return looping; }
    
    void SetRootMotionSource(const std::string& nodeName)
    {
        std::map<std::string, AnimNode>::iterator it =
            animNodes.find(nodeName);
        if(it == animNodes.end())
        {
            std::cout << "SetRootMotionSource: Node '" << nodeName << "' not found" << std::endl;
            return;
        }
        animNodes[rootMotionNode.name] = rootMotionNode;
        rootMotionNode = animNodes[nodeName];
        animNodes.erase(nodeName);
    }

    std::string Name() { return name; }
    void Name(const std::string& name) { this->name = name; }
    void Length(float l) { length = l; }
    float Length() { return length; }
    void FrameRate(float fps) { this->fps = fps; }
    float FrameRate() { return fps; }
    AnimNode& operator[](const std::string& node)
    {
        AnimNode& n = animNodes[node];
        n.name = node;
        return n;
    }
    std::map<std::string, AnimNode>& GetNodes() { return animNodes; }
    AnimNode& GetRootMotionNode() { return rootMotionNode; }
private:
    std::string name;
    float length;
    float fps;
    bool looping = true;
    std::map<std::string, AnimNode> animNodes;
    AnimNode rootMotionNode;
};

class Animation
{
public:
    size_t Count() { return anims.size(); }
    AnimTrack* operator[](const std::string& anim)
    {
        if(anims.count(anim) == 0)
            anims[anim] = new AnimTrack(fps);
        return anims[anim];
    }
    std::map<std::string, AnimTrack*>& GetTracks() { return anims; }
    AnimPose& GetBindPose() { return bindPose; }
    void FrameRate(float fps) 
    { 
        this->fps = fps;
        for(auto& kv : anims)
            kv.second->FrameRate(fps);
    }
    float FrameRate() { return fps; }
    void SetRootMotionSource(const std::string& name)
    {
        for(auto& kv : anims)
            kv.second->SetRootMotionSource(name);
    }
private:
    std::map<std::string, AnimTrack*> anims;
    AnimPose bindPose;
    float fps;
};

template<>
inline bool LoadAsset<Animation, FBX>(Animation* animSet, const std::string& filename)
{
    ScopedTimer timer("LoadAsset<Animation, FBX> '" + filename + "'");

    bool result = false;
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if(!file.is_open())
        return result;
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer((unsigned int)size);
    if(file.read(buffer.data(), size))
    {
        result = true;
        Au::Media::FBX::Reader fbxReader;
        fbxReader.ReadMemory(buffer.data(), buffer.size());
        //fbxReader.DumpFile(filename);
        fbxReader.ConvertCoordSys(Au::Media::FBX::OPENGL);
        
        std::vector<Au::Media::FBX::AnimationStack>& stacks =
            fbxReader.GetAnimationStacks();
        double fps = fbxReader.GetFrameRate();
        double timePerFrame = Au::Media::FBX::TimeSecond / fps;
        
        animSet->FrameRate((float)fps);
        AnimPose& pose = animSet->GetBindPose();
        for(unsigned i = 0; i < fbxReader.ModelCount(); ++i)
        {
            Au::Media::FBX::Model* fbxModel = 
                fbxReader.GetModel(i);
            pose.poses[fbxModel->name].set_transform(*(gfxm::mat4*)&fbxModel->transform);
        }

        for(unsigned i = 0; i < stacks.size(); ++i)
        {
            double length = stacks[i].GetLength() / timePerFrame;
            std::string animName = stacks[i].GetName();
            {
                // TODO: Check if fbx is made in blender, only then cut by first pipe symbol
                size_t pipe_pos = animName.find_first_of("|");
                if(pipe_pos != std::string::npos)
                {
                    animName = animName.substr(pipe_pos + 1);
                }
            }
            
            AnimTrack* anim = animSet->operator[](animName);
            anim->Length((float)length);
            anim->Name(animName);

            //std::cout << "AnimStack " << animName << " len: " << length << std::endl;
            
            std::vector<Au::Media::FBX::SceneNode> nodes = stacks[i].GetAnimatedNodes();
            for(unsigned j = 0; j < nodes.size(); ++j)
            {
                if(!stacks[i].HasPositionCurve(nodes[i]) &&
                    !stacks[i].HasRotationCurve(nodes[i]) &&
                    !stacks[i].HasScaleCurve(nodes[i]))
                {
                    continue;
                }
                std::string nodeName = nodes[j].Name();
                AnimNode& animNode = anim->operator[](nodeName);
                float frame = 0.0f;
                //std::cout << "  CurveNode " << nodeName << std::endl;
                for(double t = 0.0f; t < length * timePerFrame; t += timePerFrame)
                {
                    gfxm::vec3 pos = 
                        *(gfxm::vec3*)&stacks[i].EvaluatePosition(nodes[j], (int64_t)t);
                    animNode.position.x[frame] = pos.x;
                    animNode.position.y[frame] = pos.y;
                    animNode.position.z[frame] = pos.z;

                    gfxm::quat rot = 
                        *(gfxm::quat*)&stacks[i].EvaluateRotation(nodes[j], (int64_t)t);
                    animNode.rotation.x[frame] = rot.x;
                    animNode.rotation.y[frame] = rot.y;
                    animNode.rotation.z[frame] = rot.z;
                    animNode.rotation.w[frame] = rot.w;
                    
                    gfxm::vec3 scale = 
                        *(gfxm::vec3*)&stacks[i].EvaluateScale(nodes[j], (int64_t)t);
                    animNode.scale.x[frame] = scale.x;
                    animNode.scale.y[frame] = scale.y;
                    animNode.scale.z[frame] = scale.z;
                    
                    frame += 1.0f;
                }
            }
        }
    }
    
    file.close();
    
    return result;
}

#endif
