/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Tromba.h"

//==============================================================================
/**
*/
class TrombaMarinaPluginAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    TrombaMarinaPluginAudioProcessor();
    ~TrombaMarinaPluginAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	std::shared_ptr<Tromba> getTrombaPtr() { return tromba; };

	double clamp(double val, double min, double max)
	{
		if (val < min)
			return min;
		else if (val > max)
			return max;
		else
			return val;
	}
private:
	unsigned long t = 0;
	double fs;
	double freq = 440.0;

	std::shared_ptr<Tromba> tromba;

	std::shared_ptr<TrombaString> trombaString;
	std::shared_ptr<Bridge> bridge;
	std::shared_ptr<Body> body;

	double bridgeLocRatio;
	double offset;

#ifdef NOEDITOR
	AudioParameterFloat* initFreq;
	AudioParameterFloat* bowVelocity;
	AudioParameterFloat* bowForce;
	AudioParameterFloat* bowPosition;
	AudioParameterFloat* mixString;
	AudioParameterFloat* mixBridge;
	AudioParameterFloat* mixBody;

	float prevMixString, prevMixBridge, prevMixBody;
	float aG = 0.99; // averaging filter gain
#endif


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrombaMarinaPluginAudioProcessor)
};
