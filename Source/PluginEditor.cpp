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
	Timer::startTimerHz (60);
	for (int i = 0; i < 3; ++i)
	{
		mixSliders.add(new Slider(Slider::LinearHorizontal, Slider::NoTextBox));
		Slider* newSlider = mixSliders[mixSliders.size() - 1];
		newSlider->setRange(0.0, 1.0, 0.001);
		switch (i)
		{
		case 0:
			newSlider->setValue(0.2);
			break;
		case 1:
			newSlider->setValue(0.0);
			break;
		case 2:
			newSlider->setValue(0.5);
			break;
		}
		processor.setMixVal(i, newSlider->getValue());
		processor.setPrevMixVal (i, newSlider->getValue());
		newSlider->addListener(this);
		addAndMakeVisible(newSlider);
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
		int sliderHeight = totalArea.getHeight() / mixSliders.size();
		for (auto slider : mixSliders)
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
		Logger::getCurrentLogger()->outputDebugString(String(mixSliders.indexOf(slider)));
		processor.setMixVal(mixSliders.indexOf(slider), slider->getValue());
	/*	if (slider == sliders[2])
			tromba->getString()->setBreakAwayFactor(slider->getValue());
		else*/
//            tromba->getString()->setBowingParameters(0.3, 0.0, sliders[1]->getValue(), sliders[0]->getValue(), false);
	}
}
