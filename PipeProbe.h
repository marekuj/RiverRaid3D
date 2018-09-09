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

    SharedPtr<Scene> scene_;

    float yaw_;
    float pitch_;
    bool drawDebug_;

    SharedPtr<Node> cameraNode_;
    SharedPtr<Node> reflectorNode_;

    WeakPtr<Probe> probe_;

public:

    explicit PipeProbe(Context* context);

    void Setup() override;
    void Start() override;
    void Stop() override;

    void HandleKeyDown(StringHash eventType, VariantMap& eventData);
    void CreateScene();
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    void MoveCamera(float timeStep);
    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData);
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);
};

URHO3D_DEFINE_APPLICATION_MAIN(PipeProbe)
