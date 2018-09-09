#pragma once

#include <Urho3D/Core/Object.h>
#include <vector>

namespace Urho3D {
    class Scene;
    class Model;
    class Material;
}

using namespace Urho3D;

const float  PIPE_RADIUS = 10.0f;
const String PIPE_MODEL_DIR = "Models/Pipes/";
const String TRASH_MODEL_DIR = "Models/Trash/";
const String TRASH_MATERIAL_DIR = "Materials/Trash/";

class PipeGenerator: public Object {

    URHO3D_OBJECT(PipeGenerator, Object)

public:
    std::vector<Model*> pipeModels_;

    static void RegisterObject(Context* context);

    PipeGenerator(Context* context);
    void Init(Scene* scene);
    void Reset();
    float GetEdge();
    void GeneratePipes();

private:
    std::vector<Node*> pipes_;
    std::vector<Model*> trashModels_;
    std::vector<Material*> trashMaterials_;
    WeakPtr<Scene> scene_;
    WeakPtr<Material> pipeMaterial_;
    Vector3 nextPos_;

    void Start();
    void LoadModels();
    void ScanFiles(std::vector<String>& result, const String& pathName, String ext = ".mdl");
    void GenerateLights(Node* pipeNode);
    void GenerateObstacles(Node* pipeNode);
    Node* RandomObstacle(Node* parent);
};
