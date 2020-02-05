/*
  ==============================================================================

    TrombaString.h
    Created: 21 Oct 2019 4:47:58pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "Global.h"

//==============================================================================
/*
*/
class TrombaString    : public Component
{
public:
    TrombaString (NamedValueSet& parameters, double k, BowModel bowModel);
    ~TrombaString();

    void paint (Graphics&) override;
    void resized() override;
    
    Path visualiseState (int visualScaling);
    void setBridgeState (double val) { bridgeState = val; };
    
    void calculateUpdateEq();
    void dampingFinger();
    void updateStates();

    void excite();
    void NRbow();
    void disableBowing() { bowFlag = false; };
    
    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
    
    double getOutput (double ratio) { int idx = floor(ratio * N); return u[1][idx] - offset; };
    double getStateAt (int time, int idx) { return u[time][idx]; };
    
    void setStateAt (int idx, double val) { u[0][idx] = val; }; // always uNext
    void addToStateAt (int idx, double val) { u[0][idx] += val; }; // always uNext
    
    bool isExcited() { return exciteFlag; };
    bool shouldBow() { return bowing; };
    
    int getNumPoints() { return N; };
    
    void setBowingParameters (float x, float y, double Fb, double Vb, bool mouseInteraction);
    
    void setFingerPos (double val) { _dampingFingerPos.store(val); };
    
    // debug getters
    int getBowPos() { return bp; };
    double getq() { return q; };
    double getb() { return b; };
    double getNRiterator() { return NRiterator; };
    double getVb() { return Vb; };
    
#ifndef EXPONENTIALBOW
    void calcZDot();
    void setNoise (double val) { sig3 = val; };
    
    void setFn (double val) {
        _Fn.store(val);
        _fC.store(mud * val);
        _fS.store(mus * val);
    };
#endif

	void setBreakAwayFactor(double bAF) { breakAwayFactor = bAF; };
    
private:
    double k, h;
    int N;
    
    // Physical parameters
    double rho, A, T, E, Iner, s0, s1, cSq, kappaSq, lambdaSq, muSq;
    
    // update equation constants
    double A1, A2, A3, A4, A5, B1, B2, C1, C2, D, E1;
    
    // NR bow constants
    double b1, b2;
    
    // Elastoplastic
    double z, zPrev, zPrevIt, zDot, zDotNext, zDotPrev, an, anPrev, scaleFact, fnl, z_ba, fC, fS, sig0, sig1, sig2, sig3, sig3w, oOSig0, E2, oOstrvSq, zss, zssNotAbs, oOZss, oOZssMinZba, dz_ss, dz_ssAbs, strv, espon, alph, dalph_v, dalph_z, d_fnlv, d_fnlz, d_fnl, arg, mus, mud, K1, vRelTemp, zTemp, g1, g2, dg1v, dg1z, dg2v, dg2z, determ, invJac, fp, qPrevIt, velCalcDiv;
    int limitCount = 0;
    Random rand;

	double breakAwayFactor = 0.7;

    // pointers to states
    std::vector<double*> u;
    
    // states
    std::vector<std::vector<double>> uVecs;
    
    
    double* uTmp = nullptr;
    
    // excitation variables
    int xPos = 0;
    int yPos = 0;
//    bool exciteFlag = Global::initialiseWithExcitation ? (Global::exciteString ? true : false) : false;
    bool exciteFlag = false;
    bool bowFlag = false;
    bool bowing = true;
    
    std::atomic<double> _Fb {1.0};
    std::atomic<double> _Fn {0.5};
    std::atomic<double> _fC;
    std::atomic<double> _fS;
    std::atomic<double> _Vb {-0.2};
    std::atomic<double> _bowPos {96};
    double Fb, Vb, alpha;
    int bp;
    
    // Newton variables
    double a, b, uI, uIPrev, uI1, uI2, uIM1, uIM2, uIPrev1, uIPrevM1, cOhSq, kOhhSq, BM, eps, tol, NRiterator, q, qPrev, excitation;
    
    double offset;
    double connRatio;
    int connPos;
    double bridgeState;
    
    std::atomic<double> _dampingFingerPos;

	BowModel bowModel;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrombaString)
};
