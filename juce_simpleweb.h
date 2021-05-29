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

  linuxLibs:        ssl,crypto
  OSXLibs:          libssl,libcrypto
  windowsLibs:      libssl,libcrypto
  
 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_SIMPLEWEB_H_INCLUDED

//==============================================================================
#ifdef _MSC_VER
 #pragma warning (push)
 // Disable warnings for long class names, padding, and undefined preprocessor definitions.
 #pragma warning (disable: 4251 4786 4668 4820 4100 4267)
#endif


#define USE_STANDALONE_ASIO 1
#define NOGDI
#define ASIO_DISABLE_SERIAL_PORT 1

#include <juce_core/juce_core.h>
#include <juce_cryptography/juce_cryptography.h>

//#ifndef __arm__
#define SIMPLEWEB_SECURE_SUPPORTED 1
//#else
//#define SIMPLEWEB_SECURE_SUPPORTED 0
//#endif

#if SIMPLEWEB_SECURE_SUPPORTED
#define _WIN32_WINDOWS 0x601
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "common/crypto.hpp"
#endif

#include "common/WSCrypto.h" //remove openssl dep for non supported OS

#include <juce_events/juce_events.h>

using namespace juce;

#if SIMPLEWEB_SECURE_SUPPORTED
#include "openssl/ssl.h"
#include "websocket/server_wss.hpp"
#include "webserver/server_https.hpp"
#include "websocket/client_wss.hpp"
#else
#include "websocket/server_ws.hpp"
#include "webserver/server_http.hpp"
#include "websocket/client_ws.hpp"
#endif

#include "SimpleWebSocketServer.h"
#include "SimpleWebSocketClient.h"
#include "MIMETypes.h"
