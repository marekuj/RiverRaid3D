#pragma once

#include <Urho3D/Core/Object.h>

namespace Urho3D {
    class Text;
}

using namespace Urho3D;

class Hud: public Object {

    URHO3D_OBJECT(Hud, Object)

public:
    explicit Hud(Context* context);
    ~Hud();
    static void RegisterObject(Context* context);

    void SetDefaultStyle(XMLFile* style);
    void Reset(const String& informationText);
    int GetPoints();
    void AddPoints(int points);
    void AddExtraPoints(int points, const IntVector2& position);

    void HandleTextAnimationFinished(StringHash eventType, VariantMap & eventData);

private:
    SharedPtr<Text> pointsValue_;
    SharedPtr<Text> information_;
    int points_;
};
