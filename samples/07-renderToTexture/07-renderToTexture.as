Scene@ scene_;
Node@ cameraNode; // Camera scene node
float yaw = 0.0f; // Camera yaw angle
float pitch = 0.0f; // Camera pitch angle
bool drawDebug = false; // Draw debug geometry flag

Scene@ rttScene_;
Node@ rttCameraNode;

void Start()
{
  // Create "Hello World" Text
  CreateText();

  // Create the UI content
  CreateInstructions();

  // Setup the viewport for displaying the scene
  SetupViewport();

  // Finally, hook-up this HelloWorld instance to handle update events
  SubscribeToEvents();
}

void CreateText()
{
  {
    rttScene_ = Scene();

    // Create octree, use default volume (-1000, -1000, -1000) to (1000, 1000, 1000)
    // Also create a DebugRenderer component so that we can draw debug geometry
    rttScene_.CreateComponent("Octree");
    rttScene_.CreateComponent("DebugRenderer");

    // Create scene node & StaticModel component for showing a static plane
    Node@ planeNode = rttScene_.CreateChild("Plane");
    planeNode.scale = Vector3(50.0f, 1.0f, 50.0f);
    StaticModel@ planeObject = planeNode.CreateComponent("StaticModel");
    planeObject.model = cache.GetResource("Model", "Models/Plane.mdl");
    planeObject.material = cache.GetResource("Material", "Materials/StoneTiled.xml");

    // Create a Zone component for ambient lighting & fog control
    Node@ zoneNode = rttScene_.CreateChild("Zone");
    Zone@ zone = zoneNode.CreateComponent("Zone");
    zone.boundingBox = BoundingBox(-1000.0f, 1000.0f);
    zone.ambientColor = Color(0.5f, 0.5f, 0.5f);
    zone.fogColor = Color(0.4f, 0.5f, 0.8f);
    zone.fogStart = 100.0f;
    zone.fogEnd = 300.0f;

    // Create a directional light to the world. Enable cascaded shadows on it
    Node@ lightNode = rttScene_.CreateChild("DirectionalLight");
    lightNode.direction = Vector3(0.6f, -1.0f, 0.8f);
    Light@ light = lightNode.CreateComponent("Light");
    light.lightType = LIGHT_DIRECTIONAL;
    light.color = Color(0.5f, 0.5f, 0.5f);
    light.castShadows = true;
    light.shadowBias = BiasParameters(0.00025f, 0.5f);
    // Set cascade splits at 10, 50 and 200 world units, fade shadows out at 80% of maximum shadow distance
    light.shadowCascade = CascadeParameters(10.0f, 50.0f, 200.0f, 0.0f, 0.8f);

    // Create animated models
    const uint NUM_MODELS = 30;
    const float MODEL_MOVE_SPEED = 2.0f;
    const float MODEL_ROTATE_SPEED = 100.0f;
    const BoundingBox bounds(Vector3(-20.0f, 0.0f, -20.0f), Vector3(20.0f, 0.0f, 20.0f));

    for (uint i = 0; i < NUM_MODELS; ++i)
    {
      Node@ modelNode = rttScene_.CreateChild("Jill");
      modelNode.position = Vector3(Random(40.0f) - 20.0f, 0.0f, Random(40.0f) - 20.0f);
      modelNode.rotation = Quaternion(0.0f, Random(360.0f), 0.0f);

      AnimatedModel@ modelObject = modelNode.CreateComponent("AnimatedModel");
      modelObject.model = cache.GetResource("Model", "Models/Kachujin/Kachujin.mdl");
      modelObject.material = cache.GetResource("Material", "Models/Kachujin/Materials/Kachujin.xml");
      modelObject.castShadows = true;

      // Create an AnimationState for a walk animation. Its time position will need to be manually updated to advance the
      // animation, The alternative would be to use an AnimationController component which updates the animation automatically,
      // but we need to update the model's position manually in any case
      Animation@ walkAnimation = cache.GetResource("Animation", "Models/Kachujin/Kachujin_Walk.ani");
      AnimationState@ state = modelObject.AddAnimationState(walkAnimation);
      // Enable full blending weight and looping
      state.weight = 1.0f;
      state.looped = true;
      state.time = Random(walkAnimation.length);

      // Create our Mover script object that will move & animate the model during each frame's update. Here we use a shortcut
      // script-only API function, CreateScriptObject, which creates a ScriptInstance component into the scene node, then uses
      // it to instantiate the object (using the script file & class name provided)
      Mover@ mover = cast<Mover>(modelNode.CreateScriptObject(scriptFile, "Mover"));
      mover.SetParameters(MODEL_MOVE_SPEED, MODEL_ROTATE_SPEED, bounds);
    }

    // Create the camera. Limit far clip distance to match the fog
    rttCameraNode = rttScene_.CreateChild("Camera");
    Camera@ camera = rttCameraNode.CreateComponent("Camera");
    camera.farClip = 300.0f;

    // Set an initial position for the camera scene node above the plane
    rttCameraNode.position = Vector3(0.0f, 5.0f, 0.0f);
  }

  {
    // Create the scene in which we move around
    scene_ = Scene();

    // Create octree, use also default volume (-1000, -1000, -1000) to (1000, 1000, 1000)
    scene_.CreateComponent("Octree");

    // Create a Zone component for ambient lighting & fog control
    Node@ zoneNode = scene_.CreateChild("Zone");
    Zone@ zone = zoneNode.CreateComponent("Zone");
    zone.boundingBox = BoundingBox(-1000.0f, 1000.0f);
    zone.ambientColor = Color(0.1f, 0.1f, 0.1f);
    zone.fogStart = 100.0f;
    zone.fogEnd = 300.0f;

    // Create a directional light without shadows
    Node@ lightNode = scene_.CreateChild("DirectionalLight");
    lightNode.direction = Vector3(0.5f, -1.0f, 0.5f);
    Light@ light = lightNode.CreateComponent("Light");
    light.lightType = LIGHT_DIRECTIONAL;
    light.color = Color(0.2f, 0.2f, 0.2f);
    light.specularIntensity = 1.0f;

    // Create a "floor" consisting of several tiles
    for (int y = -5; y <= 5; ++y)
    {
      for (int x = -5; x <= 5; ++x)
      {
        Node@ floorNode = scene_.CreateChild("FloorTile");
        floorNode.position = Vector3(x * 20.5f, -0.5f, y * 20.5f);
        floorNode.scale = Vector3(20.0f, 1.0f, 20.f);
        StaticModel@ floorObject = floorNode.CreateComponent("StaticModel");
        floorObject.model = cache.GetResource("Model", "Models/Box.mdl");
        floorObject.material = cache.GetResource("Material", "Materials/Stone.xml");
      }
    }

    // Create a "screen" like object for viewing the second scene. Construct it from two StaticModels, a box for the frame
    // and a plane for the actual view
    {
      Node@ boxNode = scene_.CreateChild("ScreenBox");
      boxNode.position = Vector3(0.0f, 10.0f, 0.0f);
      boxNode.scale = Vector3(21.0f, 16.0f, 0.5f);
      StaticModel@ boxObject = boxNode.CreateComponent("StaticModel");
      boxObject.model = cache.GetResource("Model", "Models/Box.mdl");
      boxObject.material = cache.GetResource("Material", "Materials/Stone.xml");

      Node@ screenNode = scene_.CreateChild("Screen");
      screenNode.position = Vector3(0.0f, 10.0f, -0.27f);
      screenNode.rotation = Quaternion(-90.0f, 0.0f, 0.0f);
      screenNode.scale = Vector3(20.0f, 0.0f, 15.0f);
      StaticModel@ screenObject = screenNode.CreateComponent("StaticModel");
      screenObject.model = cache.GetResource("Model", "Models/Plane.mdl");

      // Create a renderable texture (1024x768, RGB format), enable bilinear filtering on it
      Texture2D@ renderTexture = Texture2D();
      renderTexture.SetSize(1024, 768, GetRGBFormat(), TEXTURE_RENDERTARGET);
      renderTexture.filterMode = FILTER_BILINEAR;

      // Create a new material from scratch, use the diffuse unlit technique, assign the render texture
      // as its diffuse texture, then assign the material to the screen plane object
      Material@ renderMaterial = Material();
      renderMaterial.SetTechnique(0, cache.GetResource("Technique", "Techniques/DiffUnlit.xml"));
      renderMaterial.textures[TU_DIFFUSE] = renderTexture;
      // Since the screen material is on top of the box model and may Z-fight, use negative depth bias
      // to push it forward (particularly necessary on mobiles with possibly less Z resolution)
      renderMaterial.depthBias = BiasParameters(-0.001, 0.0);
      screenObject.material = renderMaterial;

      // Get the texture's RenderSurface object (exists when the texture has been created in rendertarget mode)
      // and define the viewport for rendering the second scene, similarly as how backbuffer viewports are defined
      // to the Renderer subsystem. By default the texture viewport will be updated when the texture is visible
      // in the main view
      RenderSurface@ surface = renderTexture.renderSurface;
      Viewport@ rttViewport = Viewport(rttScene_, rttCameraNode.GetComponent("Camera"));
      surface.viewports[0] = rttViewport;
    }

    // Create the camera which we will move around. Limit far clip distance to match the fog
    cameraNode = scene_.CreateChild("Camera");
    Camera@ camera = cameraNode.CreateComponent("Camera");
    camera.farClip = 300.0f;

    // Set an initial position for the camera scene node above the plane
    cameraNode.position = Vector3(0.0f, 7.0f, -30.0f);
  }
}

void SubscribeToEvents()
{
  // Subscribe key up event
  SubscribeToEvent("KeyUp", "HandleKeyUp");

  // Subscribe HandleUpdate() function for processing update events
  SubscribeToEvent("Update", "HandleUpdate");

  // Subscribe HandlePostRenderUpdate() function for processing the post-render update event, sent after Renderer subsystem is
  // done with defining the draw calls for the viewports (but before actually executing them.) We will request debug geometry
  // rendering during that event
  SubscribeToEvent("PostRenderUpdate", "HandlePostRenderUpdate");
}

void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
  // Take the frame time step, which is stored as a float
  float timeStep = eventData["TimeStep"].GetFloat();

  // Move the camera, scale movement with time step
  MoveCamera(timeStep);
}

void HandleKeyUp(StringHash eventType, VariantMap& eventData)
{
  int key = eventData["Key"].GetInt();

  // Close console (if open) or exit when ESC is pressed
  if (key == KEY_ESCAPE)
  {
    engine.Exit();
  }
}

void SetupViewport()
{
  // Set up a viewport to the Renderer subsystem so that the 3D scene can be seen. We need to define the scene and the camera
  // at minimum. Additionally we could configure the viewport screen size and the rendering path (eg. forward / deferred) to
  // use, but now we just use full screen and default render path configured in the engine command line options
  Viewport@ viewport = Viewport(scene_, cameraNode.GetComponent("Camera"));
  renderer.viewports[0] = viewport;
}

void MoveCamera(float timeStep)
{
  // Do not move if the UI has a focused element (the console)
  if (ui.focusElement !is null)
  return;

  // Movement speed as world units per second
  const float MOVE_SPEED = 20.0f;
  // Mouse sensitivity as degrees per pixel
  const float MOUSE_SENSITIVITY = 0.1f;

  // Use this frame's mouse motion to adjust camera node yaw and pitch. Clamp the pitch between -90 and 90 degrees
  IntVector2 mouseMove = input.mouseMove;
  yaw += MOUSE_SENSITIVITY * mouseMove.x;
  pitch += MOUSE_SENSITIVITY * mouseMove.y;
  pitch = Clamp(pitch, -90.0f, 90.0f);

  // Construct new orientation for the camera scene node from yaw and pitch. Roll is fixed to zero
  cameraNode.rotation = Quaternion(pitch, yaw, 0.0f);

  // Read WASD keys and move the camera scene node to the corresponding direction if they are pressed
  // Use the Translate() function (default local space) to move relative to the node's orientation.
  if (input.keyDown[KEY_W])
    cameraNode.Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
  if (input.keyDown[KEY_S])
    cameraNode.Translate(Vector3::BACK * MOVE_SPEED * timeStep);
  if (input.keyDown[KEY_A])
    cameraNode.Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
  if (input.keyDown[KEY_D])
    cameraNode.Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);

  // Toggle debug geometry with space
  if (input.keyPress[KEY_SPACE])
    drawDebug = !drawDebug;
}

void CreateInstructions()
{
  // Construct new Text object, set string to display and font to use
  Text@ instructionText = ui.root.CreateChild("Text");
  instructionText.text =
    "Use WASD keys and mouse to move\n"
  "Space to toggle debug geometry";
  instructionText.SetFont(cache.GetResource("Font", "Fonts/Anonymous Pro.ttf"), 15);
  // The text has multiple rows. Center them in relation to each other
  instructionText.textAlignment = HA_CENTER;

  // Position the text relative to the screen center
  instructionText.horizontalAlignment = HA_CENTER;
  instructionText.verticalAlignment = VA_CENTER;
  instructionText.SetPosition(0, ui.root.height / 4);
}

void HandlePostRenderUpdate(StringHash eventType, VariantMap& eventData)
{
  // If draw debug mode is enabled, draw viewport debug geometry, which will show eg. drawable bounding boxes and skeleton
  // bones. Note that debug geometry has to be separately requested each frame. Disable depth test so that we can see the
  // bones properly
  if (drawDebug)
    renderer.DrawDebugGeometry(false);
}

// Mover script object class
class Mover : ScriptObject
{
  float moveSpeed = 0.0f;
  float rotationSpeed = 0.0f;
  BoundingBox bounds;

  void SetParameters(float moveSpeed_, float rotationSpeed_, const BoundingBox& bounds_)
  {
    moveSpeed = moveSpeed_;
    rotationSpeed = rotationSpeed_;
    bounds = bounds_;
  }

  void Update(float timeStep)
  {
    node.Translate(Vector3::FORWARD * moveSpeed * timeStep);

    // If in risk of going outside the plane, rotate the model right
    Vector3 pos = node.position;
    if (pos.x < bounds.min.x || pos.x > bounds.max.x || pos.z < bounds.min.z || pos.z > bounds.max.z)
      node.Yaw(rotationSpeed * timeStep);

    // Get the model's first (only) animation state and advance its time
    AnimatedModel@ model = node.GetComponent("AnimatedModel", true);
    AnimationState@ state = model.GetAnimationState(0);
    if (state !is null)
    state.AddTime(timeStep);
  }
}