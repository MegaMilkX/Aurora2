#ifndef AU_FBXANIMSTACK_H
#define AU_FBXANIMSTACK_H

#include <algorithm>

#include "fbxnode.h"

#include "fbxutil.h"

#include "fbxscenenode.h"

namespace Au{
namespace Media{
namespace FBX{

const int64_t TimeSecond = 46186158000;
    
enum TIME_MODE
{
    FRAMES_DEFAULT,
    FRAMES_120,
    FRAMES_100,
    FRAMES_60,
    FRAMES_50,
    FRAMES_48,
    FRAMES_30,
    FRAMES_30_DROP,
    FRAMES_NTSC_DROP,
    FRAMES_NTSC_FULL,
    FRAMES_PAL,
    FRAMES_CINEMA,
    FRAMES_1000m,
    FRAMES_CINEMA_ND,
    FRAMES_CUSTOM,
    FRAMES_96,
    FRAMES_72,
    FRAMES_59dot94,
    FRAMES_MODE_COUNT
};
    
struct Keyframe
{
    Keyframe(int64_t frame, float value)
    : frame(frame), value(value) {}
    bool operator<(const Keyframe& other)
    { return frame < other.frame; }
    int64_t frame;
    float value;
};

inline std::string AxisToFBXName(Axis axis)
{
    static std::vector<std::string> axisName =
        { "d|X", "d|Y", "d|Z" };
        
    int a = (int)axis;
    if(a < 0)
        a = -a - 1;
    
    return axisName[a];
}

inline std::string DirToFBXAxis(AxisSetup axes, int dir)
{
    std::vector<std::string> axisName =
        { "" };
        
    axisName.push_back(AxisToFBXName(axes.right));
    axisName.push_back(AxisToFBXName(axes.up));
    axisName.push_back(AxisToFBXName(axes.front));
    
    return axisName[abs(dir)];
}

inline bool EqualsFBXAxis(Axis axis, const std::string& name)
{
    if(AxisToFBXName(axis) == name)
        return true;
    return false;
}

inline int DirFromFBXAxis(AxisSetup axes, const std::string& name)
{
    int dir = 0;
    if(EqualsFBXAxis(axes.right, name))
        if((int)(axes.right) < 0)
            dir = -1;
        else
            dir = 1;
    else if(EqualsFBXAxis(axes.up, name))
        if((int)(axes.up) < 0)
            dir = -2;
        else
            dir = 2;
    else if(EqualsFBXAxis(axes.front, name))
        if((int)(axes.front) < 0)
            dir = -3;
        else
            dir = 3;
    return dir;
}

inline void ConvertCurveName(Settings& settings, std::string& name)
{
    int origDir = DirFromFBXAxis(settings.origAxes, name);
    int convDir = DirFromFBXAxis(settings.convAxes, name);

    name = DirToFBXAxis(settings.origAxes, convDir);
}

inline void ConvertCurveMult(Settings& settings, const std::string& name, float& valMult)
{
    int origDir = DirFromFBXAxis(settings.origAxes, name);
    int convDir = DirFromFBXAxis(settings.convAxes, name);
    
    if((origDir < 0) != (convDir < 0))
        valMult = -1.0f;
}
    
class AnimationCurve
{
public:
    AnimationCurve(
        Node& root, 
        Node& node, 
        const std::string& name, 
        const std::string& curveNodeName,
        Settings& settings
        )
    : name(name)
    {
        std::vector<int64_t> keyTime =
            node.Get("KeyTime")[0].GetArray<int64_t>();
        std::vector<float> keyValue = 
            node.Get("KeyValueFloat")[0].GetArray<float>();
        defaultValue = 
            node.Get("Default")[0].GetFloat();
        
        float valMult = 1.0f;
        if(curveNodeName.compare(0, 15, "Lcl Translation") == 0)
        {
            valMult = (float)settings.scaleFactor;
        }
        else if(curveNodeName.compare(0, 12, "Lcl Rotation") == 0)
        {
            defaultValue = (defaultValue * Au::Math::PI) / 180.0f;
            valMult = Au::Math::PI / 180.0f;
        }        
            
        for(unsigned i = 0; i < keyTime.size() && i < keyValue.size(); ++i)
        {
            //int frameTime = 46186158000 / FPS;
            //int frame = keyTime[i] / frameTime;
            //std::cout << keyTime[i] << ": " << keyValue[i] << std::endl;
            
            float val = keyValue[i] * valMult;
            keyframes.push_back(Keyframe(keyTime[i], val));
        }
        
        std::sort(keyframes.begin(), keyframes.end());
    }
    
    void Evaluate(int64_t time)
    {
        Keyframe* k0 = 0, *k1 = 0;
            
        for(int i = keyframes.size() - 1; i >= 0; --i)
        {
            k0 = &keyframes[i];
            if(i == keyframes.size() - 1)
                k1 = k0;
            else
                k1 = &keyframes[i + 1];
            if(k0->frame <= time)
                break;
        }
        
        if(k0 != k1)
        {
            float a = k0->value;
            float b = k1->value;
            float t = (float)(time - k0->frame) / (float)(k1->frame - k0->frame);
            
            value = a + t * (b - a);
        }
        else if(k0)
        {
            value = k0->value;
        }
        else
        {
            value = defaultValue;
        }
    }
    
    float Value() { return value; }
    
    std::string& GetName() { return name; }
    unsigned KeyframeCount() { return keyframes.size(); }
    Keyframe* GetKeyframe(unsigned id) { return &keyframes[id]; }
private:
    std::string name;
    std::vector<Keyframe> keyframes;
    float defaultValue;
    float value;
};
    
class AnimationCurveNode
{
public:
    AnimationCurveNode(Node& root, Node& node, Settings& settings)
    {
        int64_t uid = node[0].GetInt64();
        
        Node* conn = 0;
        animatedNode = root.GetConnectedParent("Model", uid, &conn);
        if(animatedNode && conn)
        {
            objectName = (*animatedNode)[1].GetString();
            propName = (*conn)[3].GetString();
        }
        
        std::vector<Node*> conns;
        std::vector<Node*> nodes = 
            root.GetConnectedChildren("AnimationCurve", uid, conns);
        for(unsigned i = 0; i < nodes.size() && i < conns.size(); ++i)
        {
            std::string connName = (*conns[i])[3].GetString();
            curves.push_back(
                AnimationCurve(
                    root, 
                    *(nodes[i]), 
                    connName, 
                    propName, 
                    settings
                )
            );
        }
    }
    
    void Evaluate(int64_t time)
    {
        for(unsigned i = 0; i < curves.size(); ++i)
        {
            curves[i].Evaluate(time);
        }
    }
    
    Node* GetAnimatedNode() { return animatedNode; }
    
    std::string& GetObjectName() { return objectName; }
    std::string& GetPropertyName() { return propName; }
    unsigned CurveCount() { return curves.size(); }
    AnimationCurve* GetCurve(unsigned id) { return &curves[id]; }
    AnimationCurve* GetCurve(const std::string& name)
    {
        for(unsigned i = 0; i < curves.size(); ++i)
        {
            if(curves[i].GetName() == name)
                return &curves[i];
        }
        return 0;
    }
private:
    std::vector<AnimationCurve> curves;
    std::string objectName;
    std::string propName;
    Node* animatedNode;
};

class AnimationLayer
{
public:
    AnimationLayer(Node& root, Node& object, Settings& settings)
    {
        int64_t uid = object[0].GetInt64();
        
        std::vector<Node*> nodes = 
            root.GetConnectedChildren("AnimationCurveNode", uid);
        for(unsigned i = 0; i < nodes.size(); ++i)
        {
            curveNodes.push_back(AnimationCurveNode(root, *(nodes[i]), settings));
            animatedNodes.push_back(curveNodes.back().GetAnimatedNode());
        }
    }
    
    std::vector<Node*>& GetAnimatedNodes() { return animatedNodes; }
    
    unsigned CurveNodeCount() { return curveNodes.size(); }
    AnimationCurveNode* GetCurveNode(unsigned id) { return &curveNodes[id]; }
    AnimationCurveNode* GetCurveNode(const std::string& nodeName, const std::string& prop)
    {
        for(unsigned i = 0; i < curveNodes.size(); ++i)
        {
            if(curveNodes[i].GetObjectName() == nodeName &&
                curveNodes[i].GetPropertyName() == prop)
                return &curveNodes[i];
        }
        return 0;
    }
private:
    std::vector<AnimationCurveNode> curveNodes;
    std::vector<Node*> animatedNodes;
};

class AnimationStack
{
public:
    AnimationStack(Node& root, Node& object, Settings& settings)
    {
        _settings = &settings;
        int64_t uid = object[0].GetInt64();
        name = object[1].GetString();
        
        std::vector<Node*> nodes =
            root.GetConnectedChildren("AnimationLayer", uid);
        for(unsigned i = 0; i < nodes.size(); ++i)
        {
            layers.push_back(AnimationLayer(root, *(nodes[i]), settings));
        }
        
        Node& prop70 = object.Get("Properties70");
        int pCount = prop70.Count("P");
        for(int i = 0; i < pCount; ++i)
        {
            Node& p = prop70.Get("P", i);
            std::string pName = p[0].GetString();
            if(pName == "LocalStop" ||
                pName == "ReferenceStop")
            {
                length = p[4].GetInt64();
                if(length)
                    break;
            }
        }
    }
    
    Au::Math::Mat4f EvaluateTransform(SceneNode& model, int64_t time)
    {
        Au::Math::Mat4f m(1.0f);
        if(!model.SourceNode()) return m;
        std::cout << model.Name() << std::endl;
        
        Au::Math::Vec3f p = EvaluatePosition(model, time);
        Au::Math::Quat r = EvaluateRotation(model, time);
        Au::Math::Vec3f s = EvaluateScale(model, time);
        
        m = 
            Au::Math::Translate(Au::Math::Mat4f(1.0f), p) * 
            Au::Math::ToMat4(r) * 
            Au::Math::Scale(Au::Math::Mat4f(1.0f), s);
        
        return m;
    }

    bool HasPositionCurve(SceneNode& model)
    {
        if(layers.empty())
            return false;
        AnimationLayer& layer = layers[0];
        // TODO Take care of layer merging
        
        AnimationCurveNode* curveNode =
            layer.GetCurveNode(model.Name(), "Lcl Translation");
        if(!curveNode)
            return false;
        else
            return true;
    }

    bool HasRotationCurve(SceneNode& model)
    {
        if(layers.empty())
            return false;
        AnimationLayer& layer = layers[0];
        // TODO Take care of layer merging
        
        AnimationCurveNode* curveNode =
            layer.GetCurveNode(model.Name(), "Lcl Rotation");
        if(!curveNode)
            return false;
        else
            return true;
    }

    bool HasScaleCurve(SceneNode& model)
    {
        if(layers.empty())
            return false;
        
        AnimationLayer& layer = layers[0];
        // TODO Take care of layer merging
        
        AnimationCurveNode* curveNode = 
            layer.GetCurveNode(model.Name(), "Lcl Scaling");
        if(!curveNode)
            return false;
        else
            return true;
    }
    
    Au::Math::Vec3f EvaluatePosition(SceneNode& model, int64_t time)
    {
        Au::Math::Vec3f v = 
            model.LclTranslation();
        if(layers.empty())
            return v;
        AnimationLayer& layer = layers[0];
        // TODO Take care of layer merging
        
        AnimationCurveNode* curveNode =
            layer.GetCurveNode(model.Name(), "Lcl Translation");
        
        if(!curveNode)
            return v;
        curveNode->Evaluate(time);
        
        AnimationCurve* c = 0;
        c = curveNode->GetCurve("d|X");
        if(c)
            v.x = c->Value();
        c = curveNode->GetCurve("d|Y");
        if(c)
            v.y = c->Value();
        c = curveNode->GetCurve("d|Z");
        if(c)
            v.z = c->Value();
        
        return v;
    }
    
    Au::Math::Quat EvaluateRotation(SceneNode& model, int64_t time)
    {
        Au::Math::Vec3f v =
            model.LclRotation();
        Au::Math::Quat q(0.0f, 0.0f, 0.0f, 1.0f);
        if(layers.empty())
            return q;
        AnimationLayer& layer = layers[0];
        // TODO Take care of layer merging
        
        AnimationCurveNode* curveNode =
            layer.GetCurveNode(model.Name(), "Lcl Rotation");
            
        if(!curveNode)
        {
            q = Au::Math::EulerToQuat(v);
            return q;
        }
        curveNode->Evaluate(time);
        
        AnimationCurve* c = 0;
        c = curveNode->GetCurve("d|X");
        if(c)
            v.x = c->Value();
        c = curveNode->GetCurve("d|Y");
        if(c)
            v.y = c->Value();
        c = curveNode->GetCurve("d|Z");
        if(c)
            v.z = c->Value();
        
        Au::Math::Vec3f lclEuler = model.LclRotation();
        
        q = Au::Math::EulerToQuat(v);
        
        Au::Math::Quat pre = model.PreRotationQuat();
        Au::Math::Quat post = model.PostRotationQuat();
        
        Au::Math::Quat lcl = Au::Math::EulerToQuat(lclEuler);
        //if(model.LclRotationAnimMode() == SceneNode::ANIMATED_ADD)

        return pre * q * post;
    }
    
    Au::Math::Vec3f EvaluateScale(SceneNode& model, int64_t time)
    {
        Au::Math::Vec3f v =
            model.LclScaling();
        if(layers.empty())
            return v;
        
        AnimationLayer& layer = layers[0];
        // TODO Take care of layer merging
        
        AnimationCurveNode* curveNode = 
            layer.GetCurveNode(model.Name(), "Lcl Scaling");
        
        if(!curveNode)
            return v;
        curveNode->Evaluate(time);
        
        AnimationCurve* c = 0;
        c = curveNode->GetCurve("d|X");
        if(c)
            v.x = c->Value();
        c = curveNode->GetCurve("d|Y");
        if(c)
            v.y = c->Value();
        c = curveNode->GetCurve("d|Z");
        if(c)
            v.z = c->Value();
        
        return v;
    }
    
    std::vector<SceneNode> GetAnimatedNodes() 
    { 
        for(unsigned i = 0; i < layers.size(); ++i)
        {
            std::vector<Node*>& layerNodes =
                layers[i].GetAnimatedNodes();
            for(unsigned j = 0; j < layerNodes.size(); ++j)
                _addAnimatedNodeIfNotExists(layerNodes[j]);
        }
        
        std::vector<SceneNode> nodes;
        for(unsigned i = 0; i < animatedNodes.size(); ++i)
            nodes.push_back(SceneNode(*_settings, *animatedNodes[i]));
        
        return nodes;
    }
    
    int64_t GetLength() { return length; }
    std::string GetName() { return name; }
    unsigned LayerCount() { return layers.size(); }
    AnimationLayer* GetLayer(unsigned id) { return &layers[id]; }
private:
    void _addAnimatedNodeIfNotExists(Node* n)
    {
        for(unsigned i = 0; i < animatedNodes.size(); ++i)
        {
            if(animatedNodes[i] == n)
                return;
        }
        animatedNodes.push_back(n);
    }
    
    std::string name;
    std::vector<AnimationLayer> layers;
    int64_t length;
    
    std::vector<Node*> animatedNodes;
    Settings* _settings;
};

}
}
}

#endif
