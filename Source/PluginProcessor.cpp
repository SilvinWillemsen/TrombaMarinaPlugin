/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TrombaMarinaPluginAudioProcessor::TrombaMarinaPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
#ifdef NOEDITOR
	addParameter(bowVelocity = new AudioParameterFloat ("bowVelocity", // parameter ID
		"Velocity", // parameter name
		-0.2f,   // minimum value
		0.2f,   // maximum value
		0.2f)); // default value
	addParameter(bowForce = new AudioParameterFloat("bowForce", // parameter ID
		"Force", // parameter name
		0.0f,   // minimum value
		1.0f,   // maximum value
		0.05f)); // default value
	addParameter(bowPosition = new AudioParameterFloat("bowPosition", // parameter ID
		"Position", // parameter name
		0.0f,   // minimum value
		1.0f,   // maximum value
		0.5f)); // default value
	addParameter(mixString = new AudioParameterFloat("mixString",
		"String Volume",
		0.0f,
		1.0f,
		0.5f));
	addParameter(mixBridge = new AudioParameterFloat("mixBridge",
		"Bridge Volume",
		0.0f,
		1.0f,
		0.5f));
	addParameter(mixBody = new AudioParameterFloat("mixBody",
		"Body Volume",
		0.0f,
		1.0f,
		0.5f));
#endif
}

TrombaMarinaPluginAudioProcessor::~TrombaMarinaPluginAudioProcessor()
{
}

//==============================================================================
const String TrombaMarinaPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TrombaMarinaPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool TrombaMarinaPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool TrombaMarinaPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double TrombaMarinaPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TrombaMarinaPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int TrombaMarinaPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TrombaMarinaPluginAudioProcessor::setCurrentProgram (int index)
{
}

const String TrombaMarinaPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void TrombaMarinaPluginAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void TrombaMarinaPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
	fs = sampleRate;

	double k = 1.0 / fs;

	NamedValueSet parameters;

	offset = 0;

	// string
	double r = 0.0005;
	double f0 = 60.0;
	double rhoS = 7850.0;
	double A = r * r * double_Pi;
	double T = (f0 * f0 * 4.0) * rhoS * A;
	bridgeLocRatio = 18.0 / 20.0;
	parameters.set("rhoS", rhoS);
	parameters.set("r", r);
	parameters.set("A", r * r * double_Pi);
	parameters.set("T", T);
	parameters.set("ES", 2e11);
	parameters.set("Iner", r * r * r * r * double_Pi * 0.25);
	parameters.set("s0S", 0.1);
	parameters.set("s1S", 1);

	// bridge
	parameters.set("M", 0.001);
	parameters.set("R", 0.1);
	parameters.set("w1", 2.0 * double_Pi * 500);
	parameters.set("offset", offset);

	// body
	parameters.set("rhoP", 10.0);
	parameters.set("H", 0.01);
	parameters.set("EP", 2e5);
	parameters.set("Lx", 1.5);
	parameters.set("Ly", 0.4);
	parameters.set("s0P", 5);
	parameters.set("s1P", 0.05);

	// connection
	parameters.set("K1", 5.0e6);
	parameters.set("alpha1", 1.0);
	parameters.set("connRatio", bridgeLocRatio);

	// plate collision
	parameters.set("K2", 5.0e10);
	parameters.set("alpha2", 1.0);
	parameters.set("colRatioX", 0.8);
	parameters.set("colRatioY", 0.5);

	tromba = std::make_shared<Tromba> (parameters, k);

	trombaString = tromba->getString();
	bridge = tromba->getBridge();
	body = tromba->getBody();

	trombaString->setFingerPos(0.0);

	prevMixString = *mixString;
	prevMixBridge = *mixBridge;
	prevMixBody = *mixBody;
}

void TrombaMarinaPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool TrombaMarinaPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void TrombaMarinaPluginAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	buffer.clear();

	float* const channelData1 = buffer.getWritePointer(0);
	float* const channelData2 = buffer.getWritePointer(1);

	float output = 0.0;
	for (int i = 0; i < buffer.getNumSamples(); ++i)
	{	
		//if (t % static_cast<int>(fs) == 0)
		//	trombaString->excite();
		if (trombaString->isExcited() && !trombaString->shouldBow())
			trombaString->excite();
		if (body->isExcited())
			body->excite();
#ifdef NOEDITOR
		trombaString->setBowingParameters (*bowPosition, 0, *bowForce, *bowVelocity, false);
#endif
		tromba->calculateUpdateEqs();
		trombaString->dampingFinger();
		tromba->calculateCollisions();
		tromba->solveSystem();
		tromba->updateStates();

#ifdef NOEDITOR
		prevMixString = prevMixString * aG + (1 - aG) * (*mixString);
		prevMixBridge = prevMixBridge * aG + (1 - aG) * (*mixBridge);
		prevMixBody = prevMixBody * aG + (1 - aG) * (*mixBody);
		output = tromba->getOutput(0.9) * (Global::debug ? 1.0 : 8.0 * Global::outputScaling) * prevMixString
			+ tromba->getOutput() * (Global::debug ? 1.0 : 3.0 * Global::outputScaling) * prevMixBridge
			+ tromba->getOutput(0.8, 0.5) * (Global::debug ? 1.0 : 50.0 * Global::outputScaling) * prevMixBody;
#else 
		output = tromba->getOutput() * (Global::debug ? 1.0 : Global::outputScaling);
#endif
		channelData1[i] = Global::clamp(output, -1, 1);
		channelData2[i] = Global::clamp(output, -1, 1);

		//++t;
	}
	
}

//==============================================================================
bool TrombaMarinaPluginAudioProcessor::hasEditor() const
{
#ifdef NOEDITOR
    return false; // (change this to false if you choose to not supply an editor)
#else
	return true;
#endif
}

AudioProcessorEditor* TrombaMarinaPluginAudioProcessor::createEditor()
{
#ifdef NOEDITOR
    return new GenericAudioProcessorEditor(this);
#else
	return new TrombaMarinaPluginAudioProcessorEditor (*this);
#endif
}

//==============================================================================
void TrombaMarinaPluginAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void TrombaMarinaPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TrombaMarinaPluginAudioProcessor();
}

