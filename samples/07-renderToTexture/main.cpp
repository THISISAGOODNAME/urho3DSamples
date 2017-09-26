//
// Created by AICDG on 2017/9/26.
//

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Graphics/Animation.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationState.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/DebugRenderer.h>
#include <Urho3D/Graphics/Graphics.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Model.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/RenderSurface.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Graphics/Technique.h>
#include <Urho3D/Graphics/Texture2D.h>
#include <Urho3D/Graphics/StaticModel.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Input/InputEvents.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>

#include "Mover.h"

using namespace Urho3D;
class MyApp : public Application
{
public:
    MyApp(Context* context)
            : Application(context)
            , drawDebug_(false)
    {
        // Register an object factory for our custom Mover component so that we can create them to scene nodes
        context->RegisterFactory<Mover>();
    }
    virtual void Setup()
    {
        // Called before engine initialization. engineParameters_ member variable can be modified here
        engineParameters_[Urho3D::EP_WINDOW_TITLE] = "07 render to texture";
        engineParameters_[Urho3D::EP_FULL_SCREEN]  = false;
    }
    virtual void Start()
    {
        // Create the scene content
        CreateScene();

        // Create the UI content
        CreateInstructions();

        // Setup the viewport for displaying the scene
        SetupViewport();

        // Hook up to the frame update and render post-update events
        SubscribeToEvents();

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

    void CreateScene()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();

        {
            // Create the scene which will be rendered to a texture
            rttScene_ = new Scene(context_);

            // Create octree, use default volume (-1000, -1000, -1000) to (1000, 1000, 1000)
            // Also create a DebugRenderer component so that we can draw debug geometry
            rttScene_->CreateComponent<Octree>();
            rttScene_->CreateComponent<DebugRenderer>();

            // Create scene node & StaticModel component for showing a static plane
            Node* planeNode = rttScene_->CreateChild("Plane");
            planeNode->SetScale(Vector3(50.0f, 1.0f, 50.0f));
            StaticModel* planeObject = planeNode->CreateComponent<StaticModel>();
            planeObject->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));
            planeObject->SetMaterial(cache->GetResource<Material>("Materials/StoneTiled.xml"));

            // Create a Zone component for ambient lighting & fog control
            Node* zoneNode = rttScene_->CreateChild("Zone");
            Zone* zone = zoneNode->CreateComponent<Zone>();
            zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
            zone->SetAmbientColor(Color(0.5f, 0.5f, 0.5f));
            zone->SetFogColor(Color(0.4f, 0.5f, 0.8f));
            zone->SetFogStart(100.0f);
            zone->SetFogEnd(300.0f);

            // Create a directional light to the world. Enable cascaded shadows on it
            Node* lightNode = rttScene_->CreateChild("DirectionalLight");
            lightNode->SetDirection(Vector3(0.6f, -1.0f, 0.8f));
            Light* light = lightNode->CreateComponent<Light>();
            light->SetLightType(LIGHT_DIRECTIONAL);
            light->SetCastShadows(true);
            light->SetColor(Color(0.5f, 0.5f, 0.5f));
            light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
            // Set cascade splits at 10, 50 and 200 world units, fade shadows out at 80% of maximum shadow distance
            light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));

            // Create animated models
            const unsigned NUM_MODELS = 30;
            const float MODEL_MOVE_SPEED = 2.0f;
            const float MODEL_ROTATE_SPEED = 100.0f;
            const BoundingBox bounds(Vector3(-20.0f, 0.0f, -20.0f), Vector3(20.0f, 0.0f, 20.0f));

            for (unsigned i = 0; i < NUM_MODELS; ++i)
            {
                Node* modelNode = rttScene_->CreateChild("Jill");
                modelNode->SetPosition(Vector3(Random(40.0f) - 20.0f, 0.0f, Random(40.0f) - 20.0f));
                modelNode->SetRotation(Quaternion(0.0f, Random(360.0f), 0.0f));

                AnimatedModel* modelObject = modelNode->CreateComponent<AnimatedModel>();
                modelObject->SetModel(cache->GetResource<Model>("Models/Kachujin/Kachujin.mdl"));
                modelObject->SetMaterial(cache->GetResource<Material>("Models/Kachujin/Materials/Kachujin.xml"));
                modelObject->SetCastShadows(true);

                // Create an AnimationState for a walk animation. Its time position will need to be manually updated to advance the
                // animation, The alternative would be to use an AnimationController component which updates the animation automatically,
                // but we need to update the model's position manually in any case
                Animation* walkAnimation = cache->GetResource<Animation>("Models/Kachujin/Kachujin_Walk.ani");

                AnimationState* state = modelObject->AddAnimationState(walkAnimation);
                // The state would fail to create (return null) if the animation was not found
                if (state)
                {
                    // Enable full blending weight and looping
                    state->SetWeight(1.0f);
                    state->SetLooped(true);
                    state->SetTime(Random(walkAnimation->GetLength()));
                }

                // Create our custom Mover component that will move & animate the model during each frame's update
                Mover* mover = modelNode->CreateComponent<Mover>();
                mover->SetParameters(MODEL_MOVE_SPEED, MODEL_ROTATE_SPEED, bounds);
            }

            // Create the camera. Limit far clip distance to match the fog
            rttCameraNode_ = rttScene_->CreateChild("Camera");
            Camera* camera = rttCameraNode_->CreateComponent<Camera>();
            camera->SetFarClip(300.0f);

            // Set an initial position for the camera scene node above the plane
            rttCameraNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
        }

        {
            // Create the scene in which we move around
            scene_ = new Scene(context_);

            // Create octree, use also default volume (-1000, -1000, -1000) to (1000, 1000, 1000)
            scene_->CreateComponent<Octree>();

            // Create a Zone component for ambient lighting & fog control
            Node *zoneNode = scene_->CreateChild("Zone");
            Zone *zone = zoneNode->CreateComponent<Zone>();
            zone->SetBoundingBox(BoundingBox(-1000.0f, 1000.0f));
            zone->SetAmbientColor(Color(0.1f, 0.1f, 0.1f));
            zone->SetFogStart(100.0f);
            zone->SetFogEnd(300.0f);

            // Create a directional light without shadows
            Node *lightNode = scene_->CreateChild("DirectionalLight");
            lightNode->SetDirection(Vector3(0.5f, -1.0f, 0.5f));
            Light *light = lightNode->CreateComponent<Light>();
            light->SetLightType(LIGHT_DIRECTIONAL);
            light->SetColor(Color(0.2f, 0.2f, 0.2f));
            light->SetSpecularIntensity(1.0f);

            // Create a "floor" consisting of several tiles
            for (int y = -5; y <= 5; ++y) {
                for (int x = -5; x <= 5; ++x) {
                    Node *floorNode = scene_->CreateChild("FloorTile");
                    floorNode->SetPosition(Vector3(x * 20.5f, -0.5f, y * 20.5f));
                    floorNode->SetScale(Vector3(20.0f, 1.0f, 20.f));
                    StaticModel *floorObject = floorNode->CreateComponent<StaticModel>();
                    floorObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
                    floorObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));
                }
            }

            // Create a "screen" like object for viewing the second scene. Construct it from two StaticModels, a box for the frame
            // and a plane for the actual view
            {
                Node *boxNode = scene_->CreateChild("ScreenBox");
                boxNode->SetPosition(Vector3(0.0f, 10.0f, 0.0f));
                boxNode->SetScale(Vector3(21.0f, 16.0f, 0.5f));
                StaticModel *boxObject = boxNode->CreateComponent<StaticModel>();
                boxObject->SetModel(cache->GetResource<Model>("Models/Box.mdl"));
                boxObject->SetMaterial(cache->GetResource<Material>("Materials/Stone.xml"));

                Node *screenNode = scene_->CreateChild("Screen");
                screenNode->SetPosition(Vector3(0.0f, 10.0f, -0.27f));
                screenNode->SetRotation(Quaternion(-90.0f, 0.0f, 0.0f));
                screenNode->SetScale(Vector3(20.0f, 0.0f, 15.0f));
                StaticModel *screenObject = screenNode->CreateComponent<StaticModel>();
                screenObject->SetModel(cache->GetResource<Model>("Models/Plane.mdl"));

                // Create a renderable texture (1024x768, RGB format), enable bilinear filtering on it
                SharedPtr<Texture2D> renderTexture(new Texture2D(context_));
                renderTexture->SetSize(1024, 768, Graphics::GetRGBFormat(), TEXTURE_RENDERTARGET);
                renderTexture->SetFilterMode(FILTER_BILINEAR);

                // Create a new material from scratch, use the diffuse unlit technique, assign the render texture
                // as its diffuse texture, then assign the material to the screen plane object
                SharedPtr<Material> renderMaterial(new Material(context_));
                renderMaterial->SetTechnique(0, cache->GetResource<Technique>("Techniques/DiffUnlit.xml"));
                renderMaterial->SetTexture(TU_DIFFUSE, renderTexture);
                // Since the screen material is on top of the box model and may Z-fight, use negative depth bias
                // to push it forward (particularly necessary on mobiles with possibly less Z resolution)
                renderMaterial->SetDepthBias(BiasParameters(-0.001f, 0.0f));
                screenObject->SetMaterial(renderMaterial);

                // Get the texture's RenderSurface object (exists when the texture has been created in rendertarget mode)
                // and define the viewport for rendering the second scene, similarly as how backbuffer viewports are defined
                // to the Renderer subsystem. By default the texture viewport will be updated when the texture is visible
                // in the main view
                RenderSurface *surface = renderTexture->GetRenderSurface();
                SharedPtr<Viewport> rttViewport(
                        new Viewport(context_, rttScene_, rttCameraNode_->GetComponent<Camera>()));
                surface->SetViewport(0, rttViewport);
            }

            // Create the camera which we will move around. Limit far clip distance to match the fog
            cameraNode_ = scene_->CreateChild("Camera");
            Camera *camera = cameraNode_->CreateComponent<Camera>();
            camera->SetFarClip(300.0f);

            // Set an initial position for the camera scene node above the plane
            cameraNode_->SetPosition(Vector3(0.0f, 7.0f, -30.0f));
        }

    }

    void CreateInstructions()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        UI* ui = GetSubsystem<UI>();

        // Construct new Text object, set string to display and font to use
        Text* instructionText = ui->GetRoot()->CreateChild<Text>();
        instructionText->SetText(
                "Use WASD keys and mouse to move\n"
                "Space to toggle debug geometry"
        );
        instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);

        // Position the text relative to the screen center
        instructionText->SetHorizontalAlignment(HA_CENTER);
        instructionText->SetVerticalAlignment(VA_CENTER);
        instructionText->SetPosition(0, ui->GetRoot()->GetHeight() / 4);
    }

    void SetupViewport()
    {
        Renderer* renderer = GetSubsystem<Renderer>();

        // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen. We need to define the scene and the camera
        // at minimum. Additionally we could configure the viewport screen size and the rendering path (eg. forward / deferred) to
        // use, but now we just use full screen and default render path configured in the engine command line options
        SharedPtr<Viewport> viewport(new Viewport(context_, scene_, cameraNode_->GetComponent<Camera>()));
        renderer->SetViewport(0, viewport);
    }

    void MoveCamera(float timeStep)
    {
        // Do not move if the UI has a focused element (the console)
        if (GetSubsystem<UI>()->GetFocusElement())
            return;

        Input* input = GetSubsystem<Input>();

        // Movement speed as world units per second
        const float MOVE_SPEED = 20.0f;
        // Mouse sensitivity as degrees per pixel
        const float MOUSE_SENSITIVITY = 0.1f;

        // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
        IntVector2 mouseMove = input->GetMouseMove();
        yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
        pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
        pitch_ = Clamp(pitch_, -90.0f, 90.0f);

        // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
        cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

        // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
        // Use the Translate() function (default local space) to move relative to the node's orientation.
        if (input->GetKeyDown(KEY_W))
            cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
        if (input->GetKeyDown(KEY_S))
            cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
        if (input->GetKeyDown(KEY_A))
            cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
        if (input->GetKeyDown(KEY_D))
            cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);

        // Toggle debug geometry with space
        if (input->GetKeyPress(KEY_SPACE))
            drawDebug_ = !drawDebug_;
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace Update;

        // Take the frame time step, which is stored as a float
        float timeStep = eventData[P_TIMESTEP].GetFloat();

        // Move the camera, scale movement with time step
        MoveCamera(timeStep);
    }

    void SubscribeToEvents()
    {
        // Called after engine initialization. Setup application & subscribe to events here
        SubscribeToEvent(E_KEYDOWN, URHO3D_HANDLER(MyApp, HandleKeyDown));
        SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(MyApp, HandleUpdate));

        // Subscribe HandlePostRenderUpdate() function for processing the post-render update event, sent after Renderer subsystem is
        // done with defining the draw calls for the viewports (but before actually executing them.) We will request debug geometry
        // rendering during that event
        SubscribeToEvent(E_POSTRENDERUPDATE, URHO3D_HANDLER(MyApp, HandlePostRenderUpdate));
    }

    void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
    {
        // If draw debug mode is enabled, draw viewport debug geometry, which will show eg. drawable bounding boxes and skeleton
        // bones. Note that debug geometry has to be separately requested each frame. Disable depth test so that we can see the
        // bones properly
        if (drawDebug_)
            GetSubsystem<Renderer>()->DrawDebugGeometry(false);
    }

private:
    /// Scene.
    SharedPtr<Scene> rttScene_;
    SharedPtr<Scene> scene_;
    /// Camera scene node.
    SharedPtr<Node> cameraNode_;
    SharedPtr<Node> rttCameraNode_;
    /// Camera yaw angle.
    float yaw_;
    /// Camera pitch angle.
    float pitch_;

    /// Flag for drawing debug geometry.
    bool drawDebug_;
};
URHO3D_DEFINE_APPLICATION_MAIN(MyApp)