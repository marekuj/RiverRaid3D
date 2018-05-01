#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>

#include <vector>

using namespace Urho3D;


class PipeGenerator: public Object {

    const String PIPE_MODEL_DIR = "Models/Pipes/";

    URHO3D_OBJECT(PipeGenerator, Object)

public:
    std::vector<Model*> models_;

    static void RegisterObject(Context * context);

    PipeGenerator(Context *context);
    ~PipeGenerator();
    void Init(Scene *scene);
    float GetEdge();
    void GeneratePipes();
private:
    std::vector<Node*> pipes_;
    WeakPtr<Scene> scene_;
    WeakPtr<Material> material_;
    Vector3 nextPos_;

    void Start();
    void LoadModels();
    void ScanPipeModels(std::vector<String> &result, const String& pathName);
    void GenerateEnemies(Node* pipeNode);
};
