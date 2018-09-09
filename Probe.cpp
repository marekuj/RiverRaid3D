#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/Constraint.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>

#include "Probe.h"

#include <iostream>

Probe::Probe(Context* context) : LogicComponent(context) {
    // Only the physics update event is needed: unsubscribe from the rest for optimization
    SetUpdateEventMask(USE_FIXEDUPDATE);
}

void Probe::RegisterObject(Context* context) {
    context->RegisterFactory<Probe>();
}

void Probe::FixedUpdate(float timeStep) {
    Vector3 direction = node_->GetPosition() - prevPosition_;
    Vector3 force;

    // Read controls
    Quaternion probeRot = probeBody_->GetRotation();
    if (controls_.buttons_ & CTRL_LEFT) {
        force.x_ = -1.0f;
    }
    if (controls_.buttons_ & CTRL_RIGHT) {
        force.x_ = 1.0f;
    }
    if (controls_.buttons_ & CTRL_FORWARD) {
        force.z_ = -1.0f;
    }
    if (controls_.buttons_ & CTRL_BACK) {
        force.z_ = 1.0f;
    }
    
    probeBody_->ApplyForce(force.Normalized() * 100);
    node_->SetDirection(direction);
    prevPosition_ = node_->GetPosition();
}

void Probe::Init() {
    auto* object = node_->CreateComponent<StaticModel>();

    auto* cache = GetSubsystem<ResourceCache>();
    object->SetModel(cache->GetResource<Model>("Models/Probe.mdl"));
    object->SetMaterial(cache->GetResource<Material>("Materials/ProbeMaterial.xml"));
    object->SetCastShadows(true);

    probeBody_ = node_->CreateComponent<RigidBody>();
    probeBody_->SetMass(4.0f);
    probeBody_->SetLinearDamping(0.2f); // Some air resistance
    probeBody_->SetAngularDamping(0.5f);

    probeBody_->SetCollisionLayer(2);
    probeBody_->SetFriction(400.75f);
    probeBody_->SetLinearVelocity(node_->GetDirection() * 30);
    auto* probeShape = node_->CreateComponent<CollisionShape>();
    probeShape->SetGImpactMesh(object->GetModel());
}

