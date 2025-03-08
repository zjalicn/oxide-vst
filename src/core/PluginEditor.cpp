#include "PluginEditor.h"

OxideAudioProcessorEditor::OxideAudioProcessorEditor(OxideAudioProcessor &p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      controlPanel(p.getDistortionProcessor()),
      delayControlPanel(p.getDelayProcessor()),
      oscilloscopeView(p.getDistortionProcessor())
{
    addAndMakeVisible(background);
    addAndMakeVisible(headerView);
    addAndMakeVisible(meterView);
    addAndMakeVisible(oscilloscopeView);
    addAndMakeVisible(controlPanel);
    addAndMakeVisible(delayControlPanel);

    // Oscilloscope update (in timerCallback)
    oscilloscopeView.updateBuffer(p.getOutputBuffer());

    headerView.onPresetSelected = [this](const juce::String &presetName)
    {
        if (presetName == "light_drive")
        {
            audioProcessor.getDistortionProcessor().setDrive(0.3f);
            audioProcessor.getDistortionProcessor().setMix(0.5f);
            audioProcessor.getDistortionProcessor().setInputGain(0.0f);
            audioProcessor.getDistortionProcessor().setOutputGain(0.0f);
            audioProcessor.getDistortionProcessor().setAlgorithm(DistortionAlgorithm::SoftClip);
        }
        else if (presetName == "heavy_metal")
        {
            audioProcessor.getDistortionProcessor().setDrive(0.8f);
            audioProcessor.getDistortionProcessor().setMix(0.7f);
            audioProcessor.getDistortionProcessor().setInputGain(3.0f);
            audioProcessor.getDistortionProcessor().setOutputGain(2.0f);
            audioProcessor.getDistortionProcessor().setAlgorithm(DistortionAlgorithm::HardClip);
        }
        else if (presetName == "fuzz")
        {
            audioProcessor.getDistortionProcessor().setDrive(1.0f);
            audioProcessor.getDistortionProcessor().setMix(1.0f);
            audioProcessor.getDistortionProcessor().setInputGain(6.0f);
            audioProcessor.getDistortionProcessor().setOutputGain(-3.0f);
            audioProcessor.getDistortionProcessor().setAlgorithm(DistortionAlgorithm::Foldback);
        }
        else if (presetName == "warm_tape")
        {
            audioProcessor.getDistortionProcessor().setDrive(0.5f);
            audioProcessor.getDistortionProcessor().setMix(0.4f);
            audioProcessor.getDistortionProcessor().setInputGain(1.5f);
            audioProcessor.getDistortionProcessor().setOutputGain(0.0f);
            audioProcessor.getDistortionProcessor().setAlgorithm(DistortionAlgorithm::Waveshaper);
        }
        else
        { // default
            audioProcessor.getDistortionProcessor().setDrive(0.5f);
            audioProcessor.getDistortionProcessor().setMix(0.5f);
            audioProcessor.getDistortionProcessor().setInputGain(0.0f);
            audioProcessor.getDistortionProcessor().setOutputGain(0.0f);
            audioProcessor.getDistortionProcessor().setAlgorithm(DistortionAlgorithm::SoftClip);
        }

        // Update the UI to reflect the new values
        meterView.setInputGain(audioProcessor.getDistortionProcessor().getInputGain());
        meterView.setOutputGain(audioProcessor.getDistortionProcessor().getOutputGain());
    };

    headerView.onSaveClicked = [this]()
    {
        // Here you would implement saving the current settings
        // For example, you could show a file save dialog
        // and save the current state as a preset file
    };

    // Set up callbacks for input/output gain changes from the UI
    meterView.onInputGainChanged = [this](float newGain)
    {
        audioProcessor.getDistortionProcessor().setInputGain(newGain);
    };

    meterView.onOutputGainChanged = [this](float newGain)
    {
        audioProcessor.getDistortionProcessor().setOutputGain(newGain);
    };

    // Initialize the meter with the current gain values
    meterView.setInputGain(audioProcessor.getDistortionProcessor().getInputGain());
    meterView.setOutputGain(audioProcessor.getDistortionProcessor().getOutputGain());

    // Start the timer for meter updates
    startTimerHz(30);

    // Set initial size
    setSize(800, 600);
}

OxideAudioProcessorEditor::~OxideAudioProcessorEditor()
{
    stopTimer();
}

void OxideAudioProcessorEditor::paint(juce::Graphics &g)
{
    // No need to paint anything here since the background component does that
}

void OxideAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // LAYOUT
    background.setBounds(bounds);
    auto headerHeight = 60;
    headerView.setBounds(bounds.removeFromTop(headerHeight));
    // meterView.setBounds(bounds);

    // OSCILLOSCOPE
    int oscilloscopeSize = std::min(bounds.getHeight(), bounds.getWidth() / 4);
    int oscilloscopeX = bounds.getX() + (bounds.getWidth() - oscilloscopeSize) / 2;
    int oscilloscopeY = bounds.getY() + (bounds.getHeight() - oscilloscopeSize) * 0.33;
    oscilloscopeView.setBounds(oscilloscopeX, oscilloscopeY, oscilloscopeSize, oscilloscopeSize);

    // // DISTORTION
    // auto controlHeight = 150;
    // controlPanel.setBounds(bounds.removeFromBottom(controlHeight + 10));

    // DELAY
    auto delayHeight = 150;
    delayControlPanel.setBounds(bounds.removeFromBottom(delayHeight + 50));
}

void OxideAudioProcessorEditor::timerCallback()
{
    float leftLevel = audioProcessor.getLeftLevel();
    float rightLevel = audioProcessor.getRightLevel();

    // Apply input gain to the displayed levels to match the actual processing
    float inputGainLinear = std::pow(10.0f, audioProcessor.getDistortionProcessor().getInputGain() / 20.0f);
    leftLevel *= inputGainLinear;
    rightLevel *= inputGainLinear;

    // Scale for display - convert RMS values to percentage heights
    leftLevel = std::pow(leftLevel * 100.0f, 0.5f) * 10.0f;
    rightLevel = std::pow(rightLevel * 100.0f, 0.5f) * 10.0f;

    // Ensure values are in range
    leftLevel = juce::jlimit(0.0f, 100.0f, leftLevel);
    rightLevel = juce::jlimit(0.0f, 100.0f, rightLevel);

    // Allow small values to show as completely empty
    if (leftLevel < 0.1f)
        leftLevel = 0.0f;
    if (rightLevel < 0.1f)
        rightLevel = 0.0f;

    // Update the meter display
    meterView.updateLevels(leftLevel, rightLevel);

    // Update the oscilloscope with latest audio buffer
    oscilloscopeView.updateBuffer(audioProcessor.getOutputBuffer());
}