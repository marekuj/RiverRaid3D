#pragma once

#include <Urho3D/Input/Controls.h>
#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

const unsigned CTRL_FORWARD = 1;
const unsigned CTRL_BACK = 2;
const unsigned CTRL_LEFT = 4;
const unsigned CTRL_RIGHT = 8;

const float ENGINE_POWER = 10.0f;

class Probe : public LogicComponent {

    URHO3D_OBJECT(Probe, LogicComponent)

public:

    explicit Probe(Context* context);
    static void RegisterObject(Context* context);

    /// Handle physics world update. Called by LogicComponent base class.
    void FixedUpdate(float timeStep) override;

    /// Initialize the vehicle. Create rendering and physics components. Called by the application.
    void Init();

    /// Movement controls.
    Controls controls_;

private:
    WeakPtr<RigidBody> probeBody_;
    WeakPtr<Camera> camera_;
    WeakPtr<Node> reflectorNode_;

    Vector3 prevPosition_;
    Timer speedTimer_;
};

