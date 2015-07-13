/*
  ==============================================================================

    Visualisation.h
    Created: 13 Jul 2015 6:20:58pm
    Author:  Cárthach Ó Nuanáin

  ==============================================================================
*/

#ifndef VISUALISATION_H_INCLUDED
#define VISUALISATION_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Element.h"

//==============================================================================
/*
*/
class Visualisation    : public Component
{
public:
    Visualisation()
    {
        // In your constructor, you should add any child components, and
        // initialise any special settings that your component needs.
        
        for(int i=0; i<50; i++) {
            Colour c(random.nextInt(255), random.nextInt(255), random.nextInt(255));
            
            elements.add(new Element(c));
            addAndMakeVisible(elements[i]);
            elements[i]->setBounds(random.nextInt(getParentWidth()-50.0), random.nextInt(getParentHeight()-50.0), 50.0, 50.0);
        }
    }

    ~Visualisation()
    {
    }

    void paint (Graphics& g)
    {
        /* This demo code just fills the component's background and
           draws some placeholder text to get you started.

           You should replace everything in this method with your own
           drawing code..
        */

//        g.fillAll (Colours::black);   // clear the background
//
//        g.setColour (Colours::grey);
//        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
//
//        g.setColour (Colours::lightblue);
//        g.setFont (14.0f);
//        g.drawText ("Visualisation", getLocalBounds(),
//                    Justification::centred, true);   // draw some placeholder text
        
//        for(int i=0; i<elements.size(); i++)
//            elements[i]->paintEntireComponent(g, true);
    }

    void resized()
    {
        // This method is where you should set the bounds of any child
        // components that your component contains..
        
    }

private:
    Random random;
    OwnedArray<Element> elements;
    
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Visualisation)
};


#endif  // VISUALISATION_H_INCLUDED
