/*
  ==============================================================================

    Bridge.h
    Created: 21 Oct 2019 4:48:59pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class Bridge    : public Component
{
public:
    Bridge (NamedValueSet& parameters, double k);
    ~Bridge();

    void paint (Graphics&) override;
    void resized() override;

    void calculateUpdateEq();
    void updateStates();
    
    void excite();
    
    double getOutput() { return u[1][0] - offset; };
    double getState (int time) { return u[time][0]; };
    void setState (double val) { u[0][0] = val; };
    
    void mouseDown (const MouseEvent& e) override;
    
    void setBodyState (double state) { bodyState = state; };
    void addToState (double val) {u[0][0] += val; };
    
private:
    double k;
    
    std::vector<double*> u;
    std::vector<double> uVecs; // not "vecs", but consistency..
    
    double M, w1, R;
    
    // update equation constants
    double A1, A2, A3, B1, B2, D;
    
    double offset = 0;
    double bodyState;
    
    float massRadius = 4.0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Bridge)
};
