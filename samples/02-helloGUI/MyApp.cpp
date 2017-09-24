//
// Created by AICDG on 2017/9/24.
//

#include <Urho3D/Core/Object.h>
#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/CheckBox.h>
#include <Urho3D/UI/LineEdit.h>
#include <Urho3D/UI/Font.h>

#include "MyApp.h"

URHO3D_DEFINE_APPLICATION_MAIN(MyApp);

MyApp::MyApp(Urho3D::Context *context)
        : Urho3D::Application(context)
        , uiRoot_(GetSubsystem<Urho3D::UI>()->GetRoot())
{
}

void MyApp::Setup(){
    // Called before engine initialization. engineParameters_ member variable can be modified here
    engineParameters_["WindowTitle"] = "02 hello GUI";
    engineParameters_["FullScreen"]  = false;
    engineParameters_["HighDPI"] = true;
    engineParameters_["WindowWidth"] = 800;
    engineParameters_["WindowHeight"] = 600;
    engineParameters_["VSync"] = true;
}

void MyApp::Start(){
    // Called after engine initialization. Setup application & subscribe to events here
    SubscribeToEvent(Urho3D::E_KEYDOWN, URHO3D_HANDLER(MyApp, HandleKeyDown));

    // Enable OS cursor
    GetSubsystem<Urho3D::Input>()->SetMouseVisible(true);

    // Load XML file containing default UI style sheet
    Urho3D::ResourceCache* cache = GetSubsystem<Urho3D::ResourceCache>();
    Urho3D::XMLFile* style = cache->GetResource<Urho3D::XMLFile>("UI/DefaultStyle.xml");

    // Set the loaded style as default style
    uiRoot_->SetDefaultStyle(style);

    // Initialize Window
    InitWindow();

    // Create and add some controls to the Window
    InitControls();
}

void MyApp::Stop() {

}

void MyApp::InitWindow() {
    // Create the Window and add it to the UI's root node
    window_ = new Urho3D::Window(context_);
    uiRoot_->AddChild(window_);

    // Set Window size and layout settings
    window_->SetMinWidth(384);
    window_->SetLayout(Urho3D::LM_VERTICAL, 6, Urho3D::IntRect(6, 6, 6, 6));
    window_->SetAlignment(Urho3D::HA_CENTER, Urho3D::VA_CENTER);
    window_->SetName("Window");

    // Create Window 'titlebar' container
    Urho3D::UIElement* titleBar = new Urho3D::UIElement(context_);
    titleBar->SetMinSize(0, 24);
    titleBar->SetVerticalAlignment(Urho3D::VA_TOP);
    titleBar->SetLayoutMode(Urho3D::LM_HORIZONTAL);

    // Create the Window title Text
    Urho3D::Text* windowTitle = new Urho3D::Text(context_);
    windowTitle->SetName("WindowTitle");
    windowTitle->SetText("Hello GUI!");

    // Create the Window's close button
    Urho3D::Button* buttonClose = new Urho3D::Button(context_);
    buttonClose->SetName("CloseButton");

    // Add the controls to the title bar
    titleBar->AddChild(windowTitle);
    titleBar->AddChild(buttonClose);

    // Add the title bar to the Window
    window_->AddChild(titleBar);

    // Apply styles
    window_->SetStyleAuto();
    windowTitle->SetStyleAuto();
    buttonClose->SetStyle("CloseButton");

    // Subscribe to buttonClose release (following a 'press') events
    SubscribeToEvent(buttonClose, Urho3D::E_RELEASED, URHO3D_HANDLER(MyApp, HandleClosePressed));

    // Subscribe also to all UI mouse clicks just to see where we have clicked
    SubscribeToEvent(Urho3D::E_UIMOUSECLICK, URHO3D_HANDLER(MyApp, HandleControlClicked));
}

void MyApp::InitControls() {
    // Create a CheckBox
    Urho3D::CheckBox* checkBox = new Urho3D::CheckBox(context_);
    checkBox->SetName("CheckBox");

    // Create a Button
    Urho3D::Button* button = new Urho3D::Button(context_);
    button->SetName("Button");
    button->SetMinHeight(24);
    Urho3D::Text* buttonText = button->CreateChild<Urho3D::Text>();
    buttonText->SetAlignment(Urho3D::HA_CENTER, Urho3D::VA_CENTER);
    Urho3D::ResourceCache* cache = GetSubsystem<Urho3D::ResourceCache>();
    Urho3D::Font* font = cache->GetResource<Urho3D::Font>("Fonts/Anonymous Pro.ttf");
    buttonText->SetFont(font, 12);
    buttonText->SetText("Our Button");

    // Create a LineEdit
    Urho3D::LineEdit* lineEdit = new Urho3D::LineEdit(context_);
    lineEdit->SetName("LineEdit");
    lineEdit->SetMinHeight(24);

    // Add controls to Window
    window_->AddChild(checkBox);
    window_->AddChild(button);
    window_->AddChild(lineEdit);

    // Apply previously set default style
    checkBox->SetStyleAuto();
    button->SetStyleAuto();
    lineEdit->SetStyleAuto();
}

void MyApp::HandleKeyDown(Urho3D::StringHash eventType, Urho3D::VariantMap &eventData) {
    using namespace Urho3D::KeyDown;
    // Check for pressing ESC. Note the engine_ member variable for convenience access to the Engine object
    int key = eventData[P_KEY].GetInt();
    if (key == Urho3D::KEY_ESCAPE)
        engine_->Exit();

}

void MyApp::HandleClosePressed(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    if (Urho3D::GetPlatform() != "Web")
        engine_->Exit();
}

void MyApp::HandleControlClicked(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData)
{
    // Get the Text control acting as the Window's title
    Urho3D::Text* windowTitle = window_->GetChildStaticCast<Urho3D::Text>("WindowTitle", true);

    // Get control that was clicked
    Urho3D::UIElement* clicked = static_cast<Urho3D::UIElement*>(eventData[Urho3D::UIMouseClick::P_ELEMENT].GetPtr());

    Urho3D::String name = "...?";
    if (clicked)
    {
        // Get the name of the control that was clicked
        name = clicked->GetName();
    }

    // Update the Window's title text
    windowTitle->SetText("Hello " + name + "!");
}