#pragma once

#include <Urho3D/Engine/Application.h>

namespace Urho3D {
    class Node;
    class Scene;
}

using namespace Urho3D;

class Probe;

const float CAMERA_DISTANCE = 45.0f;

class PipeProbe: public Application {

    URHO3D_OBJECT(PipeProbe, Application)

public:
    explicit PipeProbe(Context* context);

    void Setup() override;
    void Start() override;
    void Stop() override;
    void StartGamePlay();
    void StopGamePlay();

    void CreateScene();
    void MoveCamera(float timeStep);

    void HandleProbeCollision(StringHash eventType, VariantMap & eventData);
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);

private:
    SharedPtr<Scene> scene_;
    SharedPtr<Node> cameraNode_;
    SharedPtr<PhysicsWorld> world_;
    WeakPtr<Probe> probe_;

    Timer pointsTimer_;
    float yaw_;
    float pitch_;
    bool drawDebug_;
};

URHO3D_DEFINE_APPLICATION_MAIN(PipeProbe)
