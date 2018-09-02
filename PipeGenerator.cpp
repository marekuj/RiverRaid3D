#pragma once

#include "PipeGenerator.h"

#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>

#include <iostream>
#include <algorithm>
#include <vector>

PipeGenerator::PipeGenerator(Context *context): Object(context), models_(), nextPos_(Vector3::ZERO), pipes_(4) {
}

PipeGenerator::~PipeGenerator() {
}

void PipeGenerator::RegisterObject(Context* context) {
    context->RegisterSubsystem<PipeGenerator>();
}

void PipeGenerator::LoadModels() {
    auto* cache = GetSubsystem<ResourceCache>();
    
    for (auto resourceDir : cache->GetResourceDirs()) {
        std::vector<String> tmp;
        String fullDir = resourceDir + PIPE_MODEL_DIR;
        ScanPipeModels(tmp, fullDir);

        std::transform(tmp.begin(), tmp.end(), std::back_inserter(models_), [fullDir, cache](String file) -> Model* {
            return cache->GetResource<Model>(fullDir + file);
        });
    }

    material_ = cache->GetResource<Material>("Materials/RustyMetalMaterial.xml");
}

void PipeGenerator::Start() {
    auto* cache = GetSubsystem<ResourceCache>();
    GeneratePipes();
}

void PipeGenerator::GeneratePipes() {
    if (!pipes_.size() > 0) {
        for (auto* pipe : pipes_) {
            pipe->Remove();
        }
        pipes_.clear();
    }

    for (int i = 0; i < 2; i++) {
        Node* pipeNode = scene_->CreateChild("Pipe");
        pipeNode->SetScale(5);

        pipeNode->SetRotation(Quaternion(30 * (Rand() % 15), Vector3::DOWN));
        pipeNode->SetPosition(nextPos_);

        auto* object = pipeNode->CreateComponent<StaticModel>();
            
        auto* model = models_[Rand() % models_.size()];
        object->SetModel(model);
        object->SetMaterial(material_);

        auto* body = pipeNode->CreateComponent<RigidBody>();
        body->SetCollisionLayer(2);
        auto* shape = pipeNode->CreateComponent<CollisionShape>();
        shape->SetGImpactMesh(object->GetModel(), 0);

        nextPos_.y_ -= model->GetBoundingBox().Size().y_ * pipeNode->GetScale().y_;
        GenerateEnemies(pipeNode);

        pipes_.push_back(pipeNode);
    }
}

void PipeGenerator::GenerateEnemies(Node* pipeNode) {

}

void PipeGenerator::Init(Scene *scene) {
    scene_ = scene;
    LoadModels();
    Start();
}

float PipeGenerator::GetEdge() {
    return nextPos_.y_;
}

void PipeGenerator::ScanPipeModels(std::vector<String> & result, const String& pathName) {
    StringVector models;
    GetSubsystem<FileSystem>()->ScanDir(models, pathName, ".mdl", SCAN_FILES, false);

    for (auto model : models) {
        result.push_back(model);
    }
}

