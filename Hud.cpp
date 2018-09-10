#include <Urho3D/Core/Context.h>

#include <Urho3D/Graphics/DrawableEvents.h>

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>

#include <Urho3D/Scene/Node.h>
#include <Urho3D/Scene/ValueAnimation.h>

#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/UIEvents.h>

#include "Hud.h"

Hud::Hud(Context* context): Object(context), points_(0) {
    auto* ui = GetSubsystem<UI>();
    UIElement* uiRoot = ui->GetRoot();

    auto* cache = GetSubsystem<ResourceCache>();

    pointsValue_ = new Text(context_);
    pointsValue_->SetVisible(false);
    uiRoot->AddChild(pointsValue_);

    information_ = new Text(context_);
    information_->SetVisible(false);
    uiRoot->AddChild(information_);
}

Hud::~Hud() {
    pointsValue_->Remove();
    information_->Remove();
}

void Hud::RegisterObject(Context* context) {
    context->RegisterSubsystem<Hud>();
}

void Hud::SetDefaultStyle(XMLFile* style) {
    pointsValue_->SetDefaultStyle(style);
    pointsValue_->SetStyle("PointsHudText");

    information_->SetDefaultStyle(style);
    information_->SetStyle("InformationHudText");
}

void Hud::Reset(const String& informationText) {
    information_->SetText(informationText);
    information_->SetVisible(true);
    pointsValue_->SetVisible(false);
    points_ = 0;
}

int Hud::GetPoints() {
    return points_;
}

void Hud::AddPoints(int points) {
    if (!pointsValue_->IsVisible()) {
        information_->SetVisible(false);
        pointsValue_->SetVisible(true);
    }

    points_ += points;

    String text;
    text.AppendWithFormat("Points:\t\t %d", points_);
    pointsValue_->SetText(text);
}

void Hud::AddExtraPoints(int points, const IntVector2& position) {
    points_ += points;

    WeakPtr<Text> text(new Text(context_));
    String msg;
    msg.AppendWithFormat("+%d", points);
    text->SetStyle("Text");
    text->SetDefaultStyle(pointsValue_->GetDefaultStyle());
    text->SetText(msg);
    text->SetColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
    text->SetPosition(position);
    text->SetTemporary(true);

    GetSubsystem<UI>()->GetRoot()->AddChild(text);

    // Create light color animation
    SharedPtr<ValueAnimation> textAnimation(new ValueAnimation(context_));
    textAnimation->SetKeyFrame(1.0f, Color(1.0f, 1.0f, 1.0f, 1.0f));
    textAnimation->SetKeyFrame(9.0f, Color(1.0f, 1.0f, 1.0f, 0.1f));

    VariantMap eventData;
    eventData[0] = text;
    textAnimation->SetEventFrame(10.0f, E_ANIMATIONFINISHED, eventData);

    textAnimation->SetInterpolationMethod(InterpMethod::IM_LINEAR);
    
    // Set Light component's color animation
    text->SetAttributeAnimation("Color", textAnimation, WM_ONCE, 10.0f);
    SubscribeToEvent(E_ANIMATIONFINISHED, URHO3D_HANDLER(Hud, HandleTextAnimationFinished));
}

void Hud::HandleTextAnimationFinished(StringHash eventType, VariantMap& eventData) {
    Text* text = static_cast<Text*>(eventData[0].GetPtr());
    text->Remove();
}