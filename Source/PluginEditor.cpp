/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TrombaMarinaPluginAudioProcessorEditor::TrombaMarinaPluginAudioProcessorEditor (TrombaMarinaPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
	Timer::startTimerHz (30);
    setSize (800, 600);
}

TrombaMarinaPluginAudioProcessorEditor::~TrombaMarinaPluginAudioProcessorEditor()
{
}

//==============================================================================
void TrombaMarinaPluginAudioProcessorEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));

    g.setColour (Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), Justification::centred, 1);
}

void TrombaMarinaPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	if (!init)
		tromba->setBounds (getLocalBounds());
	
}

void TrombaMarinaPluginAudioProcessorEditor::timerCallback()
{
	if (init && processor.getTrombaPtr() != nullptr)
	{
		tromba = processor.getTrombaPtr();	
		addAndMakeVisible(tromba.get());
		Logger::outputDebugString("trombaIsAssigned");
		init = false;
		resized();
	}
	//++t;
	//if (t % 30 == 0)
	//	tromba->getString()->excite();
	repaint();
}