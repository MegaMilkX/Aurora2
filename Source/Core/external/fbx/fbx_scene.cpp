#include "fbx_scene.h"
#include <sstream>
#include <fstream>

void FbxScene::_dumpFile(const std::string& filename)
{
    std::ostringstream sstr;
    rootNode.Print(sstr);
    std::ofstream f(filename + ".dump", std::ios::out);
    f << sstr.str();
    f.close();
}

void FbxScene::_finalize()
{
    _makeGlobalSettings();
    for(unsigned i = 0; i < rootNode.ChildCount("C"); ++i){
        FbxNode& node = rootNode.GetNode("C", i);
        connections.Add(FbxConnection(node));
    }

    for(unsigned i = 0; i < rootNode.ChildCount("Model"); ++i){
        FbxNode& node = rootNode.GetNode("Model", i);
        FbxModel* model = _makeModel(node);
        if(model->GetType() == "Mesh")
        {
            _makeMesh(node);
        }
    }

    for(unsigned i = 0; i < rootNode.ChildCount("AnimationStack"); ++i){
        FbxAnimationStack* anim = _make<FbxAnimationStack>(rootNode.GetNode("AnimationStack", i));

        std::cout << anim->Name() << std::endl;
        for(size_t i = 0; i < anim->LayerCount(); ++i)
        {
            auto layer = anim->GetLayer(i);
            for(size_t j = 0; j < layer->CurveNodeCount(); ++j)
            {
                auto curveNode = layer->GetCurveNode(j);
                for(size_t k = 0; k < curveNode->CurveCount(); ++k)
                {
                    auto curve = curveNode->GetCurve(k);
                }
            }
        }
    }

/*
    for(unsigned i = 0; i < rootNode.ChildCount("Texture"); ++i) {
        GetByUid<FbxTexture>(rootNode.GetNode("Texture", i).GetProperty(0).GetInt64());
    }*/
    for(unsigned i = 0; i < rootNode.ChildCount("Material"); ++i) {
        GetByUid<FbxMaterial>(rootNode.GetNode("Material", i).GetProperty(0).GetInt64());
    }

}

void FbxScene::_makeGlobalSettings()
{
    if(rootNode.ChildCount(PROPERTY_NODE_NAME) == 0)
        return;

    FBX_TIME_MODE timeMode = FBX_FRAMES_60;
    double fps = 60.0;

    FbxNode& props70 = rootNode.GetNode(PROPERTY_NODE_NAME, 0);
    for(unsigned i = 0; i < props70.ChildCount("P"); ++i)
    {
        FbxNode& prop = props70.GetNode("P", i);
        if(prop.GetProperty(0).GetString() == "UnitScaleFactor") {
            settings.scaleFactor = prop.GetProperty(4).GetDouble() * 0.01;
        } else if(prop.GetProperty(0).GetString() == "TimeMode") {
            timeMode = (FBX_TIME_MODE)prop.GetProperty(4).GetInt32();
        } else if(prop.GetProperty(0).GetString() == "CustomFrameRate") {
            fps = prop.GetProperty(4).GetDouble();
        }
    }

    switch(timeMode) {
    case FBX_FRAMES_DEFAULT: break;
    case FBX_FRAMES_120: fps = 120.0; break;
    case FBX_FRAMES_100: fps = 100.0; break;
    case FBX_FRAMES_60: fps = 60.0; break;
    case FBX_FRAMES_50: fps = 50.0; break;
    case FBX_FRAMES_48: fps = 48.0; break;
    case FBX_FRAMES_30: fps = 30.0; break;
    case FBX_FRAMES_30_DROP: fps = 30.0; break;
    case FBX_FRAMES_NTSC_DROP: fps = 29.97; break;
    case FBX_FRAMES_NTSC_FULL: fps = 29.97; break;
    case FBX_FRAMES_PAL: fps = 25.0; break;
    case FBX_FRAMES_CINEMA: fps = 24.0; break;
    // Should not be used for frame rate
    case FBX_FRAMES_1000m: fps = 30.0; break;
    case FBX_FRAMES_CINEMA_ND: fps = 23.976; break;
    case FBX_FRAMES_CUSTOM: break;
    case FBX_FRAMES_96: fps = 96.0; break;
    case FBX_FRAMES_72: fps = 72.0; break;
    case FBX_FRAMES_59dot94: fps = 59.94; break;
    }

    settings.frameRate = fps;
}

FbxModel* FbxScene::_makeModel(FbxNode& node)
{
    int64_t uid = node.GetProperty(0).GetInt64();
    FbxModel* model = GetByUid<FbxModel>(uid);
    model->SetUid(uid);
    model->SetName(node.GetProperty(1).GetString());
    model->SetType(node.GetProperty(2).GetString());

    int64_t parent_uid = connections.FindObjectToObjectParent(uid);
    if(parent_uid <= 0)
        rootModels.emplace_back(uid);
    else
        GetByUid<FbxModel>(parent_uid)->_addChild(uid);

    FbxVector3 lclTranslation( 0.0f, 0.0f, 0.0f );
    FbxVector3 lclRotation( 0.0f, 0.0f, 0.0f );
    FbxVector3 lclScaling( 1.0f, 1.0f, 1.0f );
    FbxVector3 preRotation( 0.0f, 0.0f, 0.0f );
    FbxVector3 postRotation( 0.0f, 0.0f, 0.0f );
    if(node.ChildCount(PROPERTY_NODE_NAME))
    {
        FbxNode& props70 = node.GetNode(PROPERTY_NODE_NAME, 0);
        for(unsigned i = 0; i < props70.ChildCount("P"); ++i)
        {
            FbxNode& prop = props70.GetNode("P", i);
            if(prop.GetProperty(0).GetString() == "Lcl Translation")
            {
                lclTranslation.x = 
                    (float)(prop.GetProperty(4).GetDouble() * 
                    settings.scaleFactor);
                lclTranslation.y =
                    (float)(prop.GetProperty(5).GetDouble() *
                    settings.scaleFactor);
                lclTranslation.z =
                    (float)(prop.GetProperty(6).GetDouble() *
                    settings.scaleFactor);
            }
            else if(prop.GetProperty(0).GetString() == "Lcl Rotation")
            {
                lclRotation.x =
                    (float)(prop.GetProperty(4).GetDouble() * FbxPi / 180.0f);
                lclRotation.y =
                    (float)(prop.GetProperty(5).GetDouble() * FbxPi / 180.0f);
                lclRotation.z =
                    (float)(prop.GetProperty(6).GetDouble() * FbxPi / 180.0f);
            }
            else if(prop.GetProperty(0).GetString() == "Lcl Scaling")
            {
                lclScaling.x = (float)prop.GetProperty(4).GetDouble();
                lclScaling.y = (float)prop.GetProperty(5).GetDouble();
                lclScaling.z = (float)prop.GetProperty(6).GetDouble();
            }
            else if(prop.GetProperty(0).GetString() == "PreRotation")
            {
                preRotation.x =
                    (float)(prop.GetProperty(4).GetDouble() * FbxPi / 180.0f);
                preRotation.y =
                    (float)(prop.GetProperty(5).GetDouble() * FbxPi / 180.0f);
                preRotation.z =
                    (float)(prop.GetProperty(6).GetDouble() * FbxPi / 180.0f);
            }
            else if(prop.GetProperty(0).GetString() == "PostRotation")
            {
                postRotation.x =
                    (float)(prop.GetProperty(4).GetDouble() * FbxPi / 180.0f);
                postRotation.y =
                    (float)(prop.GetProperty(5).GetDouble() * FbxPi / 180.0f);
                postRotation.z =
                    (float)(prop.GetProperty(6).GetDouble() * FbxPi / 180.0f);
            }
        }
    }

    FbxQuat preQuat = FbxEulerToQuat(preRotation);
    FbxQuat postQuat = FbxEulerToQuat(postRotation);
    FbxQuat rotation = FbxEulerToQuat(lclRotation);

    FbxMatrix4 transform = 
        FbxTranslate(FbxMatrix4(1.0f), lclTranslation) *
        FbxToMatrix4(preQuat * rotation * postQuat) *
        FbxScale(FbxMatrix4(1.0f), lclScaling);

    model->SetTransform(transform);
    model->SetLclTranslation(lclTranslation);
    model->SetLclRotation(lclRotation);
    model->SetLclScaling(lclScaling);

    return model;
}

void FbxScene::_makeMesh(FbxNode& node)
{
    int64_t uid = node.GetProperty(0).GetInt64();
    FbxMesh* mesh = GetByUid<FbxMesh>(uid);
    /*
    objects[FbxTypeInfo<FbxMesh>::Index()].objects[uid].reset(mesh);
    objects[FbxTypeInfo<FbxMesh>::Index()].uids.emplace_back(uid);
*/
    mesh->SetUid(uid);

    int64_t child_uid = connections.FindObjectToObjectChild(uid);
    if(child_uid > 0) {
        FbxGeometry* geom = GetByUid<FbxGeometry>(child_uid);
        if(geom) {
            mesh->SetGeometryUid(child_uid);
        }
    }
}
