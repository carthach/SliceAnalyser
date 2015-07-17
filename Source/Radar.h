/*
  ==============================================================================

    Radar.h
    Created: 14 Jul 2015 11:11:54am
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#ifndef RADAR_H_INCLUDED
#define RADAR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class Radar    : public Component
{
public:
    Radar();
    ~Radar();

    void paint (Graphics&);
    void resized();

private:
    GlowEffect glowEffect;
    
    Point<int> radarPosition;
    
    float glowEffectRadius = 0.0f;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Radar)
};


#endif  // RADAR_H_INCLUDED
