//
// Created by AICDG on 2017/9/24.
//

#ifndef URHO3DSAMPLES_MYAPP_H
#define URHO3DSAMPLES_MYAPP_H

#include <Urho3D/Engine/Application.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>
#include <Urho3D/UI/Window.h>

class MyApp : public Urho3D::Application {
    URHO3D_OBJECT(MyApp, Urho3D::Application);

public:
    MyApp(Urho3D::Context* context);

    /// Setup before engine initialization. Modifies the engine parameters.
    virtual void Setup();
    /// Setup after engine initialization. Creates the logo, console & debug HUD.
    virtual void Start();
    /// Cleanup after the main loop. Called by Application.
    virtual void Stop();

private:
    /// Create and initialize a Window control.
    void InitWindow();
    /// Create and add various common controls for demonstration purposes.
    void InitControls();
    /// Handle key down event to process key controls common to all samples.
    void HandleKeyDown(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    /// Handle any UI control being clicked.
    void HandleControlClicked(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);
    /// Handle close button pressed and released.
    void HandleClosePressed(Urho3D::StringHash eventType, Urho3D::VariantMap& eventData);

    /// The Window.
    Urho3D::SharedPtr<Urho3D::Window> window_;
    /// The UI's root UIElement.
    Urho3D::SharedPtr<Urho3D::UIElement> uiRoot_;
};


#endif //URHO3DSAMPLES_MYAPP_H
