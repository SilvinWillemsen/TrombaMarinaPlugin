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
	sliders.add(new Slider(Slider::LinearHorizontal, Slider::TextBoxRight));
	sliders[0]->setRange(-0.5, 0.5, 0.01);
	sliders[0]->setValue(0.1);
	
	sliders.add(new Slider(Slider::LinearHorizontal, Slider::TextBoxRight));
	sliders[1]->setRange(0.0, 1.0, 0.01);
	sliders[1]->setValue(0.4);

	sliders.add(new Slider(Slider::LinearHorizontal, Slider::TextBoxRight));
	sliders[2]->setRange(0.0, 0.99, 0.01);
	sliders[2]->setValue(0.7);

	for (auto slider : sliders)
	{ 
		addAndMakeVisible(slider);
		slider->addListener(this);
	}
	
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
	{
		Rectangle<int> totalArea = getLocalBounds();
		tromba->setBounds(totalArea.removeFromTop(0.8 * getHeight()));
		int sliderHeight = totalArea.getHeight() / sliders.size();
		for (auto slider : sliders)
			slider->setBounds(totalArea.removeFromTop(sliderHeight));
	}
	
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

void TrombaMarinaPluginAudioProcessorEditor::sliderValueChanged(Slider* slider)
{
	if (!init)
	{
		if (slider == sliders[2])
			tromba->getString()->setBreakAwayFactor(slider->getValue());
		else
			tromba->getString()->setBowingParameters(0.3, 0.0, sliders[1]->getValue(), sliders[0]->getValue(), false);
	}
}