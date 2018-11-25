#ifndef ANIMATION_DRIVER_H
#define ANIMATION_DRIVER_H

#include <updatable.h>
#include <animation.h>

#include <util/dialog_write_data.h>

/*
class AnimMotorVec3 {
public:
    AnimMotorVec3() : prop(rttr::type::get<AnimMotorVec3>().get_property("INVALID")) {}
    AnimMotorVec3(Component* c, rttr::property prop, curve* x, curve* y, curve* z)
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
    Component* comp;
    rttr::property prop;
    curve* curves[3];
};

class AnimMotorRotation {
public:
    AnimMotorRotation() : prop(rttr::type::get<AnimMotorRotation>().get_property("INVALID")) {}
    AnimMotorRotation(Component* c, rttr::property prop, curve* x, curve* y, curve* z)
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
    Component* comp;
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
    AnimPropMotor(Component* c, rttr::property p, curve<T>* cu)
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
    Component* comp;
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
    void Add(Component* c, rttr::property p, curve<T>* cu) {
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
    void SetSpeed(float s) { speed = s; }
    float Speed() const { return speed; }
    void SetCursor(float c) { 
        cursor = c; 
        if(cursor > current->Length()) {
            if(looping) {
                cursor = cursor - current->Length();
            } else {
                cursor = current->Length();
            }
        }
    }
    float Cursor() const { return cursor; }
    void SetAutoplay(bool a) { autoplay = a; }
    bool Autoplay() const { return autoplay; }

    const std::string& CurrentAnimName() const { return currentAnimName; }

    void Tick(float dt) {
        if(!current) return;
        if(autoplay) {
            cursor += dt * speed * current->FrameRate();
            if(cursor > current->Length()) {
                if(looping) {
                    cursor = cursor - current->Length();
                } else {
                    cursor = current->Length();
                }
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
    bool autoplay = true;
    float blend = 0.0f;
    float blendTick = 0.0f;
    float strength = 1.0f;
    float speed = 1.0f;
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
    bool AddMotor(AnimMotor& motor, Component* comp, rttr::property prop, AnimationNode& animNode) {
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
                Component* comp = so->Get(comp_node.second.name);
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

    virtual bool _write(std::ostream& out, ExportData& exportData) {
        
        return true;
    }
    virtual bool _read(std::istream& in, size_t sz, ImportData& importData) {
        
        return true;
    }
    virtual bool _editor() {
        AnimationDriver* driver = this;
        bool preview = true;
        if(ImGui::Checkbox("Preview", &preview)) {}
        if(preview) glfwPostEmptyEvent();

        for(size_t i = 0; i < driver->LayerCount(); ++i)
        {
            AnimLayer* layer = driver->GetLayer(i);
            if (ImGui::TreeNode(MKSTR("Layer " << i).c_str()))
            {
                if (ImGui::BeginCombo("Current anim", layer->CurrentAnimName().c_str(), 0)) // The second parameter is the label previewed before opening the combo.
                {
                    for(size_t n = 0; n < driver->AnimCount(); ++n) {
                        const std::string& anim_name = driver->GetAnimName(n);
                        bool is_selected = (anim_name == layer->CurrentAnimName());
                        if(ImGui::Selectable(anim_name.c_str(), is_selected)) {
                            layer->Play(anim_name);
                        }
                        if(is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                rttr::type t = rttr::type::get<AnimLayer::MODE>();
                rttr::enumeration en = t.get_enumeration();
                if(ImGui::BeginCombo("Mode", en.value_to_name(layer->Mode()).to_string().c_str()))
                {                            
                    for(auto& n : en.get_names()) {
                        auto& val = en.name_to_value(n);
                        bool is_selected = (val.get_value<AnimLayer::MODE>() == layer->Mode());
                        if(ImGui::Selectable(n.to_string().c_str(), is_selected)) {
                            layer->SetMode(val.get_value<AnimLayer::MODE>());
                        }
                        if(is_selected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
                float weight = layer->Strength();
                float speed = layer->Speed();
                float cursor = layer->Cursor();
                if(ImGui::DragFloat("Cursor", &cursor, 0.01f)) {
                    layer->SetCursor(cursor);
                }
                if(ImGui::DragFloat("Speed", &speed, 0.01f, 0.0f, 10.0f)) {
                    layer->SetSpeed(speed);
                }
                if(ImGui::DragFloat("Weight", &weight, 0.01f, 0.0f, 1.0f)) {
                    layer->SetStrength(weight);
                }
                bool looping = layer->Looping();
                if(ImGui::Checkbox("Looping", &looping)) {
                    layer->Looping(looping);
                } ImGui::SameLine();
                bool autoplay = layer->Autoplay();
                if(ImGui::Checkbox("Autoplay", &autoplay)) {
                    layer->SetAutoplay(autoplay);
                }
                if(ImGui::Button("Remove Layer")) {
                    driver->RemoveLayer(i);
                }
                ImGui::TreePop();
            }
        }
        if(ImGui::Button("Add Layer")) {
            driver->AddLayer();
        }
        if (ImGui::TreeNode("Animations"))
        {
            int i = 0;
            for(auto kv : motors) {
                const std::string& name = kv.first;

                char buf[128];
                strcpy_s(buf, 128, name.c_str());
                ImGui::InputText((std::string("##") + std::to_string(i)).c_str(), buf, 128);
                ImGui::Button(name.c_str()); ImGui::SameLine();
                if(ImGui::Button((std::string("Export##") + std::to_string(i)).c_str())) {
                    DialogExportResource(kv.second.anim, ".anim");
                }

                ++i;
            }
            if(ImGui::Button("+")) {
                auto window = Editor::GUI::WindowPool::Add(std::shared_ptr<EditorDataPick<Animation>>(
                    new EditorDataPick<Animation>(
                        [this](std::shared_ptr<Animation> anim){
                            if(anim){
                                AddAnim(anim->Name(), anim);
                            }
                        }
                    )
                ));
                window->SetSuffix("anim");
            }

            ImGui::TreePop();
        }
        return true;
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

    GlobalSceneSerializer()
        .CustomComponentWriter<AnimationDriver>([](Component* c, nlohmann::json&){
            AnimationDriver* driver = (AnimationDriver*)c;
            nlohmann::json janim_array = nlohmann::json::array();
            
        });
    GlobalSceneSerializer()
        .CustomComponentReader<AnimationDriver>([](Component* c, nlohmann::json&){
            
        });
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
