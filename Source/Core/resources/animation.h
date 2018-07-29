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

    bool Build(Resource* r)
    {
        if(!r) return false;
        Resource* r_fps = r->Get("FrameRate");
        Resource* r_tpf = r->Get("FrameTime");
        Resource* r_tracks = r->Get("Track");
        Resource* r_bind_poses = r->Get("Bind");
        if(!r_fps || !r_tpf || !r_tracks || !r_bind_poses) return false;
        FrameRate((float)*(double*)r_fps->Data());
        double timePerFrame = *(double*)r_tpf->Data();
        LOG("FPS: " << *(double*)r_fps->Data());
        LOG("TPF: " << *(double*)r_tpf->Data());

        AnimPose& pose = GetBindPose();

        for(auto kv : r_bind_poses->GetChildren())
        {
            std::string name = kv.first;
            gfxm::mat4 transform = *(gfxm::mat4*)kv.second->Data();
            pose.poses[name].set_transform(transform);
        }

        for(auto kv : r_tracks->GetChildren())
        {
            std::string animName = kv.first;
            double length = *(double*)kv.second->Get("Length")->Data();
            Resource* r_nodes = kv.second->Get("Node");

            AnimTrack* anim = this->operator[](animName);
            anim->Length((float)length);
            anim->Name(animName);

            for(auto node_kv : r_nodes->GetChildren())
            {
                std::string name = node_kv.first;
                Resource* r_pos = node_kv.second->Get("Pos");
                Resource* r_rot = node_kv.second->Get("Rot");
                Resource* r_scl = node_kv.second->Get("Scl");

                AnimNode& animNode = anim->operator[](name);

                struct keyframe
                {
                    float frame;
                    float value;
                };

                std::vector<keyframe> frames_pos_x;
                std::vector<keyframe> frames_pos_y;
                std::vector<keyframe> frames_pos_z;
                std::vector<keyframe> frames_rot_x;
                std::vector<keyframe> frames_rot_y;
                std::vector<keyframe> frames_rot_z;
                std::vector<keyframe> frames_rot_w;
                std::vector<keyframe> frames_scl_x;
                std::vector<keyframe> frames_scl_y;
                std::vector<keyframe> frames_scl_z;

#define FILL_FRAMES(VECTOR, RESOURCE, SUBRESOURCE) { \
if(RESOURCE) { \
Resource* r_ = RESOURCE->Get(SUBRESOURCE); \
if(r_) VECTOR = std::vector<keyframe>( (keyframe*)r_->Data(), (keyframe*)(r_->Data() + r_->DataSize()) ); \
} \
}

                FILL_FRAMES(frames_pos_x, r_pos, "X");
                FILL_FRAMES(frames_pos_y, r_pos, "Y");
                FILL_FRAMES(frames_pos_z, r_pos, "Z");
                
                FILL_FRAMES(frames_rot_x, r_rot, "X");
                FILL_FRAMES(frames_rot_y, r_rot, "Y");
                FILL_FRAMES(frames_rot_z, r_rot, "Z");
                FILL_FRAMES(frames_rot_w, r_rot, "W");

                FILL_FRAMES(frames_scl_x, r_scl, "X");
                FILL_FRAMES(frames_scl_y, r_scl, "Y");
                FILL_FRAMES(frames_scl_z, r_scl, "Z");

#undef FILL_FRAMES
#define FILL_ANIM(from, to) for(auto k : from) to[k.frame] = k.value;
                
                FILL_ANIM(frames_pos_x, animNode.position.x);
                FILL_ANIM(frames_pos_y, animNode.position.y);
                FILL_ANIM(frames_pos_z, animNode.position.z);

                FILL_ANIM(frames_rot_x, animNode.rotation.x);
                FILL_ANIM(frames_rot_y, animNode.rotation.y);
                FILL_ANIM(frames_rot_z, animNode.rotation.z);
                FILL_ANIM(frames_rot_w, animNode.rotation.w);

                FILL_ANIM(frames_scl_x, animNode.scale.x);
                FILL_ANIM(frames_scl_y, animNode.scale.y);
                FILL_ANIM(frames_scl_z, animNode.scale.z);

#undef FILL_ANIM
            }
        }

        return true;
    }
private:
    std::map<std::string, AnimTrack*> anims;
    AnimPose bindPose;
    float fps;
};

#endif
