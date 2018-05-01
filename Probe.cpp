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
    if (node_->GetPosition().y_ - 150 < pipeGenerator_->GetEdge()) {
        pipeGenerator_->GeneratePipes();
    }

    float pitch = 0.0f;
    float roll = 0.0f;

    Vector3 direction = node_->GetPosition() - prevPosition_;
    Vector3 force;

    // Read controls
    Quaternion probeRot = probeBody_->GetRotation();
    if (controls_.buttons_ & CTRL_LEFT) {
        roll = 1.0f;
        force.x_ = -1.0f;
    }
    if (controls_.buttons_ & CTRL_RIGHT) {
        roll = -1.0f;
        force.x_ = 1.0f;
    }
    if (controls_.buttons_ & CTRL_FORWARD) {
        pitch = 1.0f;
        force.z_ = -1.0f;
    }
    if (controls_.buttons_ & CTRL_BACK) {
        pitch = -1.0f;
        force.z_ = 1.0f;
    }


    if (pitch != 0.0f) {
        probeRot = probeRot * Quaternion(pitch, Vector3::RIGHT);
    }
    if (roll != 0.0f) {
        probeRot = probeRot * Quaternion(roll, Vector3::FORWARD);
    }

    probeBody_->SetRotation(probeRot);
    probeBody_->ApplyForce(force * 300);

    prevPosition_ = node_->GetPosition();
}

void Probe::Init() {
    pipeGenerator_ = context_->GetSubsystem<PipeGenerator>();
    auto* object = node_->CreateComponent<StaticModel>();

    //node_->SetRotation(Quaternion(90, Vector3::LEFT));
    //node_->SetDirection(Vector3(0.0f, 1.0f, 0.0f));
    //controls_.pitch_ = 90;

    auto* cache = GetSubsystem<ResourceCache>();
    object->SetModel(cache->GetResource<Model>("Models/Probe.mdl"));
    object->SetMaterial(cache->GetResource<Material>("Materials/ProbeMaterial.xml"));
    object->SetCastShadows(true);

    // Create RigidBody and CollisionShape components like above. Give the RigidBody mass to make it movable
    // and also adjust friction. The actual mass is not important; only the mass ratios between colliding
    // objects are significant
    probeBody_ = node_->CreateComponent<RigidBody>();
    probeBody_->SetMass(4.0f);
    probeBody_->SetLinearDamping(0.2f); // Some air resistance
    probeBody_->SetAngularDamping(0.5f);

    probeBody_->SetCollisionLayer(2);
    //probeBody->SetFriction(400.75f);
    //probeBody_->SetLinearVelocity(node_->GetDirection() * 10);
    auto* probeShape = node_->CreateComponent<CollisionShape>();
    //probeShape->SetBox(object->GetModel()->GetBoundingBox().Size());
    probeShape->SetGImpactMesh(object->GetModel());
}

