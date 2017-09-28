Window@ window;

void Start()
{
    // Enable OS cursor
    input.mouseVisible = true;

    // Load XML file containing default UI style sheet
    XMLFile@ style = cache.GetResource("XMLFile", "UI/DefaultStyle.xml");

    // Set the loaded style as default style
    ui.root.defaultStyle = style;

    // Initialize Window
    InitWindow();

    // Create and add some controls to the Window
    InitControls();

    // Finally, hook-up this HelloWorld instance to handle update events
    SubscribeToEvents();
}

void InitControls()
{
    // Create a CheckBox
    CheckBox@ checkBox = CheckBox();
    checkBox.name = "CheckBox";

    // Create a Button
    Button@ button = Button();
    button.name = "Button";
    button.minHeight = 24;

    // Create a LineEdit
    LineEdit@ lineEdit = LineEdit();
    lineEdit.name = "LineEdit";
    lineEdit.minHeight = 24;

    // Add controls to Window
    window.AddChild(checkBox);
    window.AddChild(button);
    window.AddChild(lineEdit);

    // Apply previously set default style
    checkBox.SetStyleAuto();
    button.SetStyleAuto();
    lineEdit.SetStyleAuto();
}

void InitWindow()
{
    // Create the Window and add it to the UI's root node
    window = Window();
    ui.root.AddChild(window);

    // Set Window size and layout settings
    window.minWidth = 384;
    window.SetLayout(LM_VERTICAL, 6, IntRect(6, 6, 6, 6));
    window.SetAlignment(HA_CENTER, VA_CENTER);
    window.name = "Window";

    // Create Window 'titlebar' container
    UIElement@ titleBar = UIElement();
    titleBar.SetMinSize(0, 24);
    titleBar.verticalAlignment = VA_TOP;
    titleBar.layoutMode = LM_HORIZONTAL;

    // Create the Window title Text
    Text@ windowTitle = Text();
    windowTitle.name = "WindowTitle";
    windowTitle.text = "Hello GUI!";

    // Create the Window's close button
    Button@ buttonClose = Button();
    buttonClose.name = "CloseButton";

    // Add the controls to the title bar
    titleBar.AddChild(windowTitle);
    titleBar.AddChild(buttonClose);

    // Add the title bar to the Window
    window.AddChild(titleBar);

    // Apply styles
    window.SetStyleAuto();
    windowTitle.SetStyleAuto();
    buttonClose.style = "CloseButton";

    // Subscribe to buttonClose release (following a 'press') events
    SubscribeToEvent(buttonClose, "Released", "HandleClosePressed");

    // Subscribe also to all UI mouse clicks just to see where we have clicked
    SubscribeToEvent("UIMouseClick", "HandleControlClicked");
}

void HandleClosePressed(StringHash eventType, VariantMap& eventData)
{
    engine.Exit();
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

void HandleControlClicked(StringHash eventType, VariantMap& eventData)
{
    // Get the Text control acting as the Window's title
    Text@ windowTitle = window.GetChild("WindowTitle", true);

    // Get control that was clicked
    UIElement@ clicked = eventData["Element"].GetPtr();

    String name = "...?";
    if (clicked !is null)
    {
        // Get the name of the control that was clicked
        name = clicked.name;
    }

    // Update the Window's title text
    windowTitle.text = "Hello " + name + "!";
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