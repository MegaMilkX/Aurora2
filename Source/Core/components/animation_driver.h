#ifndef ANIMATION_DRIVER_H
#define ANIMATION_DRIVER_H

#include <updatable.h>
#include <animation.h>

/*
class AnimMotorVec3 {
public:
    AnimMotorVec3() : prop(rttr::type::get<AnimMotorVec3>().get_property("INVALID")) {}
    AnimMotorVec3(SceneObject::Component* c, rttr::property prop, curve* x, curve* y, curve* z)
    : comp(c), prop(prop), curves{x, y, z} {

    }

    void Overwrite(float cursor) {
        gfxm::vec3 evaluated(
            curves[0]->at(cursor),
            curves[1]->at(cursor),
            curves[2]->at(cursor)
        );
        prop.set_value(comp, evaluated);
    }

    void Blend(float cursor, float weight) {
        gfxm::vec3 before = prop.get_value(comp).get_value<gfxm::vec3>();
        gfxm::vec3 evaluated(
            curves[0]->at(cursor),
            curves[1]->at(cursor),
            curves[2]->at(cursor)
        );
        prop.set_value(comp, gfxm::lerp(before, evaluated, weight));
    }

    void Add(float cursor, float weight) {
        gfxm::vec3 before = prop.get_value(comp).get_value<gfxm::vec3>();
        gfxm::vec3 start_evaluated(
            curves[0]->at(0.0f),
            curves[1]->at(0.0f),
            curves[2]->at(0.0f)
        );
        gfxm::vec3 evaluated(
            curves[0]->at(cursor),
            curves[1]->at(cursor),
            curves[2]->at(cursor)
        );
        gfxm::vec3 delta = evaluated - start_evaluated;
        delta *= weight;
        prop.set_value(comp, before + delta);
    }
private:
    SceneObject::Component* comp;
    rttr::property prop;
    curve* curves[3];
};

class AnimMotorRotation {
public:
    AnimMotorRotation() : prop(rttr::type::get<AnimMotorRotation>().get_property("INVALID")) {}
    AnimMotorRotation(SceneObject::Component* c, rttr::property prop, curve* x, curve* y, curve* z)
    : comp(c), prop(prop), curves{x, y, z} { }
    void Overwrite(float cursor) {
        gfxm::vec3 evaluated(
            curves[0]->at(cursor),
            curves[1]->at(cursor),
            curves[2]->at(cursor)
        );
        prop.set_value(comp, evaluated);
    }
    void Blend(float cursor, float weight) {
        gfxm::vec3 before = prop.get_value(comp).get_value<gfxm::vec3>();
        gfxm::vec3 evaluated(
            curves[0]->at(cursor),
            curves[1]->at(cursor),
            curves[2]->at(cursor)
        );
        gfxm::quat qa = gfxm::euler_to_quat(before);
        gfxm::quat qb = gfxm::euler_to_quat(evaluated);
        gfxm::quat fin = gfxm::slerp(qa, qb, weight);
        prop.set_value(comp, gfxm::to_euler(fin));
    }
    void Add(float cursor, float weight) {
        gfxm::vec3 before = prop.get_value(comp).get_value<gfxm::vec3>();
        gfxm::vec3 start_evaluated(
            curves[0]->at(0.0f),
            curves[1]->at(0.0f),
            curves[2]->at(0.0f)
        );
        gfxm::vec3 evaluated(
            curves[0]->at(cursor),
            curves[1]->at(cursor),
            curves[2]->at(cursor)
        );
        gfxm::quat qa = gfxm::euler_to_quat(start_evaluated);
        gfxm::quat qb = gfxm::euler_to_quat(evaluated);
        gfxm::quat qc = gfxm::euler_to_quat(before);
        gfxm::quat fin = gfxm::slerp(qa, qb, weight);
        fin = qc * fin;
        prop.set_value(comp, gfxm::to_euler(fin));
    }
private:
    SceneObject::Component* comp;
    rttr::property prop;
    curve* curves[3];
};
*/

class AnimPropMotorBase {
public:
    virtual ~AnimPropMotorBase() {}
    virtual void Overwrite(float) = 0;
    virtual void Blend(float, float) = 0;
    virtual void Add(float, float) = 0;
};

template<typename T>
class AnimPropMotor : public AnimPropMotorBase {
public:
    AnimPropMotor(SceneObject::Component* c, rttr::property p, curve<T>* cu)
    : comp(c), prop(p), curv(cu) { }
    void Overwrite(float cursor) {
        prop.set_value(comp, curv->at(cursor));
    }
    void Blend(float cursor, float weight) {
        T before = prop.get_value(comp).get_value<T>();
        T eval = curv->at(cursor);
        prop.set_value(comp, Interpolate(before, eval, weight));
    }
    void Add(float cursor, float weight) {
        T before = prop.get_value(comp).get_value<T>();
        T zero_eval = curv->at(0.0f);
        T eval = curv->at(cursor);
        T delta = Diff(zero_eval, eval);
        prop.set_value(comp, AddValues(before, delta));
    }
private:
    SceneObject::Component* comp;
    rttr::property prop;
    curve<T>* curv;

    template<typename T>
    T Diff(const T& a, const T& b) {
        return a - b;
    }
    template<>
    gfxm::quat Diff(const gfxm::quat& a, const gfxm::quat& b) {
        return gfxm::inverse(a) * b;
    }

    template<typename T>
    T AddValues(const T& a, const T& b) {
        return a + b;
    }
    template<>
    gfxm::quat AddValues(const gfxm::quat& a, const gfxm::quat& b) {
        return a * b;
    }

    template<typename T>
    T Interpolate(const T& a, const T& b, float f) {
        return gfxm::lerp(a, b, f);
    }
    template<>
    gfxm::quat Interpolate(const gfxm::quat& a, const gfxm::quat& b, float f) {
        return gfxm::slerp(a, b, f);
    }
};

class AnimMotor {
public:
    void SetAnim(std::shared_ptr<Animation> anim) {
        this->anim = anim;
    }
    template<typename T>
    void Add(SceneObject::Component* c, rttr::property p, curve<T>* cu) {
        motors.emplace_back(
            std::shared_ptr<AnimPropMotorBase>(
                new AnimPropMotor<T>(c, p, cu)
            )
        );
    }
    void Length(float len) { length = len; }
    float Length() const { return length; }
    void FrameRate(float fps) { frameRate = fps; }
    float FrameRate() const { return frameRate; }

    void Overwrite(float cursor) {
        for(auto& motor : motors) {
            motor->Overwrite(cursor);
        }
    }
    void Blend(float cursor, float weight) {
        for(auto& motor : motors) {
            motor->Blend(cursor, weight);
        }
    }
    void Add(float cursor, float weight) {
        for(auto& motor : motors) {
            motor->Add(cursor, weight);
        }
    }
private:
    float length = 0.0f;
    float frameRate = 60.0f;
    std::vector<std::shared_ptr<AnimPropMotorBase>> motors;
    std::shared_ptr<Animation> anim;
};

class AnimationDriver;
class AnimLayer {
public:
    enum MODE {
        OVERWRITE,
        BLEND,
        ADD
    };
    AnimLayer(AnimationDriver* driver);
    void Play(const std::string& anim);
    void BlendOver(const std::string& anim, float speed);
    void SetMode(MODE mode) { this->mode = mode; }
    MODE Mode() const { return mode; }
    void Looping(bool v) { looping = v; }
    bool Looping() const { return looping; }
    void SetStrength(float s) { strength = s; }
    float Strength() const { return strength; }
    const std::string& CurrentAnimName() const { return currentAnimName; }

    void Tick(float dt) {
        if(!current) return;
        cursor += dt * current->FrameRate();
        if(cursor > current->Length()) {
            if(looping) {
                cursor = cursor - current->Length();
            } else {
                cursor = current->Length();
            }
        }

        if(mode == OVERWRITE) {
            current->Overwrite(cursor);
        } else if (mode == BLEND) {
            current->Blend(cursor, strength);
        } else if (mode == ADD) {
            current->Add(cursor, strength);
        }
    }
private:
    float cursor = 0.0f;

    AnimationDriver* driver;
    MODE mode = OVERWRITE;
    bool looping = true;
    float blend = 0.0f;
    float blendTick = 0.0f;
    float strength = 1.0f;
    AnimMotor* current = 0;
    AnimMotor* blendTarget = 0;
    std::string currentAnimName;
};

class AnimationDriver : public Updatable
{
    RTTR_ENABLE(Updatable)
public:
    AnimationDriver() {
        AddLayer();
    }

    size_t LayerCount() const { return layers.size(); }
    AnimLayer* GetLayer(size_t i) { return &layers[i]; }
    AnimLayer* AddLayer() { 
        layers.emplace_back(AnimLayer(this));
        return &layers[layers.size() - 1];
    }
    void RemoveLayer(size_t i) {
        layers.erase(layers.begin() + i);
    }

    template<typename T>
    bool AddMotor(AnimMotor& motor, SceneObject::Component* comp, rttr::property prop, AnimationNode& animNode) {
        rttr::type prop_type = prop.get_type();
        if(prop_type != rttr::type::get<T>()) {
            return false;
        }
        if(animNode.CurveType() != rttr::type::get<curve<T>>()) {
            std::cout << "Curve type and property are mismatched" << std::endl;
            return false;
        }

        motor.Add(comp, prop, animNode.GetCurve<T>());

        return true;
    }

    void AddAnim(const std::string& name, std::shared_ptr<Animation> anim) {
        if(!anim) return;
        AnimMotor& motor = motors[name];
        motor.SetAnim(anim);
        motor.Length(anim->Length());
        motor.FrameRate(anim->FrameRate());
        for(size_t i = 0; i < anim->TargetCount(); ++i) {
            auto target = anim->GetTarget(i);
            auto so = Object()->FindObject(target->name);
            if(!so) continue;
            for(auto& comp_node : target->children) {
                rttr::type comp_type = rttr::type::get_by_name(comp_node.second.name);
                SceneObject::Component* comp = so->Get(comp_node.second.name);
                if(!comp) continue;
                for(auto& prop_node : comp_node.second.children) {
                    rttr::property prop = comp_type.get_property(prop_node.second.name);
                    if(!prop.is_valid()) continue;
                    rttr::type prop_type = prop.get_type();

                    if(AddMotor<float>(motor, comp, prop, prop_node.second) ||
                        AddMotor<gfxm::vec2>(motor, comp, prop, prop_node.second) ||
                        AddMotor<gfxm::vec3>(motor, comp, prop, prop_node.second) ||
                        AddMotor<gfxm::vec4>(motor, comp, prop, prop_node.second) ||
                        AddMotor<gfxm::quat>(motor, comp, prop, prop_node.second)) {
                    }
                }
            }
        }
    }

    void AddAnim(const std::string& name, const std::string& resource) {
        std::shared_ptr<Animation> anim = 
            GlobalResourceFactory().Get<Animation>(resource);
        AddAnim(name, anim);
    }

    size_t AnimCount() { return motors.size(); }
    AnimMotor* GetAnim(size_t i) { 
        auto it = motors.begin();
        std::advance(it, i);
        if(it == motors.end()) return 0;
        return &it->second;
    }
    AnimMotor* GetAnim(const std::string& name) {
        auto it = motors.find(name);
        if(it == motors.end()) return 0;
        return &it->second;
    }
    const std::string& GetAnimName(size_t i) { 
        auto it = motors.begin();
        std::advance(it, i);
        if(it == motors.end()) return "";
        return it->first;
    }

    virtual void OnUpdate() {
        for(auto& l : layers) {
            l.Tick(1.0f / 60.0f);
        }
    }
private:
    std::vector<AnimLayer> layers;
    std::map<std::string, AnimMotor> motors;
};
STATIC_RUN(AnimationDriver) {
    rttr::registration::enumeration<AnimLayer::MODE>("AnimLayerMode")(
        rttr::value("OVERWRITE", AnimLayer::OVERWRITE),
        rttr::value("BLEND", AnimLayer::BLEND),
        rttr::value("ADD", AnimLayer::ADD)  
    );

    rttr::registration::class_<AnimationDriver>("AnimationDriver")
        .constructor<>()(
            rttr::policy::ctor::as_raw_ptr
        );
}

inline AnimLayer::AnimLayer(AnimationDriver* driver)
: driver(driver) {

}

inline void AnimLayer::Play(const std::string& anim) {
    if(!driver) return;
    current = driver->GetAnim(anim);
    blend = 0.0f;
    blendTick = 0.0f;
    if(!current) return;
    currentAnimName = anim;
    cursor = 0.0f;
}

#endif
