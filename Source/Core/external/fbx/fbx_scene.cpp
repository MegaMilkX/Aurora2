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

    for(unsigned i = 0; i < rootNode.ChildCount("Geometry"); ++i){
        FbxNode& node = rootNode.GetNode("Geometry", i);
        int64_t uid = node.GetProperty(0).GetInt64();
        FbxGeometry* geom = GetOrCreateByUid<FbxGeometry>(uid);
        geom->Make(node, settings.scaleFactor);
    }

    for(unsigned i = 0; i < rootNode.ChildCount("C"); ++i){
        FbxNode& node = rootNode.GetNode("C", i);
        connections.Add(FbxConnection(node));
    }

    for(unsigned i = 0; i < rootNode.ChildCount("Model"); ++i){
        FbxNode& node = rootNode.GetNode("Model", i);
        FbxModel* model = _makeModel(node);
        if(model->GetType() == FbxMesh::Type())
        {
            _makeMesh(node);
        }
    }

    for(unsigned i = 0; i < rootNode.ChildCount("AnimationLayer"); ++i){
        FbxNode& node = rootNode.GetNode("AnimationLayer", i);
        int64_t uid = node.GetProperty(0).GetInt64();
        FbxAnimationLayer* animLayer = GetOrCreateByUid<FbxAnimationLayer>(uid);
        animLayer->Make(node);
    }

    for(unsigned i = 0; i < rootNode.ChildCount("AnimationStack"); ++i){
        FbxNode& node = rootNode.GetNode("AnimationStack", i);
        int64_t uid = node.GetProperty(0).GetInt64();
        FbxAnimationStack* anim = GetOrCreateByUid<FbxAnimationStack>(uid);
        
        anim->Make(node, connections);

        std::cout << anim->Name() << std::endl;
        for(size_t i = 0; i < anim->LayerCount(); ++i)
        {
            int64_t layerUid = anim->GetLayerUid(i);

        }
    }
}

void FbxScene::_makeGlobalSettings()
{
    if(rootNode.ChildCount(PROPERTY_NODE_NAME) == 0)
        return;
    FbxNode& props70 = rootNode.GetNode(PROPERTY_NODE_NAME, 0);
    for(unsigned i = 0; i < props70.ChildCount("P"); ++i)
    {
        FbxNode& prop = props70.GetNode("P", i);
        if(prop.GetProperty(0).GetString() == "UnitScaleFactor")
        {
            settings.scaleFactor = prop.GetProperty(4).GetDouble() * 0.01;
        }
    }
}

FbxModel* FbxScene::_makeModel(FbxNode& node)
{
    int64_t uid = node.GetProperty(0).GetInt64();
    FbxModel* model = GetOrCreateByUid<FbxModel>(uid);
    model->SetUid(uid);
    model->SetName(node.GetProperty(1).GetString());
    model->SetType(node.GetProperty(2).GetString());

    int64_t parent_uid = connections.FindObjectToObjectParent(uid);
    if(parent_uid <= 0)
        rootModels.emplace_back(uid);
    else
        GetOrCreateByUid<FbxModel>(parent_uid)->_addChild(uid);

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
    FbxMesh* mesh = GetOrCreateByUid<FbxMesh>(uid);
    mesh->SetUid(uid);

    int64_t child_uid = connections.FindObjectToObjectChild(uid);
    if(child_uid > 0) {
        FbxGeometry* geom = GetByUid<FbxGeometry>(child_uid);
        if(geom) {
            mesh->SetGeometryUid(child_uid);
        }
    }
}
