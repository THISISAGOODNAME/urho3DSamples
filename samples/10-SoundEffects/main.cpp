//
// Created by AICDG on 2017/9/26.
//

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Audio/Audio.h>
#include <Urho3D/Audio/AudioEvents.h>
#include <Urho3D/Audio/Sound.h>
#include <Urho3D/Audio/SoundSource.h>
#include <Urho3D/Engine/Application.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Engine/EngineDefs.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/IO/Log.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Button.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Slider.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/UI/UIEvents.h>

using namespace Urho3D;

// Custom variable identifier for storing sound effect name within the UI element
static const StringHash VAR_SOUNDRESOURCE("SoundResource");
static const unsigned NUM_SOUNDS = 3;

static String soundNames[] = {
        "Fist",
        "Explosion",
        "Power-up"
};

static String soundResourceNames[] = {
        "Sounds/PlayerFistHit.wav",
        "Sounds/BigExplosion.wav",
        "Sounds/Powerup.wav"
};

class MyApp : public Application
{
    URHO3D_OBJECT(MyApp, Application);

public:
    MyApp(Context* context) :
            Application(context)
    {
    }
    virtual void Setup()
    {
        // Called before engine initialization. engineParameters_ member variable can be modified here
        engineParameters_[Urho3D::EP_WINDOW_TITLE] = "10 sound effects";
        engineParameters_[Urho3D::EP_FULL_SCREEN]  = false;
        engineParameters_[EP_SOUND] = true;
    }
    virtual void Start()
    {
        // Create a scene which will not be actually rendered, but is used to hold SoundSource components while they play sounds
        scene_ = new Scene(context_);

        // Create music sound source
        musicSource_ = scene_->CreateComponent<SoundSource>();
        // Set the sound type to music so that master volume control works correctly
        musicSource_->SetSoundType(SOUND_MUSIC);

        // Enable OS cursor
        GetSubsystem<Input>()->SetMouseVisible(true);

        // Create the user interface
        CreateUI();

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

    void CreateUI()
    {
        UIElement* root = GetSubsystem<UI>()->GetRoot();
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        XMLFile* uiStyle = cache->GetResource<XMLFile>("UI/DefaultStyle.xml");
        // Set style to the UI root so that elements will inherit it
        root->SetDefaultStyle(uiStyle);

        // Create buttons for playing back sounds
        for (unsigned i = 0; i < NUM_SOUNDS; ++i)
        {
            Button* button = CreateButton(i * 140 + 20, 20, 120, 40, soundNames[i]);
            // Store the sound effect resource name as a custom variable into the button
            button->SetVar(VAR_SOUNDRESOURCE, soundResourceNames[i]);
            SubscribeToEvent(button, E_PRESSED, URHO3D_HANDLER(MyApp, HandlePlaySound));
        }

        // Create buttons for playing/stopping music
        Button* button = CreateButton(20, 80, 120, 40, "Play Music");
        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(MyApp, HandlePlayMusic));

        button = CreateButton(160, 80, 120, 40, "Stop Music");
        SubscribeToEvent(button, E_RELEASED, URHO3D_HANDLER(MyApp, HandleStopMusic));

        Audio* audio = GetSubsystem<Audio>();

        // Create sliders for controlling sound and music master volume
        Slider* slider = CreateSlider(20, 140, 200, 20, "Sound Volume");
        slider->SetValue(audio->GetMasterGain(SOUND_EFFECT));
        SubscribeToEvent(slider, E_SLIDERCHANGED, URHO3D_HANDLER(MyApp, HandleSoundVolume));

        slider = CreateSlider(20, 200, 200, 20, "Music Volume");
        slider->SetValue(audio->GetMasterGain(SOUND_MUSIC));
        SubscribeToEvent(slider, E_SLIDERCHANGED, URHO3D_HANDLER(MyApp, HandleMusicVolume));
    }

    Button* CreateButton(int x, int y, int xSize, int ySize, const String& text)
    {
        UIElement* root = GetSubsystem<UI>()->GetRoot();
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        Font* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

        // Create the button and center the text onto it
        Button* button = root->CreateChild<Button>();
        button->SetStyleAuto();
        button->SetPosition(x, y);
        button->SetSize(xSize, ySize);

        Text* buttonText = button->CreateChild<Text>();
        buttonText->SetAlignment(HA_CENTER, VA_CENTER);
        buttonText->SetFont(font, 12);
        buttonText->SetText(text);

        return button;
    }

    Slider* CreateSlider(int x, int y, int xSize, int ySize, const String& text)
    {
        UIElement* root = GetSubsystem<UI>()->GetRoot();
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        Font* font = cache->GetResource<Font>("Fonts/Anonymous Pro.ttf");

        // Create text and slider below it
        Text* sliderText = root->CreateChild<Text>();
        sliderText->SetPosition(x, y);
        sliderText->SetFont(font, 12);
        sliderText->SetText(text);

        Slider* slider = root->CreateChild<Slider>();
        slider->SetStyleAuto();
        slider->SetPosition(x, y + 20);
        slider->SetSize(xSize, ySize);
        // Use 0-1 range for controlling sound/music master volume
        slider->SetRange(1.0f);

        return slider;
    }

    void HandlePlaySound(StringHash eventType, VariantMap& eventData)
    {
        Button* button = static_cast<Button*>(GetEventSender());
        const String& soundResourceName = button->GetVar(VAR_SOUNDRESOURCE).GetString();

        // Get the sound resource
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        Sound* sound = cache->GetResource<Sound>(soundResourceName);

        if (sound)
        {
            // Create a SoundSource component for playing the sound. The SoundSource component plays
            // non-positional audio, so its 3D position in the scene does not matter. For positional sounds the
            // SoundSource3D component would be used instead
            SoundSource* soundSource = scene_->CreateComponent<SoundSource>();
            // Component will automatically remove itself when the sound finished playing
            soundSource->SetAutoRemoveMode(REMOVE_COMPONENT);
            soundSource->Play(sound);
            // In case we also play music, set the sound volume below maximum so that we don't clip the output
            soundSource->SetGain(0.75f);
        }
    }

    void HandlePlayMusic(StringHash eventType, VariantMap& eventData)
    {
        ResourceCache* cache = GetSubsystem<ResourceCache>();
        Sound* music = cache->GetResource<Sound>("Music/Ninja Gods.ogg");
        // Set the song to loop
        music->SetLooped(true);

        musicSource_->Play(music);
    }

    void HandleStopMusic(StringHash eventType, VariantMap& eventData)
    {
        // Remove the music player node from the scene
        musicSource_->Stop();
    }

    void HandleSoundVolume(StringHash eventType, VariantMap& eventData)
    {
        using namespace SliderChanged;

        float newVolume = eventData[P_VALUE].GetFloat();
        GetSubsystem<Audio>()->SetMasterGain(SOUND_EFFECT, newVolume);
    }

    void HandleMusicVolume(StringHash eventType, VariantMap& eventData)
    {
        using namespace SliderChanged;

        float newVolume = eventData[P_VALUE].GetFloat();
        GetSubsystem<Audio>()->SetMasterGain(SOUND_MUSIC, newVolume);
    }

private:
    /// Scene.
    SharedPtr<Scene> scene_;

    SoundSource* musicSource_;
};
URHO3D_DEFINE_APPLICATION_MAIN(MyApp)