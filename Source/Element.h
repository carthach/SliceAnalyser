/*
  ==============================================================================

    Element.h
    Created: 13 Jul 2015 6:17:27pm
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#ifndef ELEMENT_H_INCLUDED
#define ELEMENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class Element    : public Component
{
public:
    Element();
    Element(Colour colour);
    
    ~Element();

    void paint (Graphics&);
    void resized();
    
   Point<float> position;

private:
    Colour colour;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Element)
};


#endif  // ELEMENT_H_INCLUDED
