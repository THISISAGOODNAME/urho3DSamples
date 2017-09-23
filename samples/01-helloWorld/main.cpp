//
// Created by AICDG on 2017/9/23.
//

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>

using namespace Urho3D;
class MyApp : public Application
{
public:
    MyApp(Context* context) :
            Application(context)
    {
    }
    virtual void Setup()
    {
        // Called before engine initialization. engineParameters_ member variable can be modified here
        engineParameters_[Urho3D::EP_WINDOW_TITLE] = "01 hello world";
        engineParameters_[Urho3D::EP_FULL_SCREEN]  = false;
    }
    virtual void Start()
    {
        // Called after engine initialization. Setup application & subscribe to events here
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(MyApp, HandleKeyDown));

        // Create "Hello World" Text
        CreateText();
    }
    virtual void Stop()
    {
        // Perform optional cleanup after main loop has terminated
    }
    void HandleKeyDown(StringHash eventType, VariantMap& eventData)
    {
        using namespace KeyDown;
        // Check for pressing ESC. Note the engine_ member variable for convenience access to the Engine object
        int key = eventData[P_KEY].GetInt();
        if (key == KEY_ESCAPE)
            engine_->Exit();
    }

    void CreateText()
    {
        Urho3D::ResourceCache* cache = GetSubsystem<Urho3D::ResourceCache>();

        // Construct new Text object
        Urho3D::SharedPtr<Urho3D::Text> helloText(new Urho3D::Text(context_));

        // Set String to display
        helloText->SetText("Hello World from Urho3D!");

        // Set font and text color
        helloText->SetFont(cache->GetResource<Urho3D::Font>("Fonts/Anonymous Pro.ttf"), 30);
        helloText->SetColor(Urho3D::Color(0.0f, 1.0f, 0.0f));

        // Align Text center-screen
        helloText->SetHorizontalAlignment(Urho3D::HA_CENTER);
        helloText->SetVerticalAlignment(Urho3D::VA_CENTER);

        // Add Text instance to the UI root element
        GetSubsystem<Urho3D::UI>()->GetRoot()->AddChild(helloText);
    }
};
URHO3D_DEFINE_APPLICATION_MAIN(MyApp)