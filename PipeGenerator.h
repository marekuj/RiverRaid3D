#pragma once

#include <Urho3D/Core/Object.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/Scene/Node.h>
#include <Urho3D/Core/Context.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Material.h>

#include <vector>

using namespace Urho3D;

const float  PIPE_RADIUS = 10.0f;
const String PIPE_MODEL_DIR = "Models/Pipes/";
const String TRASH_MODEL_DIR = "Models/Trash/";
const String TRASH_MATERIAL_DIR = "Materials/Trash/";

class PipeGenerator: public Object {


    URHO3D_OBJECT(PipeGenerator, Object)

public:
    std::vector<Model*> pipeModels_;

    static void RegisterObject(Context * context);

    PipeGenerator(Context *context);
    ~PipeGenerator();
    void Init(Scene *scene);
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
    void ScanFiles(std::vector<String> &result, const String& pathName, String ext = ".mdl");
    void GenerateLights(Node* pipeNode);
    void GenerateEnemies(Node* pipeNode);
    Node* RandomEnemy(Node* parent);
};
