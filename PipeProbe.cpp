#include <Urho3D/Urho3D.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/InputEvents.h>

// remove below along with SayHello sample
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Resource/ResourceCache.h>

using namespace Urho3D;

class PipeProbe : public Application {

public:
    PipeProbe(Context* context): Application(context) {
    }

    virtual void Setup() {
        // Called before engine initialization. engineParameters_ member variable can be modified here
        engineParameters_[EP_FULL_SCREEN] = false;
    }

    virtual void Start() {
        SayHello();
        // Called after engine initialization. Setup application & subscribe to events here
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(PipeProbe, HandleKeyDown));
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
    }

    void SayHello() {
        auto* cache = GetSubsystem<ResourceCache>();

        // Construct new Text object
        SharedPtr<Text> helloText(new Text(context_));

        // Set String to display
        helloText->SetText("Pipe Probe!");

        // Set font and text color
        helloText->SetFont(cache->GetResource<Font>("Fonts/BlueHighway.ttf"), 30);
        helloText->SetColor(Color(0.0f, 1.0f, 0.0f));

        // Align Text center-screen
        helloText->SetHorizontalAlignment(HA_CENTER);
        helloText->SetVerticalAlignment(VA_CENTER);

        // Add Text instance to the UI root element
        GetSubsystem<UI>()->GetRoot()->AddChild(helloText);
    }
};

URHO3D_DEFINE_APPLICATION_MAIN(PipeProbe)
