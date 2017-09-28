// This first example, maintaining tradition, prints a "Hello World" message.
// Furthermore it shows:
//     - Using the Sample utility functions as a base for the application
//     - Adding a Text element to the graphical user interface
//     - Subscribing to and handling of update events


void Start()
{
    // Create "Hello World" Text
    CreateText();

    // Finally, hook-up this HelloWorld instance to handle update events
    SubscribeToEvents();
}

void CreateText()
{
    // Construct new Text object
    Text@ helloText = Text();

    // Set String to display
    helloText.text = "Hello World from Urho3D!";

    // Set font and text color
    helloText.SetFont(cache.GetResource("Font", "Fonts/Anonymous Pro.ttf"), 30);
    helloText.color = Color(0.0f, 1.0f, 0.0f);

    // Align Text center-screen
    helloText.horizontalAlignment = HA_CENTER;
    helloText.verticalAlignment = VA_CENTER;

    // Add Text instance to the UI root element
    ui.root.AddChild(helloText);
}

void SubscribeToEvents()
{
    // Subscribe key up event
    SubscribeToEvent("KeyUp", "HandleKeyUp");

    // Subscribe HandleUpdate() function for processing update events
    SubscribeToEvent("Update", "HandleUpdate");
}

void HandleUpdate(StringHash eventType, VariantMap& eventData)
{
    // Do nothing for now, could be extended to eg. animate the display
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