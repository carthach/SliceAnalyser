/*
  ==============================================================================

    Radar.cpp
    Created: 14 Jul 2015 11:11:54am
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "Radar.h"

//==============================================================================
Radar::Radar() : radarPosition(0,0)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.

    setComponentEffect(&glowEffect);

    glowEffectRadius = 5.0;

    
    glowEffect.setGlowProperties(glowEffectRadius, Colours::limegreen);

    

}

Radar::~Radar()
{
    
}


void Radar::paint (Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (Colours::transparentBlack);   // clear the background
    
    
    g.setColour(Colours::white);

//    g.setColour (Colours::grey);
//    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
//
//    g.setColour (Colours::lightblue);
//    g.setFont (14.0f);
//    g.drawText ("Radar", getLocalBounds(),
//                Justification::centred, true);   // draw some placeholder text
    
//    int lineWidth = 1;
//    g.drawEllipse(lineWidth, lineWidth, getWidth()-lineWidth*2, getHeight()-lineWidth*2, lineWidth);
//    g.drawLine(getWidth()/2.0, getHeight()/2.0, getX(), getY());
    
    Path myPath;
    
    myPath.addPieSegment(0, 0, getWidth(), getHeight(), 0.4 * (2.0 * M_PI), 1.25 * (2.0 * M_PI), 0.0);
    
    
    
//    myPath.startNewSubPath (10.0f, 10.0f);          // move the current position to (10, 10)
//    myPath.lineTo (100.0f, 200.0f);                 // draw a line from here to (100, 200)
//    myPath.quadraticTo (0.0f, 150.0f, 5.0f, 50.0f); // draw a curve that ends at (5, 50)
//    myPath.closeSubPath();                          // close the subpath with a line back to (10, 10)
//    // add an ellipse as well, which will form a second sub-path within the path..
//    myPath.addEllipse (50.0f, 50.0f, 40.0f, 30.0f);
//    // double the width of the whole thing..
//    myPath.applyTransform (AffineTransform::scale (2.0f, 1.0f));
    // and draw it to a graphics context with a 5-pixel thick outline.
    g.strokePath (myPath, PathStrokeType (1.0f));
    
    radarPosition = getMouseXYRelative();
    g.drawLine(getWidth()/2.0, getHeight()/2.0, radarPosition.getX(), radarPosition.getY());
}

void Radar::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..

}
