/*
  ==============================================================================

  ==============================================================================
*/


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_simpleweb
  vendor:           benkuper
  version:          1.0.0
  name:             SimpleWeb
  description:      SimpleWebServer & SimpleWebSocket
  website:          https://github.com/benkuper/juce_simpleweb
  license:          GPLv3

  linuxLibs:        
  OSXLibs:          
  windowsLibs:      
  
 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_SIMPLEWEB_H_INCLUDED

//==============================================================================
#ifdef _MSC_VER
 #pragma warning (push)
 // Disable warnings for long class names, padding, and undefined preprocessor definitions.
 #pragma warning (disable: 4251 4786 4668 4820)
#endif

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>
#include <juce_gui_basics/juce_gui_basics.h>


using namespace juce;

#include "SimpleWebSocket.h"
//#include "SimpleWebServer.h"