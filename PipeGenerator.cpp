#pragma once

#include <Urho3D/Core/Context.h>

#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/VertexBuffer.h>

#include <Urho3D/IO/FileSystem.h>

#include <Urho3D/Math/MathDefs.h>

#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>

#include <Urho3D/Resource/ResourceCache.h>

#include <Urho3D/Scene/Scene.h>

#include <algorithm>
#include <iostream>

#include "CollisionLayers.h"
#include "Obstacle.h"
#include "PipeGenerator.h"

PipeGenerator::PipeGenerator(Context *context): Object(context), pipeModels_(), nextPos_(Vector3::ZERO) {
}

void PipeGenerator::RegisterObject(Context* context) {
    context->RegisterSubsystem<PipeGenerator>();
}

void PipeGenerator::LoadModels() {
    auto* cache = GetSubsystem<ResourceCache>();
    
    for (const auto& resourceDir : cache->GetResourceDirs()) {
        std::vector<String> tmp;
        String fullDir = resourceDir + PIPE_MODEL_DIR;
        ScanFiles(tmp, fullDir);

        std::transform(tmp.begin(), tmp.end(), std::back_inserter(pipeModels_), [fullDir, cache](String file) -> Model* {
            return cache->GetResource<Model>(fullDir + file);
        });
    }
    pipeMaterial_ = cache->GetResource<Material>("Materials/RustyMetalMaterial.xml");

    for (const auto& resourceDir : cache->GetResourceDirs()) {
        std::vector<String> tmp;
        String fullDir = resourceDir + TRASH_MODEL_DIR;
        ScanFiles(tmp, fullDir);

        std::transform(tmp.begin(), tmp.end(), std::back_inserter(trashModels_), [fullDir, cache](String file) -> Model* {
            return cache->GetResource<Model>(fullDir + file);
        });
    }

    for (const auto& resourceDir : cache->GetResourceDirs()) {
        std::vector<String> tmp;
        String fullDir = resourceDir + TRASH_MATERIAL_DIR;
        ScanFiles(tmp, fullDir, ".xml");

        std::transform(tmp.begin(), tmp.end(), std::back_inserter(trashMaterials_), [fullDir, cache](String file) -> Material* {
            return cache->GetResource<Material>(fullDir + file);
        });
    }
}

void PipeGenerator::Start() {
    GeneratePipes();
}

void PipeGenerator::GeneratePipes() {
    if (pipes_.size() > 10) {
        for (std::vector<Node*>::iterator it = pipes_.begin(); it != pipes_.end() - 5; ++it) {
            (*it)->Remove();
        }
        pipes_.erase(pipes_.begin(), pipes_.end() - 5);
    }

    for (int j = 0; j < 3; ++j) {
        Node* pipeNode = scene_->CreateChild("Pipe");
        pipeNode->SetScale(5);

        pipeNode->SetRotation(Quaternion(30 * (Rand() % 15), Vector3::DOWN));
        pipeNode->SetPosition(nextPos_);

        auto* object = pipeNode->CreateComponent<StaticModel>();
            
        auto* model = pipeModels_[Rand() % pipeModels_.size()];
        object->SetModel(model);
        object->SetMaterial(pipeMaterial_);

        auto* body = pipeNode->CreateComponent<RigidBody>();
        body->SetCollisionLayer(LAYER_WORLD | LAYER_PIPE);
        auto* shape = pipeNode->CreateComponent<CollisionShape>();
        shape->SetGImpactMesh(object->GetModel(), 0);

        GenerateLights(pipeNode);
        if (nextPos_ != Vector3::ZERO) { //do not generate obstacles for very first pipe
            GenerateObstacles(pipeNode);
        }

        pipes_.push_back(pipeNode);

        nextPos_.y_ -= model->GetBoundingBox().Size().y_ * pipeNode->GetScale().y_;
    }
}

void PipeGenerator::GenerateLights(Node* pipeNode) {
    auto* model = pipeNode->GetComponent<StaticModel>()->GetModel();

    auto buff = model->GetVertexBuffers()[0];
    unsigned char* data = buff->GetShadowData();

    unsigned vertexCount = buff->GetVertexCount();
    unsigned vertexSize = buff->GetVertexSize();

    unsigned normalStart = VertexBuffer::GetElementOffset(buff->GetElements(), TYPE_VECTOR3, SEM_NORMAL);
    unsigned vertexStart = VertexBuffer::GetElementOffset(buff->GetElements(), TYPE_VECTOR3, SEM_POSITION);

    int offset = vertexCount / 4;
    for (int j = 0; j < vertexCount; j+=offset) {        
        const Vector3& vertex = *((const Vector3*)(&data[(vertexStart + j) * vertexSize]));
        const Vector3& normal = *((const Vector3*)(&data[(vertexStart + j) * vertexSize + normalStart]));

        Node* lightNode = pipeNode->CreateChild("PointLight");
        auto* light = lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_POINT);
        light->SetRange(150);
        lightNode->SetPosition(vertex);
        lightNode->SetDirection(normal);
    }
}

void PipeGenerator::GenerateObstacles(Node* pipeNode) {
    auto* model = pipeNode->GetComponent<StaticModel>()->GetModel();

    auto buff = model->GetVertexBuffers()[0];
    unsigned char* data = buff->GetShadowData();

    unsigned vertexCount = buff->GetVertexCount();
    unsigned vertexSize = buff->GetVertexSize();

    unsigned normalStart = VertexBuffer::GetElementOffset(buff->GetElements(), TYPE_VECTOR3, SEM_NORMAL);
    unsigned vertexStart = VertexBuffer::GetElementOffset(buff->GetElements(), TYPE_VECTOR3, SEM_POSITION);

    for (int j = 0; j < 5; ++j) {
        int rand = Rand() % vertexCount;
        const Vector3& vertex = *((const Vector3*)(&data[(vertexStart + rand) * vertexSize]));
        const Vector3&  normal = *((const Vector3*)(&data[(vertexStart + rand) * vertexSize + normalStart]));

        auto* obstacle = RandomObstacle(pipeNode);
        obstacle->SetPosition(vertex + (2.5f + Random(PIPE_RADIUS)) * normal);
    }
}

Node* PipeGenerator::RandomObstacle(Node* parent) {
    auto* obstacleNode = parent->CreateChild("obstacle");
    auto* obstacle = obstacleNode->CreateComponent<Obstacle>();
    auto* model = trashModels_[Rand() % trashModels_.size()];
    auto* material = trashMaterials_[Rand() % trashMaterials_.size()];
    obstacle->Init(model, material);

    return obstacleNode;
}

void PipeGenerator::Init(Scene *scene) {
    scene_ = scene;
    LoadModels();
    Start();
}

void PipeGenerator::Reset() {
    for (auto pipe : pipes_) {
        pipe->Remove();
    }

    pipes_.clear();
    nextPos_ = Vector3::ZERO;
}

float PipeGenerator::GetEdge() {
    return nextPos_.y_;
}

void PipeGenerator::ScanFiles(std::vector<String> & result, const String& pathName, String ext) {
    StringVector files;
    GetSubsystem<FileSystem>()->ScanDir(files, pathName, ext, SCAN_FILES, false);

    for (auto file : files) {
        result.push_back(file);
    }
}

