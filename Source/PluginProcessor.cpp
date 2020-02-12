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
	bridgeLocRatio = 1.65 / 1.90;
	mixVals.resize(3, 0.0);
	prevMixVals.resize(3, 0.0);

#ifdef NOEDITOR
	addParameter(bowVelocity = new AudioParameterFloat ("bowVelocity", // parameter ID
		"Velocity", // parameter name
		-0.5f,   // minimum value
		0.5f,   // maximum value
		0.1f)); // default value
	addParameter(bowForce = new AudioParameterFloat("bowForce", // parameter ID
		"Force", // parameter name
		0.0f,   // minimum value
		0.5f,   // maximum value
		0.05f)); // default value
	addParameter(bowPosition = new AudioParameterFloat("bowPosition", // parameter ID
		"Position", // parameter name
		0.01f,   // minimum value
		0.8f,   // maximum value
		0.25f)); // default value
	addParameter(mixString = new AudioParameterFloat("mixString",
		"String Volume",
		0.0f,
		1.0f,
		0.2f));
	addParameter(mixBridge = new AudioParameterFloat("mixBridge",
		"Bridge Volume",
		0.0f,
		1.0f,
		0.0f));
	addParameter(mixBody = new AudioParameterFloat("mixBody",
		"Body Volume",
		0.0f,
		1.0f,
		0.5f));
	addParameter(dampingFingerPos = new AudioParameterFloat("dampingFingerPos",
		"Pos damp finger",
		0.0f,
		1.0f,
		0.5f));
	addParameter(dampingFingerForce = new AudioParameterFloat("dampingFingerForce",
		"Force damp finger",
		0.0f,
		1.0f,
		0.1f));

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

	offset = 5e-6;

	// string
	double r = 0.0005;
	double f0 = 32.0;
	double rhoS = 7850.0;
	double A = r * r * double_Pi;
	double L = 1.90;
	double T = (f0 * f0 * L * L * 4.0) * rhoS * A;

	outputStringRatio = (1.0 - bridgeLocRatio);
	parameters.set("L", L);
	parameters.set("rhoS", rhoS);
	parameters.set("r", r);
	parameters.set("A", r * r * double_Pi);
	parameters.set("T", T);
	parameters.set("ES", 2e11);
	parameters.set("Iner", r * r * r * r * double_Pi * 0.25);
	parameters.set("s0S", 0.1);
	parameters.set("s1S", 0.05);

	// bridge
	parameters.set("M", 0.001);
	parameters.set("R", 0.05);
	parameters.set("w1", 2.0 * double_Pi * 500);
	parameters.set("offset", offset);

	// body
	parameters.set("rhoP", 50.0);
	parameters.set("H", 0.01);
	parameters.set("EP", 2e5);
	parameters.set("Lx", 1.35);
	parameters.set("Ly", 0.18);
	parameters.set("s0P", 2);
	parameters.set("s1P", 0.05);

	// connection
	parameters.set("K1", 5.0e6);
	parameters.set("alpha1", 1.0);
	parameters.set("connRatio", bridgeLocRatio);

	// plate collision
	parameters.set("K2", 5.0e8);
	parameters.set("alpha2", 1.0);
	parameters.set("colRatioX", 0.8);
	parameters.set("colRatioY", 0.75);

	tromba = std::make_shared<Tromba> (parameters, k, exponential);

	trombaString = tromba->getString();
	bridge = tromba->getBridge();
	body = tromba->getBody();

	double test = (bridgeLocRatio) * 0.5;
	trombaString->setFingerPos(test);
	trombaString->setFingerForce(0.1);

#ifdef NOEDITOR
	prevMixVals[0] = *mixString;
	prevMixVals[1] = *mixBridge;
	prevMixVals[2] = *mixBody;
#endif
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
		trombaString->setBowingParameters (*bowPosition * bridgeLocRatio, 0, *bowForce, *bowVelocity, false);
		trombaString->setFingerPos(*dampingFingerPos * bridgeLocRatio);
		trombaString->setFingerForce(*dampingFingerForce);
#endif
		tromba->calculateUpdateEqs();
        trombaString->dampingFinger();
		tromba->calculateCollisions();
		tromba->solveSystem();
		tromba->updateStates();

#ifdef NOEDITOR
		prevMixVals[0] = prevMixVals[0] * aG + (1 - aG) * (*mixString);
		prevMixVals[1] = prevMixVals[1] * aG + (1 - aG) * (*mixBridge);
		prevMixVals[2] = prevMixVals[2] * aG + (1 - aG) * (*mixBody);
#else
		prevMixVals[0] = prevMixVals[0] * aG + (1 - aG) * (mixVals[0]);
		prevMixVals[1] = prevMixVals[1] * aG + (1 - aG) * (mixVals[1]);
		prevMixVals[2] = prevMixVals[2] * aG + (1 - aG) * (mixVals[2]);
#endif
		output = tromba->getOutput(outputStringRatio) * (Global::debug ? 1.0 : 8.0 * Global::outputScaling) * prevMixVals[0]
			+ tromba->getOutput() * (Global::debug ? 1.0 : 3.0 * Global::outputScaling) * prevMixVals[1]
			+ tromba->getOutput(0.8, 0.75) * (Global::debug ? 1.0 : 50.0 * Global::outputScaling) * prevMixVals[2];

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

