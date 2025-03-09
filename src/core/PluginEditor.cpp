#include "PluginEditor.h"

OxideAudioProcessorEditor::OxideAudioProcessorEditor(OxideAudioProcessor &p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      controlPanel(p.getDistortionProcessor())
{
    addAndMakeVisible(background);
    addAndMakeVisible(headerView);
    addAndMakeVisible(meterView);
    addAndMakeVisible(controlPanel);

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
    auto fullBounds = bounds; // Store the full bounds

    // Header height
    auto headerHeight = 60;

    // Background fills the entire area (including behind the header)
    background.setBounds(fullBounds);

    // Header at the top
    headerView.setBounds(bounds.removeFromTop(headerHeight));

    // Control panel height
    auto controlHeight = 150;

    // Position the control panel at the bottom
    controlPanel.setBounds(bounds.removeFromBottom(controlHeight));

    // The meter view takes the remaining space
    meterView.setBounds(bounds);
}

void OxideAudioProcessorEditor::timerCallback()
{
    // Get raw levels from the processor
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
}