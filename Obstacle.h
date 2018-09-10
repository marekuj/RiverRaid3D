#pragma once

#include <Urho3D/Scene/LogicComponent.h>

using namespace Urho3D;

class Obstacle: public LogicComponent {

    URHO3D_OBJECT(Obstacle, LogicComponent)

public:
    explicit Obstacle(Context* context);

    static void RegisterObject(Context* context);

    /// Initialize the vehicle. Create rendering and physics components. Called by the application.
    void Init(Model* model, Material* material);

    /// Handle physics world update. Called by LogicComponent base class.
    void FixedUpdate(float timeStep) override;

private:
    float floatFactor_;
};

