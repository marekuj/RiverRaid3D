#pragma once

#include "PipeGenerator.h"

#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/VertexBuffer.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Math/MathDefs.h>

#include <iostream>
#include <algorithm>
#include <vector>

PipeGenerator::PipeGenerator(Context *context): Object(context), pipeModels_(), nextPos_(Vector3::ZERO), pipes_(4) {
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
        ScanFiles(tmp, fullDir);

        std::transform(tmp.begin(), tmp.end(), std::back_inserter(pipeModels_), [fullDir, cache](String file) -> Model* {
            return cache->GetResource<Model>(fullDir + file);
        });
    }
    pipeMaterial_ = cache->GetResource<Material>("Materials/RustyMetalMaterial.xml");

    for (auto resourceDir : cache->GetResourceDirs()) {
        std::vector<String> tmp;
        String fullDir = resourceDir + TRASH_MODEL_DIR;
        ScanFiles(tmp, fullDir);

        std::transform(tmp.begin(), tmp.end(), std::back_inserter(trashModels_), [fullDir, cache](String file) -> Model* {
            return cache->GetResource<Model>(fullDir + file);
        });
    }

    for (auto resourceDir : cache->GetResourceDirs()) {
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
    if (!pipes_.size() > 0) {
        for (auto* pipe : pipes_) {
            pipe->Remove();
        }
        pipes_.clear();
    }

    for (int j = 0; j < 2; ++j) {
        Node* pipeNode = scene_->CreateChild("Pipe");
        pipeNode->SetScale(5);

        pipeNode->SetRotation(Quaternion(30 * (Rand() % 15), Vector3::DOWN));
        pipeNode->SetPosition(nextPos_);

        auto* object = pipeNode->CreateComponent<StaticModel>();
            
        auto* model = pipeModels_[Rand() % pipeModels_.size()];
        object->SetModel(model);
        object->SetMaterial(pipeMaterial_);

        auto* body = pipeNode->CreateComponent<RigidBody>();
        body->SetCollisionLayer(2);
        auto* shape = pipeNode->CreateComponent<CollisionShape>();
        shape->SetGImpactMesh(object->GetModel(), 0);

        nextPos_.y_ -= model->GetBoundingBox().Size().y_ * pipeNode->GetScale().y_;

        GenerateLights(pipeNode);
        GenerateEnemies(pipeNode);
        pipes_.push_back(pipeNode);
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

    int j = 0;
    int offset = vertexCount / 4;
    while (j < vertexCount) {
        const Vector3& vertex = *((const Vector3*)(&data[(vertexStart + j) * vertexSize]));
        const Vector3&  normal = *((const Vector3*)(&data[(vertexStart + j) * vertexSize + normalStart]));

        Node* lightNode = pipeNode->CreateChild("PointLight");
        auto* light = lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_POINT);
        light->SetRange(100);
        lightNode->SetPosition(vertex);
        lightNode->SetDirection(normal);

        j += offset;
    }
}

void PipeGenerator::GenerateEnemies(Node* pipeNode) {
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

        auto* enemy = RandomEnemy(pipeNode);
        enemy->SetPosition(vertex + (1.5f + Random(PIPE_RADIUS)) * normal);
    }
}

Node* PipeGenerator::RandomEnemy(Node* parent) {
    auto* enemyNode = parent->CreateChild("enemy");
    enemyNode->SetScale(Vector3::ONE * 0.4f);
    enemyNode->SetRotation(Quaternion(Random(180), Vector3::DOWN) * Quaternion(Random(180), Vector3::RIGHT));

    auto* cache = GetSubsystem<ResourceCache>();
    auto* object = enemyNode->CreateComponent<StaticModel>();        
    auto* model = trashModels_[Rand() % trashModels_.size()];
    object->SetModel(model);
    object->SetMaterial(trashMaterials_[Rand() % trashMaterials_.size()]);

    auto* body = enemyNode->CreateComponent<RigidBody>();
    body->SetCollisionLayer(2);
    enemyNode->CreateComponent<CollisionShape>()->SetGImpactMesh(model);

    return enemyNode;
}

void PipeGenerator::Init(Scene *scene) {
    scene_ = scene;
    LoadModels();
    Start();
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

