/*
  ==============================================================================

    Body.cpp
    Created: 21 Oct 2019 4:49:29pm
    Author:  Silvin Willemsen

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "Body.h"

//==============================================================================
Body::Body (NamedValueSet& parameters, double k) :  k (k),
                                                    rho (*parameters.getVarPointer("rhoP")),
                                                    H (*parameters.getVarPointer("H")),
                                                    E (*parameters.getVarPointer("EP")),
                                                    s0 (*parameters.getVarPointer("s0P")),
                                                    s1 (*parameters.getVarPointer("s1P")),
                                                    Lx (*parameters.getVarPointer("Lx")),
                                                    Ly (*parameters.getVarPointer("Ly"))
{
    double Dkappa = E * H * H * H / (12.0 * (1.0-0.3 * 0.3));
    kappaSq = Dkappa / (rho * H);
    
    h = 2.0 * sqrt(k * (s1 + sqrt(kappaSq + s1 * s1)));
    
    // lower limit on h, otherwise there will be too many points
    if (h < 0.02)
        h = 0.02;
    
//    // Scale damping by rho * A
//    s0 = s0 * rho * H;
//    s1 = s1 * rho * H;
    
    Nx = floor(Lx/h);
    Ny = floor(Ly/h);
    
    h = std::max (Lx/Nx, Ly/Ny);
    N = Nx * Ny;
    
    // initialise state vectors
    uVecs.reserve (3);
    
    for (int i = 0; i < 3; ++i)
        uVecs.push_back (std::vector<double> (N, 0));
    u.resize (3);
    for (int i = 0; i < u.size(); ++i)
        u[i] = &uVecs[i][0];

    
    D = 1.0f / (1.0f + s0 * k);
    B1 = (kappaSq * k * k) / (h * h * h * h);
    B2 = (2.0f * s1 * k) / (h * h);
    
    A1 = (2.0 - 4.0 * B2 - 20.0 * B1);
    A2 = 8.0 * B1;
    A3 = -2.0 * B1;
    A4 = -B1;
    A5 = (s0 * k - 1.0 + 4.0 * B2);
    A6 = -B2;
    
    A1 *= D;
    A2 *= D;
    A3 *= D;
    A4 *= D;
    A5 *= D;
    A6 *= D;
    
    excitationWidth = floor (std::min(Nx, Ny) * 0.2);
}

Body::~Body()
{
}

void Body::paint (Graphics& g)
{
    int startIdx = 2;
    float stateWidth = getWidth() / static_cast<double> (Nx - 2 * startIdx);
    float stateHeight = getHeight() / static_cast<double> (Ny - 2 * startIdx);
    int scaling = Global::outputScaling * 100.0;
    
    for (int x = startIdx; x < Nx; ++x)
    {
        for (int y = startIdx; y < Ny; ++y)
        {
            int cVal = Global::clamp (255 * 0.5 * (u[1][x + Nx * y] * scaling + 1), 0, 255);
            g.setColour (Colour::fromRGB (cVal, cVal, cVal));
            g.fillRect ((x - startIdx) * stateWidth, (y - startIdx) * stateHeight, stateWidth, stateHeight);
        }
    }
    g.setColour (Colours::yellow);
}

void Body::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}

void Body::calculateUpdateEq()
{
    for (int l = 2; l < Nx - 2; ++l)
    {
        for (int m = 2; m < Ny - 2; ++m)
        {
            u[0][l+m * Nx] =
            A1 * u[1][l+m * Nx]
            + A2 * (u[1][l + (m+1) * Nx] + u[1][l + (m-1) * Nx] + u[1][l+1 + m * Nx] + u[1][l-1 + m * Nx] )
            + A3 * (u[1][l+1 + (m+1) * Nx] + u[1][l-1 + (m+1) * Nx] + u[1][l+1 + (m-1) * Nx] + u[1][l-1 + (m-1) * Nx])
            + A4 * (u[1][l + (m+2) * Nx] + u[1][l + (m-2) * Nx] + u[1][l+2 + m * Nx] + u[1][l-2 + m * Nx])
            + A5 * u[2][l + m * Nx]
            + A6 * (u[2][l + (m+1) * Nx] + u[2][l + (m-1) * Nx] + u[2][l+1 + m * Nx] + u[2][l-1 + m * Nx]);
        }
    }
}

//    for (int l = 2; l < Nx - 2; l++)
//    {
//        for (int m = 2; m < Ny - 2; m++)
//        {
//            u[0][l][m] = A1 * u[1][l][m]
//                        + A2 * (u[1][l][m + 1] + u[1][l][m - 1] + u[1][l + 1][m] + u[1][l - 1][m])
//                        + A3 * (u[1][l + 1][m + 1] + u[1][l - 1][m + 1] + u[1][l + 1][m - 1] + u[1][l - 1][m - 1])
//                        + A4 * (u[1][l][m + 2] + u[1][l][m - 2] + u[1][l + 2][m] + u[1][l - 2][m])
//                        + A5 * u[2][l][m]
//                        + A6 * (u[2][l][m + 1] + u[2][l][m - 1] + u[2][l + 1][m] + u[2][l - 1][m]);
//        }
//    }

void Body::updateStates()
{
    uTmp = u[2];
    
    u[2] = u[1];
    u[1] = u[0];
    u[0] = uTmp;
}

void Body::excite()
{
    exciteFlag = false;

    excitationWidth = Global::clamp (excitationWidth, 1, Nx - 5);
    std::vector<std::vector<double>> excitationArea (excitationWidth, std::vector<double> (excitationWidth, 0));
    
    for (int i = 1; i < excitationWidth; ++i)
    {
        for (int j = 1; j < excitationWidth; ++j)
        {
            excitationArea[i][j] = (10.0 / (excitationWidth * excitationWidth) * 0.25 * (1 - cos(2.0 * double_Pi * i / static_cast<int>(excitationWidth+1))) * (1 - cos(2.0 * double_Pi * j / static_cast<int>(excitationWidth+1)))) / (Global::outputScaling * 100.0);
        }
    }
    
    int startIdX = Global::clamp (idX - excitationWidth * 0.5, 2, Nx - 3 - excitationWidth);
    int startIdY = Global::clamp (idY - excitationWidth * 0.5, 2, Ny - 3 - excitationWidth);
    
    if (Global::debug)
    {
        u[1][5 + Nx * 5] += 1;
        u[2][5 + Nx * 5] += 1;
    } else {
        for (int i = 1; i < excitationWidth; ++i)
        {
            for (int j = 1; j < excitationWidth; ++j)
            {
                u[1][i + startIdX + (j + startIdY) * Nx] += excitationArea[i][j];
                u[2][i + startIdX + (j + startIdY) * Nx] += excitationArea[i][j];
            }
        }
    }
    
}

void Body::mouseDrag (const MouseEvent& e)
{
    exciteFlag = true;
    
    idX = Nx * (e.x / static_cast<float> (getWidth()));
    idY = Ny * (e.y / static_cast<float> (getHeight()));
}
