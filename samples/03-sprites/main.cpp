//
// Created by AICDG on 2017/9/25.
//

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/UI/Sprite.h>
#include <Urho3D/UI/UI.h>

using namespace Urho3D;

// Number of sprites to draw
static const unsigned NUM_SPRITES = 100;

// Custom variable identifier for storing sprite velocity within the UI element
static const StringHash VAR_VELOCITY("Velocity");

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
        engineParameters_[Urho3D::EP_WINDOW_TITLE] = "03 sprites";
        engineParameters_[Urho3D::EP_FULL_SCREEN]  = false;
    }
    virtual void Start()
    {
        // Create the sprites to the user interface
        CreateSprites();

        // Called after engine initialization. Setup application & subscribe to events here
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(MyApp, HandleKeyDown));
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MyApp, HandleUpdate));
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

    void CreateSprites()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        Graphics* graphics = GetSubsystem<Graphics>();
        UI* ui = GetSubsystem<UI>();

        // Get rendering window size as floats
        float width = (float)graphics->GetWidth();
        float height = (float)graphics->GetHeight();

        // Get the Urho3D fish texture
        Texture2D* decalTex = cache->GetResource<Texture2D>("Textures/UrhoDecal.dds");

        for (unsigned i = 0; i < NUM_SPRITES; ++i)
        {
            // Create a new sprite, set it to use the texture
            SharedPtr<Sprite> sprite(new Sprite(context_));
            sprite->SetTexture(decalTex);

            // The UI root element is as big as the rendering window, set random position within it
            sprite->SetPosition(Vector2(Random() * width, Random() * height));

            // Set sprite size & hotspot in its center
            sprite->SetSize(IntVector2(128, 128));
            sprite->SetHotSpot(IntVector2(64, 64));

            // Set random rotation in degrees and random scale
            sprite->SetRotation(Random() * 360.0f);
            sprite->SetScale(Random(1.0f) + 0.5f);

            // Set random color and additive blending mode
            sprite->SetColor(Color(Random(0.5f) + 0.5f, Random(0.5f) + 0.5f, Random(0.5f) + 0.5f));
            sprite->SetBlendMode(BLEND_ADD);

            // Add as a child of the root UI element
            ui->GetRoot()->AddChild(sprite);

            // Store sprite's velocity as a custom variable
            sprite->SetVar(VAR_VELOCITY, Vector2(Random(200.0f) - 100.0f, Random(200.0f) - 100.0f));

            // Store sprites to our own container for easy movement update iteration
            sprites_.Push(sprite);
        }
    }

    void MoveSprites(float timeStep)
    {
        Graphics* graphics = GetSubsystem<Graphics>();
        float width = (float)graphics->GetWidth();
        float height = (float)graphics->GetHeight();

        // Go through all sprites
        for (unsigned i = 0; i < sprites_.Size(); ++i)
        {
            Sprite* sprite = sprites_[i];

            // Rotate
            float newRot = sprite->GetRotation() + timeStep * 30.0f;
            sprite->SetRotation(newRot);

            // Move, wrap around rendering window edges
            Vector2 newPos = sprite->GetPosition() + sprite->GetVar(VAR_VELOCITY).GetVector2() * timeStep;
            if (newPos.x_ < 0.0f)
                newPos.x_ += width;
            if (newPos.x_ >= width)
                newPos.x_ -= width;
            if (newPos.y_ < 0.0f)
                newPos.y_ += height;
            if (newPos.y_ >= height)
                newPos.y_ -= height;
            sprite->SetPosition(newPos);
        }
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace Update;

        // Take the frame time step, which is stored as a float
        float timeStep = eventData[P_TIMESTEP].GetFloat();

        // Move sprites, scale movement with time step
        MoveSprites(timeStep);
    }

private:
    /// Vector to store the sprites for iterating through them.
    Vector<SharedPtr<Sprite> > sprites_;

};
URHO3D_DEFINE_APPLICATION_MAIN(MyApp)