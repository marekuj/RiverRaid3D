#include <Urho3D/Core/Context.h>

#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIElement.h>
#include <Urho3D/UI/UIEvents.h>

#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Resource/XMLFile.h>

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
