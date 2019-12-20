/*
  ==============================================================================

    Tromba.h
    Created: 21 Oct 2019 4:50:13pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Global.h"
#include "Body.h"
#include "Bridge.h"
#include "TrombaString.h"

//==============================================================================
/*
*/
class Tromba    : public Component
{
public:
    Tromba (NamedValueSet& parameters, double k);
    ~Tromba();

    void paint (Graphics&) override;
    void resized() override;
    
    void calculateCollisions();
//    void calculateConnection();
    void solveSystem();
    void calculateUpdateEqs();
    void updateStates();
    
    double getOutput() { return bridge->getOutput() * 10; };
    double getOutput (double ratio) { return trombaString->getOutput (ratio); };
    double getOutput (double ratioX, double ratioY) { return body->getOutput (ratioX, ratioY); };
    
    std::shared_ptr<TrombaString> getString() { return trombaString; };
    std::shared_ptr<Bridge> getBridge() { return bridge; };
    std::shared_ptr<Body> getBody() { return body; };
    
    void setCurSample (unsigned long curSamp) { curSample = curSamp; };
    
private:
    
    double k, kSq; 
    // Instrument components (String, body and bridge)
    std::shared_ptr<TrombaString> trombaString;
    std::shared_ptr<Bridge> bridge;
    std::shared_ptr<Body> body;
    
    // String variables needed for calculating connections
    double rhoS, A, T, ES, Iner, s0S, s1S, hS;

    // Mass variables needed for calculating connections
    double M, w1, R;
    
    // Body variables needed for calculating collision
    double rhoP, H, E, hP, s0P, s1P;

    // Connection variables
    double K1, alph1, g1, etaConn, psi1, psi1Prev, psiPrevg1, gg1, gg1Ksq;
    double connRatio;
    int cP;
    
    // Collision variables
    double K2, alph2, g2, etaCol, psi2, psi2Prev, psiPrevg2, gg2, gg2Ksq;
    double colRatioX, colRatioY;
    int cPX, cPY;
    
    // Connection calculation coefficients
    double A1S, A2S, A3S, A4S, A5S, B1S, B2S, B3S, B4S, B5S, DS;
    double B1B, B2B, B3B, DB;
    double B1P, B2P, B3P, DP;
    
    // Variables for doing the linear system solve
    double a11, a12, a21, a22;
    double v1, v2;
    double solut1, solut2;
    double oOMassTerm, oOStringTerm, oOPlateTerm;
    double oOMassTermO2, oOStringTermO2, oOPlateTermO2;
    
    double kSqOStringTerm, kSqOMassTerm, kSqOPlateTerm;

    
    double oOdet;
    double oOhS, oOhPSq, oOk, oO2k;
    
    unsigned long curSample;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Tromba)
};
