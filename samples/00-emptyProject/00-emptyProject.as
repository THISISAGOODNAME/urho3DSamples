void Start()
{
    // Finally, hook-up this HelloWorld instance to handle update events
    SubscribeToEvents();
}

void SubscribeToEvents()
{
    // Subscribe key up event
    SubscribeToEvent("KeyUp", "HandleKeyUp");
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