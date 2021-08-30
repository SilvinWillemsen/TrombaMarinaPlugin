/*
  ==============================================================================

    TrombaString.cpp
    Created: 21 Oct 2019 4:47:58pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "TrombaString.h"

//==============================================================================
TrombaString::TrombaString (NamedValueSet& parameters, double k, BowModel bowModel) :
                    k (k),
                    L (*parameters.getVarPointer("L")),
                    rho (*parameters.getVarPointer("rhoS")),
                    A (*parameters.getVarPointer("A")),
                    T (*parameters.getVarPointer("T")),
                    E (*parameters.getVarPointer("ES")),
                    Iner (*parameters.getVarPointer("Iner")),
                    s0 (*parameters.getVarPointer("s0S")),
                    s1 (*parameters.getVarPointer("s1S")),
                    offset (*parameters.getVarPointer("offset")),
                    connRatio (*parameters.getVarPointer("connRatio")),
                    bowModel (bowModel)
{
    
    // calculate wave speed and stiffness coefficient
    cSq = T / (rho * A);
    kappaSq = E * Iner / (rho * A);
    
    // calculate grid spacing and number of points
    h = sqrt ((cSq * k * k + 4.0 * s1 * k + sqrt(pow(cSq * k * k + 4.0 * s1 * k, 2) + 16.0 * kappaSq * k * k)) / 2.0);
    
//    // Scale damping by rho * A
//    s0 = s0 * rho * A;
//    s1 = s1 * rho * A;
    
    N = floor (0.95 / h);
    h = 1.0 / static_cast<double>(N);
    std::cout << "String numpoints: " << N << std::endl;
    // initialise state vectors
    uVecs.reserve (3);
    
    for (int i = 0; i < 3; ++i)
        uVecs.push_back (std::vector<double> (N, offset));
    
    u.resize (3);
    
    for (int i = 0; i < u.size(); ++i)
        u[i] = &uVecs[i][0];
    
    // courant numbers
    lambdaSq = cSq * k * k / (h * h);
    muSq = k * k * kappaSq / (h * h * h * h);
    
    std::cout << "Courant number: " << (lambdaSq + 4.0 * muSq) << std::endl;
    // Newton Variables bow model
    cOhSq = cSq / (h * h);
    kOhhSq = kappaSq / (h * h * h * h);
    
    tol = 1e-4;
    
    a = 100; // Free parameter
    BM = sqrt(2.0 * a) * exp (0.5);
    
    // Elasto-Plastic bow model
    
    //// the Contact Force (be with you) //////
    mus = 0.6; // static friction coeff
    mud = 0.1; // dynamic friction coeff (must be < mus!!) %EDIT: and bigger than 0
    strv = 0.1;      // "stribeck" velocity
    
    _Fn.store(0.3);    // Normal force
    
    _fC.store (mud * _Fn.load()); // coulomb force
    _fS.store (mus * _Fn.load()); // stiction force
    
    sig0 = 10000;                   // bristle stiffness
    sig1 = 0.001*sqrt(sig0);          // bristle damping
    sig2 = 0.6;                     // viscous friction term
    sig3 = 0.1;                       // noise term
    oOstrvSq = 1.0 / (strv * strv);   // One over strv^2
    z_ba = 0.7 * fC / sig0;         // break-away displacement (has to be < f_c/sigma_0!!)
    
    // Initialise variables for Newton Raphson
    tol = 1e-7;
    q = 0;
    qPrev = 0;
    z = 0;
    zPrev = 0;
    zDotPrev = 0;
    anPrev = 0;
    fp = 0;
    
    velCalcDiv =  1.0 / (sig2 + scaleFact * (2.0 / k + 2.0 * s0));
    oOSig0 = 1.0 / sig0;
    
    // set coefficients for update equation
    B1 = s0 * k;
    B2 = (2.0 * s1 * k) / (h * h);
    
    D = 1.0 / (1.0 + s0 * k);
    
    b1 = 2.0 / (k * k);
    b2 = (2.0 * s1) / (k * h * h);
    
    A1 = 2.0 - 2.0 * lambdaSq - 6.0 * muSq - 2.0 * B2;
    A1ss = 2.0 - 2.0 * lambdaSq - 5.0 * muSq - 2.0 * B2;
    A2 = lambdaSq + 4.0 * muSq + B2;
    A3 = muSq;
    A4 = B1 - 1.0 + 2.0 * B2;
    A5 = B2;
    
    A1 *= D;
    A1ss *= D;
    A2 *= D;
    A3 *= D;
    A4 *= D;
    A5 *= D;
    
    s0 *= rho * A;
    s1 *= rho * A;
    b2 = (2.0 * s1) / (k * h * h);
    
    Eexp = (1.0 / h) * BM / (rho * A / (k * k) + s0 / k);
    Eelasto = 1.0 / (h * (rho * A / (k * k) + s0 / k));
    
    qPrev = -_Vb.load();
    
    if (Global::bowDebug && Global::exciteString && bowing)
    {
        bp = _bowPos.load();
        setBowingParameters (0.0, 0.0, 1, 0.2, true);
    }
    
    connPos = floor(connRatio * N);
}

TrombaString::~TrombaString()
{
}

void TrombaString::paint (Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background
    g.setColour (Colours::grey);
    double ratioFingerToBridge = _dampingFingerPos.load() / connRatio;
    for (double i = 1.0; i < round(1.0 / ratioFingerToBridge); ++i)
    {
        g.drawLine (i / double(round(1.0 / ratioFingerToBridge)) * connRatio * getWidth(), 0.0, i / double(round(1.0 / ratioFingerToBridge)) * connRatio * getWidth(), getHeight());
    }
    g.setColour (Colours::cyan);
    int visualScaling = Global::outputScaling * 100;
    Path stringPath = visualiseState (visualScaling, g);
    g.strokePath (stringPath, PathStrokeType(2.0f));
    g.setColour (Colours::yellow);
    
    if (_dampingFingerForce.load() != 0)
        g.drawEllipse (_dampingFingerPos.load() * getWidth(), getHeight() * 0.5, 2, 2, (_dampingFingerForce.load() * 0.8 + 0.2) * 20.0);
    
    double opa = bowModel == exponential ? _Fb.load() * 10.0 : _Fn.load() * 10.0;
    if (opa >= 1.0)
    {
        g.setOpacity(1.0);
    }
    else
    {
        g.setOpacity(opa);
    }
    g.fillRect(xPos - 5, yPos - getHeight() * 0.25, 10, getHeight() * 0.5);
    
    g.setColour (Colours::lawngreen);
//    std::cout << "Width = " << getWidth() << std::endl;
//    std::cout << "Bridge coordinate: (" << connPos / static_cast<double>(N) * getWidth() << ", " << getHeight() * 0.5 - (bridgeState * visualScaling) << ")" << std::endl;
    if (!isnan(bridgeState))
        g.drawEllipse (connPos * h * getWidth(), getHeight() * 0.5 - (bridgeState * visualScaling), 2, 2, 5);
}

void TrombaString::resized()
{

}

Path TrombaString::visualiseState (int visualScaling, Graphics& g)
{
    auto stringBounds = getHeight() / 2.0;
    Path stringPath;
    stringPath.startNewSubPath (0, stringBounds - visualScaling * offset);
    int stateWidth = getWidth();
    auto spacing = stateWidth / static_cast<double>(N);
    auto x = spacing;
    
    for (int y = 1; y < N; y++)
    {
        float newY = -u[1][y] * visualScaling + stringBounds; // Needs to be -u, because a positive u would visually go down
        
       /* if (isnan(x) || isinf(abs(x) || isnan(newY) || isinf(abs(newY))))
        {
            std::cout << "Wait" << std::endl;
        };
        */
        if (isnan(newY))
            newY = 0;
        stringPath.lineTo(x, newY);
//        g.drawEllipse(x, newY, 2, 2, 5);
        x += spacing;
    }
    stringPath.lineTo(stateWidth, stringBounds - visualScaling * offset);
    return stringPath;
}

void TrombaString::calculateUpdateEq()
{
    for (int l = 2; l < N - 2; ++l)
    {
        u[0][l] = A1 * u[1][l] + A2 * (u[1][l + 1] + u[1][l - 1]) - A3 * (u[1][l + 2] + u[1][l - 2]) + A4 * u[2][l] - A5 * (u[2][l + 1] + u[2][l - 1]);
    }
    int l = 1;
    u[0][l] = A1ss * u[1][l] + A2 * (u[1][l + 1] + u[1][l - 1]) - A3 * (u[1][l + 2] + 2.0 * offset) + A4 * u[2][l] - A5 * (u[2][l + 1] + u[2][l - 1]);
    l = N - 2;
    u[0][l] = A1ss * u[1][l] + A2 * (u[1][l + 1] + u[1][l - 1]) - A3 * (u[1][l - 2] + 2.0 * offset) + A4 * u[2][l] - A5 * (u[2][l + 1] + u[2][l - 1]);
    
    if (!bowing)
    {
        return;
    }
    else if (bowFlag)
    {
        // for using the same 'dynamic variables' during one loop
        Vb = _Vb.load();
        Fb = _Fb.load();
        sig3w = (rand.nextFloat() * 2 - 1) * sig3;
        fC = _fC.load();
        fS = _fS.load();
        bp = floor (_bowPos.load());
        alpha = _bowPos.load() - bp;
        NRbow();
        if (getBowModel() == exponential)
            excitation = Eexp * Fb * q * Global::exp1(-a * q * q);
        else if (getBowModel() == elastoPlastic)
            excitation = Eelasto * (sig0 * z + sig1 * zDot + sig2 * q + sig3w); //* (rho * csA);

        Global::extrapolation (u[0], bp, alpha, -excitation);
    }
    
}

void TrombaString::dampingFinger()
{
    float dampLoc = Global::clamp(_dampingFingerPos.load() * N, 3, N - 4);
    double uVal = Global::interpolation(u[0], floor(dampLoc), dampLoc - floor(dampLoc)) - offset;
    double dampingScaling = 1.0 - _dampingFingerForce.load();
	//Logger::getCurrentLogger()->outputDebugString (String (dampingScaling));
    Global::extrapolation (u[0], floor(dampLoc), pow(dampLoc - floor(dampLoc), 7), -(uVal - uVal * dampingScaling));
}

void TrombaString::updateStates()
{
    uTmp = u[2];
    u[2] = u[1];
    u[1] = u[0];
    u[0] = uTmp;
    
    qPrev = q;
}

void TrombaString::excite()
{
    exciteFlag = false;
//    int width = floor ((N * 2.0) / 5.0) / 2.0;
    int width = 10;
    int loc = floor (N * static_cast<float>(xPos) / static_cast<float>(getWidth()));
    if (Global::debug)
        loc = floor(N / 2.0);
    
    int startIdx = Global::clamp (loc - width / 2.0, 2, N - 2 - width);
    double amp = Global::debug ? -offset * 10000.0 : 100.0;
    for (int i = 0; i < width; ++i)
    {
        double val = amp * (1.0 - cos (2.0 * double_Pi * i / width)) * (Global::debug ? 0.5 : 0.5 / (Global::outputScaling * 10.0));
        u[1][startIdx + i] = u[1][startIdx + i] + val;
        u[2][startIdx + i] = u[2][startIdx + i] + val;
    }
}

void TrombaString::NRbow()
{
    
    // Interpolation
    uI = Global::interpolation (u[1], bp, alpha);
    uIPrev = Global::interpolation (u[2], bp, alpha);
    uI1 = Global::interpolation (u[1], bp + 1, alpha);
    uI2 = Global::interpolation (u[1], bp + 2, alpha);
    uIM1 = Global::interpolation (u[1], bp - 1, alpha);
    uIM2 = Global::interpolation (u[1], bp - 2, alpha);
    uIPrev1 = Global::interpolation (u[2], bp + 1, alpha);
    uIPrevM1 = Global::interpolation (u[2], bp - 1, alpha);
    
    // error term
    eps = 1;
    NRiterator = 0;
    
    if (getBowModel() == exponential)    // Calculate precalculable part
    {
        b = 2.0 / k * Vb + 2.0 * s0 * Vb - b1 * (uI - uIPrev) - cOhSq * (uI1 - 2.0 * uI + uIM1) + kOhhSq * (uI2 - 4.0 * uI1 + 6.0 * uI - 4.0 * uIM1 + uIM2) - b2 * ((uI1 - 2 * uI + uIM1) - (uIPrev1 - 2.0 * uIPrev + uIPrevM1));


        // NR loop
        while (eps > tol && NRiterator < 100)
        {
            q = qPrev - (Fb * BM * qPrev * exp (-a * qPrev * qPrev) + 2.0 * qPrev / k + 2.0 * s0 * qPrev + b) /
                    (Fb * BM * (1.0 - 2.0 * a * qPrev * qPrev) * exp (-a * qPrev * qPrev) + 2.0 / k + 2.0 * s0);
            eps = std::abs (q - qPrev);
            qPrev = q;
            ++NRiterator;
            if (NRiterator > 98)
            {
                std::cout << "Nope" << std::endl;
            }
        }
    }
    else if (getBowModel() == elastoPlastic)
    {
        b = 2.0 / k * Vb - b1 * (uI - uIPrev) - cOhSq * (uI1 - 2 * uI + uIM1) + kOhhSq * (uI2 - 4 * uI1 + 6 * uI - 4 * uIM1 + uIM2) + 2 * s0 * Vb - b2 * ((uI1 - 2 * uI + uIM1) - (uIPrev1 - 2 * uIPrev + uIPrevM1));
        z_ba = 0.7 * fC * oOSig0;

        while (eps > tol && NRiterator < 50 && fC > 0)
        {
            calcZDot();
            
            g1 = (2.0 / k + 2 * s0) * q + (sig0 * z + sig1 * zDot + sig2 * q + sig3w) / (rho * A * h) + b;
            g2 = zDot - an;
            
            // compute derivatives
            
            // dz_ss/dv
            dz_ss = (-2 * abs(q) * oOstrvSq * oOSig0) * (fS-fC) * espon;
            dz_ssAbs = Global::sgn(zss) * dz_ss;
            
            dalph_v = 0; //d(alph)/dv
            dalph_z = 0; //d(alph)/dz
            zss = abs(zss);
            if ((Global::sgn(z)==Global::sgn(q)) && (abs(z)>z_ba) && (abs(z)<zss) )
            {
                double cosarg = cos(Global::sgn(z) * arg);
                dalph_v = 0.5 * double_Pi * cosarg * dz_ssAbs * (z_ba - abs(z)) * oOZssMinZba * oOZssMinZba;
                dalph_z = 0.5 * double_Pi * cosarg * Global::sgn(z) * oOZssMinZba;
            }
            zss = zssNotAbs;
            d_fnlv = 1 - z * ((alph + q * dalph_v) * zss - dz_ss * alph * q) * oOZss * oOZss;
            d_fnlz = -q * oOZss * (z * dalph_z + alph);
            //            d_fnl = d_fnlv * K1 + d_fnlz * kHalf;
            
            dg1v = 2.0 / k + 2 * s0 + sig1 / (rho * A * h) * d_fnlv + sig2 / (rho * A * h);
            dg1z = sig0 / (rho * A * h) + sig1 / (rho * A * h) * d_fnlz;
            dg2v = d_fnlv;
            dg2z = d_fnlz - 2.0 / k;
            
            determ = dg1v * dg2z - dg1z * dg2v;
            qPrevIt = q;
            zPrevIt = z;
            q = q - (1 / determ) * (dg2z * g1 - dg1z * g2);
            z = z - (1 / determ) * (-dg2v * g1 + dg1v * g2);
            
            eps = sqrt((q-qPrevIt)*(q-qPrevIt) + (z-zPrevIt)*(z-zPrevIt));
            ++NRiterator;
        }
        if (NRiterator == 50)
        {
            ++limitCount;
            std::cout << _Fn.load() << " Limit! " << limitCount <<  std::endl;
        }
        //        std::cout << i << std::endl;
        calcZDot();
        
        zPrev = z;
        zDotPrev = zDot;
        anPrev = an;
    }
}

void TrombaString::mouseDown (const MouseEvent& e)
{
    if (bowing)
        return;
    xPos = e.x;
    yPos = e.y;
    exciteFlag = true;
}

void TrombaString::mouseDrag (const MouseEvent& e)
{
    if (!bowing)
        return;
	if (ModifierKeys::getCurrentModifiers() == ModifierKeys::leftButtonModifier + ModifierKeys::ctrlModifier)
	{
		_dampingFingerPos.store(e.x / static_cast<float>(getWidth()));
		std::cout << e.x / static_cast<float>(getWidth()) << std::endl;
	}
	else
    {
        if (getBowModel() == exponential)
            setBowingParameters (e.x, e.y, 0.05, 0.2, true);
        else if (getBowModel() == elastoPlastic)
            setBowingParameters (e.x, e.y, _Fn.load(), 0.2, true);
    }
}

void TrombaString::mouseUp (const MouseEvent& e)
{
    bowFlag = false;
}

void TrombaString::setBowingParameters (float x, float y, double Fb, double Vb, bool mouseInteraction)
{
#ifndef NOEDITOR
	xPos = x * (mouseInteraction ? 1 : getWidth());
	yPos = y * (mouseInteraction ? 1 : getHeight());
#else
	xPos = x * N;
	yPos = y * N;
#endif
	bowFlag = true;
	_Vb.store(Global::bowDebug || !mouseInteraction ? Vb : -(yPos / static_cast<float> (getHeight()) - 0.5) * 2.0 * 0.2);
	if (getBowModel() == exponential)
		_Fb.store (Fb);
	else if (getBowModel() == elastoPlastic)
		setFn (Fb);
#ifdef NOEDITOR
	int loc = xPos;
#else
	int loc = Global::bowDebug ? floor(N * 0.5) : floor(N * static_cast<float> (xPos) / static_cast<float> (getWidth()));
#endif
    _bowPos.store (Global::clamp (loc, 3, N - 5)); // check whether these values are correct!!);
}

void TrombaString::calcZDot()
{
    espon = Global::exp1 (-((q * q) * oOstrvSq));         //exponential function
    zss = Global::sgn(q) * (fC + (fS - fC) * espon) * oOSig0;   //steady state curve: z_ss(v)
    //            std::cout << zss << std::endl;
    if (q==0)
        zss = fS * oOSig0;
    
    // elasto-plastic function \alph (v,z)
    alph=0;
    
    oOZss = 1 / zss; // should use the absolute zss
    zssNotAbs = zss;
    zss = abs(zss);
    
    oOZssMinZba = 1 / (zss - z_ba); // should use the absolute zss
    
    if (Global::sgn(z)==Global::sgn(q))
    {
        if ((abs(z)>z_ba) && (abs(z)<zss))
        {
            arg = double_Pi * (z - Global::sgn(z) * 0.5 * (zss + z_ba)) * oOZssMinZba;
            alph = 0.5 * (1 + sin(Global::sgn(z) * arg));
        }
        else if (abs(z)>=zss)
        {
            alph=1;
        }
    }
    zss = zssNotAbs;
    an = 2.0 / k * (z - zPrev) - anPrev;
    
    // non-linear function estimate
    zDot = q * (1 - alph * z * oOZss);
}

void TrombaString::reset()
{
    for (int i = 0; i < uVecs.size(); ++i)
    {
        for (int j = 0; j < uVecs[i].size(); ++j)
        {
            uVecs[i][j] = offset;
        }
    }
    q = 0;
    qPrev = 0;
    z = 0;
    zPrev = 0;
    zDotPrev = 0;
    anPrev = 0;
}
