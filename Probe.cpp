#include <Urho3D/Core/Context.h>

#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/StaticModel.h>

#include <Urho3D/Math/Ray.h>

#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/Constraint.h>
#include <Urho3D/Physics/PhysicsEvents.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>

#include <Urho3D/Resource/ResourceCache.h>

#include <Urho3D/Scene/Scene.h>

#include <iostream>

#include "CollisionLayers.h"
#include "Hud.h"
#include "Probe.h"

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

    probeBody_->ApplyForce(force.Normalized() * 3.0f * probeBody_->GetLinearVelocity().Length());
    node_->SetDirection(direction);
    prevPosition_ = node_->GetPosition();

    if (speedTimer_.GetMSec(false) > 5000) {
        probeBody_->SetLinearDamping(probeBody_->GetLinearDamping() - 0.01f);
        speedTimer_.Reset();
    }

    Ray ray(node_->GetPosition(), direction);
    PhysicsRaycastResult result;
    GetScene()->GetComponent<PhysicsWorld>()->SphereCast(result, ray, 4.0f, 0.1f, LAYER_OBSTACLE);
    if (result.body_) {
        result.body_->SetCollisionLayer(LAYER_WORLD);

        Vector2 flatPos(camera_->WorldToScreenPoint(result.position_));
        IntVector2 windowSize(GetSubsystem<Graphics>()->GetSize());
        windowSize.x_ *= flatPos.x_;
        windowSize.y_ *= flatPos.y_;
        GetSubsystem<Hud>()->AddExtraPoints(100, windowSize);
    }
}

void Probe::Init() {
    camera_ = GetSubsystem<Renderer>()->GetViewport(0)->GetCamera();
    auto* object = node_->CreateComponent<StaticModel>();

    auto* cache = GetSubsystem<ResourceCache>();
    object->SetModel(cache->GetResource<Model>("Models/Probe.mdl"));
    object->SetMaterial(cache->GetResource<Material>("Materials/ProbeMaterial.xml"));
    object->SetCastShadows(true);

    probeBody_ = node_->CreateComponent<RigidBody>();
    probeBody_->SetMass(4.0f);
    probeBody_->SetLinearDamping(0.2f);
    probeBody_->SetAngularDamping(0.5f);

    probeBody_->SetCollisionLayer(LAYER_WORLD);
    probeBody_->SetFriction(400.75f);
    probeBody_->SetLinearVelocity(node_->GetDirection() * 40);
    auto* probeShape = node_->CreateComponent<CollisionShape>();
    probeShape->SetGImpactMesh(object->GetModel());

    // Create probe reflector
    reflectorNode_ = node_->CreateChild("PointLight");
    reflectorNode_->SetDirection(Vector3::FORWARD);
    auto* light = reflectorNode_->CreateComponent<Light>();
    light->SetLightType(LIGHT_SPOT);
    light->SetRange(250);
}

