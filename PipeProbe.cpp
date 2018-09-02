#include <Urho3D/Urho3D.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Input/Input.h>

#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/SceneEvents.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Graphics/Renderer.h>

#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Geometry.h>

#include "Probe.h"
#include "PipeGenerator.h"

#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/DebugNew.h>
#include <Urho3D/Core/Profiler.h>
#include <Urho3D/Engine/DebugHud.h>

#include <Urho3D/IO/FileSystem.h>

#include <vector>
#include <iostream>

using namespace Urho3D;

const float CAMERA_DISTANCE = 45.0f;

class PipeProbe : public Application {

    SharedPtr<Scene> scene_;

    float yaw_;
    float pitch_;
    bool drawDebug_;

    SharedPtr<Node> cameraNode_;
    SharedPtr<Node> zoneNode_;

    WeakPtr<Probe> probe_;

public:

    PipeProbe(Context* context): 
        Application(context),
        yaw_(0.0f),
        pitch_(90.0f),
        drawDebug_(true) {

        SetRandomSeed(Urho3D::Time::GetTimeSinceEpoch());

        Probe::RegisterObject(context);
        PipeGenerator::RegisterObject(context);
    }

    virtual void Setup() {
        // Called before engine initialization. engineParameters_ member variable can be modified here
        engineParameters_[EP_FULL_SCREEN] = false;
    }

    virtual void Start() {
        // Called after engine initialization. Setup application & subscribe to events here
        CreateScene();
        auto *pipeGenerator = GetSubsystem<PipeGenerator>();
        pipeGenerator->Init(scene_);

        Node* probeNode = scene_->CreateChild("Probe");
        probeNode->SetPosition(Vector3(0.0f, -1.0f, 0.0f));
        probeNode->SetDirection(Vector3::DOWN);
        //probeNode->SetRotation(Quaternion(180, Vector3::RIGHT) * Quaternion(180, Vector3::DOWN));

        probe_ = probeNode->CreateComponent<Probe>();
        probe_->Init();

        //GetSubsystem<Input>()->SetMouseMode(MM_ABSOLUTE);

        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(PipeProbe, HandleKeyDown));
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(PipeProbe, HandleUpdate));
        SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(PipeProbe, HandlePostUpdate));
        SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(PipeProbe, HandlePostRenderUpdate));

        // Unsubscribe the SceneUpdate event from base class as the camera node is being controlled in HandlePostUpdate() in this sample
        UnsubscribeFromEvent(E_SCENEUPDATE);
    }

    virtual void Stop() {
        // Perform optional cleanup after main loop has terminated
    }

    void HandleKeyDown(StringHash eventType, VariantMap& eventData) {
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
    }

    void CreateScene() {
        auto* cache = GetSubsystem<ResourceCache>();

        scene_ = new Scene(context_);

        // Create scene subsystem components
        scene_->CreateComponent<Octree>();
        scene_->CreateComponent<PhysicsWorld>();
        scene_->CreateComponent<DebugRenderer>();
                
        URHO3D_PROFILE(CustomImageCopy);
        XMLFile* style = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
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

        // Create static scene content. First create a zone for ambient lighting and fog control
        zoneNode_ = scene_->CreateChild("Zone");
        auto* zone = zoneNode_->CreateComponent<Zone>();
        zone->SetAmbientColor(Color(1.0f, 1.0f, 1.0f));
        zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
        zone->SetFogStart(300.0f);
        zone->SetFogEnd(500.0f);
        zone->SetBoundingBox(BoundingBox(-2000.0f, 2000.0f));
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData) {
        using namespace Update;

        // Take the frame time step, which is stored as a float
        float timeStep = eventData[P_TIMESTEP].GetFloat();

        // Move the camera, scale movement with time step
        //MoveCamera(timeStep);

        auto* input = GetSubsystem<Input>();
        if (probe_) {
           // auto* input = GetSubsystem<Input>();
            //probe_->controls_.yaw_ += (float)input->GetMouseMoveX() * YAW_SENSITIVITY;
            //probe_->controls_.pitch_ += (float)input->GetMouseMoveY() * YAW_SENSITIVITY;

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

    void MoveCamera(float timeStep) {
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
        //pitch_ = Clamp(pitch_, -90.0f, 90.0f);

        // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
        cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

        // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
        // Use the Translate() function (default local space) to move relative to the node's orientation.
        if (input->GetKeyDown(KEY_W))
            cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
        if (input->GetKeyDown(KEY_S))
            cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
        if (input->GetKeyDown(KEY_A))
            cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
        if (input->GetKeyDown(KEY_D))
            cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
    }

    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData) {
        // If draw debug mode is enabled, draw physics debug geometry. Use depth test to make the result easier to interpret
        if (drawDebug_)
            scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
    }

    void HandlePostUpdate(StringHash eventType, VariantMap& eventData) {
        if (!probe_)
            return;

        Node* probeNode = probe_->GetNode();
        Quaternion dir = cameraNode_->GetRotation();

        /*
        Ray cameraRay(cameraNode_->GetWorldPosition(), Vector3::FORWARD);
        PhysicsRaycastResult result;
        scene_->GetComponent<PhysicsWorld>()->RaycastSingle(result, cameraRay, 1000, 2);
        if (result.body_) {
            Vector3 v(cameraRay.origin_ - result.normal_);
            float angle = cameraRay.origin_.Angle(result.normal_);

            if (angle > 0 && Abs(90 - angle) > 5 ) {
                cameraNode_->Rotate(Quaternion(90 - angle, cameraNode_->GetDirection()));
            }
        }
        */
      
        Vector3 cameraTargetPos = probeNode->GetPosition() - dir * Vector3(0.0f, 0.0f, CAMERA_DISTANCE);

        cameraNode_->SetPosition(cameraTargetPos);
        cameraNode_->SetRotation(dir);

        zoneNode_->SetPosition(cameraTargetPos);
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(PipeProbe)
