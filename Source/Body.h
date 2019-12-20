/*
  ==============================================================================

    Body.h
    Created: 21 Oct 2019 4:49:29pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Global.h"


//==============================================================================
/*
*/
class Body    : public Component
{
public:
    Body (NamedValueSet& parameters, double k);
    ~Body();

    void paint (Graphics&) override;
    void resized() override;

    void calculateUpdateEq();
    void updateStates();
    
    void excite();
    
    
    double getOutput (double ratioX, double ratioY) {
        int idx = floor (Nx * ratioX) + floor (ratioY * Ny * Nx);
        return u[1][idx];
        
    };
    double getStateAt (int time, int idX, int idY) {
        return u[time][idX + Nx * idY];
    };

    void addToStateAt (int idX, int idY, double val) { u[0][idX + Nx * idY] += val; }; // always uNext

    int getNumHorPoints() { return Nx; };
    int getNumVertPoints() { return Ny; };
    
    void mouseDrag (const MouseEvent& e) override;
    
    bool isExcited() { return exciteFlag; };
    double getGridSpacing() { return h; };
    
private:
    double k, h;
    int N, Nx, Ny;
    
    double rho, H, E, s0, s1, Lx, Ly, kappaSq;
    
    // pointers to the different states
    std::vector<double*> u;
    // states
    std::vector<std::vector<double>> uVecs;
    double* uTmp;
    
    // update equation constants
    double A1, A2, A3, A4, A5, A6, B1, B2, D;
    
    int excitationWidth;
    bool exciteFlag = false;
    int idX, idY;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Body)
};
