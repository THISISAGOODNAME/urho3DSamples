//
// Created by AICDG on 2017/9/28.
//

#include <Urho3D/Urho3DAll.h>

#include "Vehicle.h"

const float CAMERA_DISTANCE = 10.0f;

using namespace Urho3D;
class MyApp : public Application
{
public:
    MyApp(Context* context)
            : Application(context)
            , drawDebug_(false)
    {
        // Register factory and attributes for the Vehicle component so it can be created via CreateComponent, and loaded / saved
        Vehicle::RegisterObject(context);
    }
    virtual void Setup()
    {
        // Called before engine initialization. engineParameters_ member variable can be modified here
        engineParameters_[Urho3D::EP_WINDOW_TITLE] = "12 vehicle";
        engineParameters_[Urho3D::EP_FULL_SCREEN]  = false;
    }
    virtual void Start()
    {
        // Create the scene content
        CreateScene();

        // Create the UI content
        CreateInstructions();

        // Create the controllable vehicle
        CreateVehicle();

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

        scene_ = new Scene(context_);

        // Create scene subsystem components
        scene_->CreateComponent<Octree>();
        scene_->CreateComponent<PhysicsWorld>();

        // Create camera and define viewport. We will be doing load / save, so it's convenient to create the camera outside the scene,
        // so that it won't be destroyed and recreated, and we don't have to redefine the viewport on load
        cameraNode_ = new Node(context_);
        Camera* camera = cameraNode_->CreateComponent<Camera>();
        camera->SetFarClip(500.0f);
        GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));

        // Create static scene content. First create a zone for ambient lighting and fog control
        Node* zoneNode = scene_->CreateChild("Zone");
        Zone* zone = zoneNode->CreateComponent<Zone>();
        zone->SetAmbientColor(Color(0.15f, 0.15f, 0.15f));
        zone->SetFogColor(Color(0.5f, 0.5f, 0.7f));
        zone->SetFogStart(300.0f);
        zone->SetFogEnd(500.0f);
        zone->SetBoundingBox(BoundingBox(-2000.0f, 2000.0f));

        // Create a directional light with cascaded shadow mapping
        Node* lightNode = scene_->CreateChild("DirectionalLight");
        lightNode->SetDirection(Vector3(0.3f, -0.5f, 0.425f));
        Light* light = lightNode->CreateComponent<Light>();
        light->SetLightType(LIGHT_DIRECTIONAL);
        light->SetCastShadows(true);
        light->SetShadowBias(BiasParameters(0.00025f, 0.5f));
        light->SetShadowCascade(CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f));
        light->SetSpecularIntensity(0.5f);

        // Create heightmap terrain with collision
        Node* terrainNode = scene_->CreateChild("Terrain");
        terrainNode->SetPosition(Vector3::ZERO);
        Terrain* terrain = terrainNode->CreateComponent<Terrain>();
        terrain->SetPatchSize(64);
        terrain->SetSpacing(Vector3(2.0f, 0.1f, 2.0f)); // Spacing between vertices and vertical resolution of the height map
        terrain->SetSmoothing(true);
        terrain->SetHeightMap(cache->GetResource<Image>("Textures/HeightMap.png"));
        terrain->SetMaterial(cache->GetResource<Material>("Materials/Terrain.xml"));
        // The terrain consists of large triangles, which fits well for occlusion rendering, as a hill can occlude all
        // terrain patches and other objects behind it
        terrain->SetOccluder(true);

        RigidBody* body = terrainNode->CreateComponent<RigidBody>();
        body->SetCollisionLayer(2); // Use layer bitmask 2 for static geometry
        CollisionShape* shape = terrainNode->CreateComponent<CollisionShape>();
        shape->SetTerrain();

        // Create 1000 mushrooms in the terrain. Always face outward along the terrain normal
        const unsigned NUM_MUSHROOMS = 1000;
        for (unsigned i = 0; i < NUM_MUSHROOMS; ++i)
        {
            Node* objectNode = scene_->CreateChild("Mushroom");
            Vector3 position(Random(2000.0f) - 1000.0f, 0.0f, Random(2000.0f) - 1000.0f);
            position.y_ = terrain->GetHeight(position) - 0.1f;
            objectNode->SetPosition(position);
            // Create a rotation quaternion from up vector to terrain normal
            objectNode->SetRotation(Quaternion(Vector3::UP, terrain->GetNormal(position)));
            objectNode->SetScale(3.0f);
            StaticModel* object = objectNode->CreateComponent<StaticModel>();
            object->SetModel(cache->GetResource<Model>("Models/Mushroom.mdl"));
            object->SetMaterial(cache->GetResource<Material>("Materials/Mushroom.xml"));
            object->SetCastShadows(true);

            RigidBody* body = objectNode->CreateComponent<RigidBody>();
            body->SetCollisionLayer(2);
            CollisionShape* shape = objectNode->CreateComponent<CollisionShape>();
            shape->SetTriangleMesh(object->GetModel(), 0);
        }
    }

    void CreateInstructions()
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        UI* ui = GetSubsystem<UI>();

        // Construct new Text object, set string to display and font to use
        Text* instructionText = ui->GetRoot()->CreateChild<Text>();
        instructionText->SetText(
                "Use WASD keys to drive, mouse to rotate camera\n"
                "F5 to save scene, F7 to load"
        );
        instructionText->SetFont(cache->GetResource<Font>("Fonts/Anonymous Pro.ttf"), 15);

        // The text has multiple rows. Center them in relation to each other
        instructionText->SetTextAlignment(HA_CENTER);

        // Position the text relative to the screen center
        instructionText->SetHorizontalAlignment(HA_CENTER);
        instructionText->SetVerticalAlignment(VA_CENTER);
        instructionText->SetPosition(0, ui->GetRoot()->GetHeight() / 4);
    }

    void HandleUpdate(StringHash eventType, VariantMap& eventData)
    {
        using namespace Update;

        Input* input = GetSubsystem<Input>();

        if (vehicle_)
        {
            UI* ui = GetSubsystem<UI>();

            // Get movement controls and assign them to the vehicle component. If UI has a focused element, clear controls
            if (!ui->GetFocusElement())
            {
                vehicle_->controls_.Set(CTRL_FORWARD, input->GetKeyDown(KEY_W));
                vehicle_->controls_.Set(CTRL_BACK, input->GetKeyDown(KEY_S));
                vehicle_->controls_.Set(CTRL_LEFT, input->GetKeyDown(KEY_A));
                vehicle_->controls_.Set(CTRL_RIGHT, input->GetKeyDown(KEY_D));


                vehicle_->controls_.yaw_ += (float)input->GetMouseMoveX() * YAW_SENSITIVITY;
                vehicle_->controls_.pitch_ += (float)input->GetMouseMoveY() * YAW_SENSITIVITY;

                // Limit pitch
                vehicle_->controls_.pitch_ = Clamp(vehicle_->controls_.pitch_, 0.0f, 80.0f);

                // Check for loading / saving the scene
                if (input->GetKeyPress(KEY_F5))
                {
                    File saveFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/VehicleDemo.xml",
                                  FILE_WRITE);
                    scene_->SaveXML(saveFile);
                }
                if (input->GetKeyPress(KEY_F7))
                {
                    File loadFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/VehicleDemo.xml", FILE_READ);
                    scene_->LoadXML(loadFile);
                    // After loading we have to reacquire the weak pointer to the Vehicle component, as it has been recreated
                    // Simply find the vehicle's scene node by name as there's only one of them
                    Node* vehicleNode = scene_->GetChild("Vehicle", true);
                    if (vehicleNode)
                        vehicle_ = vehicleNode->GetComponent<Vehicle>();
                }
            }
            else
                vehicle_->controls_.Set(CTRL_FORWARD | CTRL_BACK | CTRL_LEFT | CTRL_RIGHT, false);
        }
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

        if (!vehicle_)
            return;

        Node* vehicleNode = vehicle_->GetNode();

        // Physics update has completed. Position camera behind vehicle
        Quaternion dir(vehicleNode->GetRotation().YawAngle(), Vector3::UP);
        dir = dir * Quaternion(vehicle_->controls_.yaw_, Vector3::UP);
        dir = dir * Quaternion(vehicle_->controls_.pitch_, Vector3::RIGHT);

        Vector3 cameraTargetPos = vehicleNode->GetPosition() - dir * Vector3(0.0f, 0.0f, CAMERA_DISTANCE);
        Vector3 cameraStartPos = vehicleNode->GetPosition();

        // Raycast camera against static objects (physics collision mask 2)
        // and move it closer to the vehicle if something in between
        Ray cameraRay(cameraStartPos, cameraTargetPos - cameraStartPos);
        float cameraRayLength = (cameraTargetPos - cameraStartPos).Length();
        PhysicsRaycastResult result;
        scene_->GetComponent<PhysicsWorld>()->RaycastSingle(result, cameraRay, cameraRayLength, 2);
        if (result.body_)
            cameraTargetPos = cameraStartPos + cameraRay.direction_ * (result.distance_ - 0.5f);

        cameraNode_->SetPosition(cameraTargetPos);
        cameraNode_->SetRotation(dir);
    }

    void CreateVehicle()
    {
        Node* vehicleNode = scene_->CreateChild("Vehicle");
        vehicleNode->SetPosition(Vector3(0.0f, 5.0f, 0.0f));

        // Create the vehicle logic component
        vehicle_ = vehicleNode->CreateComponent<Vehicle>();
        // Create the rendering and physics components
        vehicle_->Init();
    }

private:
    /// Scene.
    SharedPtr<Scene> scene_;
    /// Camera scene node.
    SharedPtr<Node> cameraNode_;
    /// Camera yaw angle.
    float yaw_;
    /// Camera pitch angle.
    float pitch_;

    /// Flag for drawing debug geometry.
    bool drawDebug_;

    /// The controllable vehicle component.
    WeakPtr<Vehicle> vehicle_;
};
URHO3D_DEFINE_APPLICATION_MAIN(MyApp)