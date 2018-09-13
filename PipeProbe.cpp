#include <Urho3D/Urho3D.h>

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/Timer.h>

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>

#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Geometry.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>

#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>

#include <Urho3D/Math/Ray.h>

#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>

#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/PhysicsEvents.h>

#include <Urho3D/Resource/ResourceCache.h>

#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

#include "CollisionLayers.h"
#include "Hud.h"
#include "PipeProbe.h"
#include "Probe.h"
#include "PipeGenerator.h"
#include "Obstacle.h"

#include <Urho3D/Core/Profiler.h>
#include <Urho3D/DebugNew.h>
#include <Urho3D/Engine/DebugHud.h>
#include <Urho3D/Graphics/DebugRenderer.h>

#include <vector>
#include <iostream>

PipeProbe::PipeProbe(Context* context):
    Application(context),
    yaw_(0.0f),
    pitch_(90.0f),
    drawDebug_(false) {

    SetRandomSeed(Time::GetTimeSinceEpoch());

    Hud::RegisterObject(context);
    Probe::RegisterObject(context);
    PipeGenerator::RegisterObject(context);
    Obstacle::RegisterObject(context);
}

void PipeProbe::Setup() {
    // Called before engine initialization. engineParameters_ member variable can be modified here
    engineParameters_[EP_FULL_SCREEN] = false;
}

void PipeProbe::Start() {
    // Called after engine initialization. Setup application & subscribe to events here
    CreateScene();
    GetSubsystem<PipeGenerator>()->Init(scene_);
    
    SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(PipeProbe, HandleKeyDown));
    SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(PipeProbe, HandleUpdate));
    SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(PipeProbe, HandlePostUpdate));
    SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(PipeProbe, HandlePostRenderUpdate));
    SubscribeToEvent(E_PHYSICSCOLLISIONSTART, URHO3D_HANDLER(PipeProbe, HandleProbeCollision));

    // Unsubscribe the SceneUpdate event from base class as the camera node is being controlled in HandlePostUpdate() in this sample
    UnsubscribeFromEvent(E_SCENEUPDATE);

    GetSubsystem<Hud>()->Reset("Press ENTER to start...");
}

void PipeProbe::StartGamePlay() {
    if (probe_ != nullptr && probe_->IsEnabled()) {
        return;
    }

    if (probe_ != nullptr) {
        probe_->GetNode()->Remove();
        GetSubsystem<PipeGenerator>()->Reset();
    }

    Node* probeNode = scene_->CreateChild("Probe");
    probeNode->SetPosition(Vector3(0.0f, -1.0f, 0.0f));
    probeNode->SetDirection(Vector3::DOWN);

    probe_ = probeNode->CreateComponent<Probe>();
    probe_->Init();

    pointsTimer_.Reset();
}

void PipeProbe::StopGamePlay() {
    probe_->SetEnabled(false);

    auto* hud = GetSubsystem<Hud>();
    String information;
    information.AppendWithFormat("Probe has been crashed!\nScore %d\nPress ENTER to try again.", hud->GetPoints());
    hud->Reset(information);
}

void PipeProbe::Stop() {
    // Perform optional cleanup after main loop has terminated
}

void PipeProbe::HandleProbeCollision(StringHash eventType, VariantMap& eventData) {
    using namespace PhysicsCollisionStart;

    if (probe_ == nullptr || !probe_->IsEnabled()) {
        return;
    }

    Node* nodeA = static_cast<Node*>(eventData[P_NODEA].GetPtr());
    Node* nodeB = static_cast<Node*>(eventData[P_NODEB].GetPtr());

    if (nodeA->GetComponent<Probe>() || nodeB->GetComponent<Probe>()) {
        StopGamePlay();
    }
}

void PipeProbe::HandleKeyDown(StringHash eventType, VariantMap& eventData) {
    using namespace KeyDown;
    // Check for pressing ESC. Note the engine_ member variable for convenience access to the Engine object
    int key = eventData[P_KEY].GetInt();
    if (key == KEY_ESCAPE)
        engine_->Exit();

    if (key == KEY_F1)
        drawDebug_ ^= true;

    if (key == KEY_F2) {
        auto* debugHud = GetSubsystem<DebugHud>();
        if (debugHud->GetMode() == DEBUGHUD_SHOW_NONE) {
            debugHud->SetMode(DEBUGHUD_SHOW_STATS);
        } else {
            debugHud->SetMode(DEBUGHUD_SHOW_NONE);
        }
    }

    if (key == KEY_RETURN || key == KEY_RETURN2 || key == KEY_KP_ENTER) {
        StartGamePlay();
    }
}

void PipeProbe::CreateScene() {
    auto* cache = GetSubsystem<ResourceCache>();

    scene_ = new Scene(context_);

    // Create scene subsystem components
    scene_->CreateComponent<Octree>();
    world_ = scene_->CreateComponent<PhysicsWorld>();
    scene_->CreateComponent<DebugRenderer>();

    XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
    GetSubsystem<Hud>()->SetDefaultStyle(style);
                
    URHO3D_PROFILE(CustomImageCopy);
    DebugHud* debugHud = engine_->CreateDebugHud();
    debugHud->SetDefaultStyle(style);
    debugHud->SetMode(DEBUGHUD_SHOW_STATS);

    // Create camera and define viewport. We will be doing load / save, so it's convenient to create the camera outside the scene,
    // so that it won't be destroyed and recreated, and we don't have to redefine the viewport on load
    cameraNode_ = new Node(context_);
    cameraNode_->SetPosition(Vector3::ZERO);
    cameraNode_->SetDirection(Vector3::DOWN);
    auto* camera = cameraNode_->CreateComponent<Camera>();
    camera->SetFarClip(500.0f);

    GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));
}

void PipeProbe::HandleUpdate(StringHash eventType, VariantMap& eventData) {
    using namespace Update;

    // Take the frame time step, which is stored as a float
    float timeStep = eventData[P_TIMESTEP].GetFloat();

    // Move the camera, scale movement with time step
    MoveCamera(timeStep);

    auto* input = GetSubsystem<Input>();
    if (probe_) {
        auto* ui = GetSubsystem<UI>();
        if (!ui->GetFocusElement()) {
            probe_->controls_.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
            probe_->controls_.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
            probe_->controls_.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
            probe_->controls_.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));
        } else {
            probe_->controls_.Set(CTRL_FORWARD | CTRL_BACK | CTRL_LEFT | CTRL_RIGHT, false);
        }
    }
}

void PipeProbe::MoveCamera(float timeStep) {
    // Do not move if the UI has a focused element (the console)
    if (GetSubsystem<UI>()->GetFocusElement())
        return;

    auto* input = GetSubsystem<Input>();

    // Movement speed as world units per second
    const float MOVE_SPEED = 90.0f;
    // Mouse sensitivity as degrees per pixel
    const float MOUSE_SENSITIVITY = 0.1f;

    // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
    IntVector2 mouseMove = input->GetMouseMove();
    yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
    pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
    yaw_ = Clamp(yaw_, -90.0f, 90.0f);
    pitch_ = Clamp(pitch_, 0.0f, 180.0f);

    // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
    cameraNode_->SetRotation(Quaternion(pitch_, 0.0f, 0.0f) * Quaternion(0.0f, yaw_, 0.0f));
}

void PipeProbe::HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {
    // If draw debug mode is enabled, draw physics debug geometry. Use depth test to make the result easier to interpret
    if (drawDebug_)
        scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
}

void PipeProbe::HandlePostUpdate(StringHash eventType, VariantMap& eventData) {
    if (!probe_ || !probe_->IsEnabled())
        return;

    Node* probeNode = probe_->GetNode();
    Quaternion dir = cameraNode_->GetRotation();
    
    Vector3 cameraStartPos = probeNode->GetPosition();
    Vector3 cameraTargetPos = cameraStartPos - dir * Vector3(0.0f, 0.0f, CAMERA_DISTANCE);

    Ray cameraRay(cameraStartPos,  cameraTargetPos - cameraStartPos);
    PhysicsRaycastResult raycastResult;
    world_->RaycastSingle(raycastResult, cameraRay, (cameraTargetPos - cameraStartPos).Length(), LAYER_PIPE);
    if (raycastResult.body_) {
        cameraTargetPos = cameraStartPos + cameraRay.direction_ * (raycastResult.distance_ - 0.5f);
    }

    cameraNode_->SetPosition(cameraTargetPos);

    auto * pipeGenerator = GetSubsystem<PipeGenerator>();
    if (probeNode->GetPosition().y_ - 500 < pipeGenerator->GetEdge()) {
        pipeGenerator->GeneratePipes();
    }

    if (pointsTimer_.GetMSec(false) > 100) {
        GetSubsystem<Hud>()->AddPoints(1);
        pointsTimer_.Reset();
    }

}
