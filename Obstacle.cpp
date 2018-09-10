#include <Urho3D/Core/Context.h>

#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>

#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/RigidBody.h>

#include "Obstacle.h"
#include "CollisionLayers.h"

Obstacle::Obstacle(Context* context) : LogicComponent(context), floatFactor_(0) {
    SetUpdateEventMask(USE_FIXEDUPDATE);
}

void Obstacle::RegisterObject(Context* context) {
    context->RegisterFactory<Obstacle>();
}

void Obstacle::Init(Model* model, Material* material) {
    node_->SetScale(Vector3::ONE * 0.4f);
    node_->SetRotation(Quaternion(Random(180), Vector3::DOWN) * Quaternion(Random(180), Vector3::RIGHT));

    auto* object = node_->CreateComponent<StaticModel>();
    object->SetModel(model);
    object->SetMaterial(material);

    auto* body = node_->CreateComponent<RigidBody>();
    body->SetCollisionLayer(LAYER_WORLD | LAYER_OBSTACLE);
    node_->CreateComponent<CollisionShape>()->SetGImpactMesh(model);
}

void Obstacle::FixedUpdate(float timeStep) {
    node_->SetPosition(node_->GetPosition() + Sin(floatFactor_++) * timeStep * Vector3::ONE);
}
