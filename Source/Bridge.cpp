/*
  ==============================================================================

    Bridge.cpp
    Created: 21 Oct 2019 4:48:59pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "Bridge.h"
#include "Global.h"

//==============================================================================
Bridge::Bridge (NamedValueSet& parameters, double k) :  k (k),
                                                        M (*parameters.getVarPointer ("M")),
                                                        w1 (*parameters.getVarPointer ("w1")),
                                                        R (*parameters.getVarPointer ("R")),
                                                        offset(*parameters.getVarPointer ("offset"))
{
    // initialise state vectors
    uVecs.resize(3, offset);
    
    u.resize (3);
    
    for (int i = 0; i < u.size(); ++i)
        u[i] = &uVecs[i];
    
    B1 = M / (k * k);
    B2 = M * w1 * w1;
    
    D = 1.0 / (M / (k * k) + R * M / (2.0 * k));
    
    A1 = 2.0 * B1 - B2;
    A2 = -B1 + R * M / (2.0 * k);
    A3 = B2;
    A1 *= D;
    A2 *= D;
    A3 *= D;
}

Bridge::~Bridge()
{
}

void Bridge::paint (Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */
    int visualScaling = Global::outputScaling * 100.0;
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background
    g.setColour (Colours::grey);
    g.drawLine (0, getHeight() * 0.5 - offset * visualScaling - massRadius, getWidth(), getHeight() * 0.5 - offset * visualScaling - massRadius, 1.0);
    g.setColour (Colours::lawngreen);
    g.drawEllipse (getWidth() * 0.5 - 3, -u[1][0] * visualScaling + getHeight() * 0.5 - 2.0 * massRadius, 6.0, 6.0, massRadius);
    int cVal = Global::clamp (255 * 0.5 * (bodyState * visualScaling * 0.1 + 1), 0, 255);
    g.setColour (Colour::fromRGB (cVal, cVal, cVal));
    g.fillRect (getWidth() * 0.2, getHeight() * 0.5 - visualScaling * bodyState, 0.6 * getWidth(), 5);
}

void Bridge::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void Bridge::calculateUpdateEq()
{
    u[0][0] = A1 * u[1][0] + A2 * u[2][0] + A3 * offset;
}

void Bridge::updateStates()
{
    double* uTmp = u[2];
    u[2] = u[1];
    u[1] = u[0];
    u[0] = uTmp;
}

void Bridge::excite()
{
    u[1][0] += 1.0 / (Global::outputScaling * 100.0);
    u[2][0] += 1.0 / (Global::outputScaling * 100.0);
}

void Bridge::mouseDown (const MouseEvent& e)
{
    excite();
}
