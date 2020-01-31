/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "Tromba.h"

//==============================================================================
/**
*/
class TrombaMarinaPluginAudioProcessorEditor : public AudioProcessorEditor, public Timer, public Slider::Listener
{
public:
    TrombaMarinaPluginAudioProcessorEditor (TrombaMarinaPluginAudioProcessor&);
    ~TrombaMarinaPluginAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

	void timerCallback() override;
	void sliderValueChanged (Slider* slider) override;


private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
	OwnedArray<Slider> sliders;
    TrombaMarinaPluginAudioProcessor& processor;
	std::shared_ptr<Tromba> tromba;
	unsigned long t = 0;
	bool init = true;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrombaMarinaPluginAudioProcessorEditor)
};
