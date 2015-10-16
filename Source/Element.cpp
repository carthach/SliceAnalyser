/*
  ==============================================================================

    Element.cpp
    Created: 13 Jul 2015 6:17:27pm
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#include "../JuceLibraryCode/JuceHeader.h"
#include "Element.h"

//C++11 Chaining!
//==============================================================================
Element::Element() : Element::Element(Colours::black)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
}

Element::Element(Colour colour)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    
    
    this->colour = colour;
}

Element::~Element()
{
}

void Element::paint (Graphics& g)
{
    /* This demo code just fills the component's background and
       draws some placeholder text to get you started.

       You should replace everything in this method with your own
       drawing code..
    */

    g.fillAll (Colours::transparentWhite);   // clear the background

//    g.setColour (Colours::grey);
//    g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
//
//    g.setColour (Colours::lightblue);
//    g.setFont (14.0f);
//    g.drawText ("Element", getLocalBounds(),
//                Justification::centred, true);   // draw some placeholder text
    
    g.setColour(colour);
    g.fillEllipse(0, 0, getWidth(), getHeight());
}

void Element::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
}