#include "WebSocket.h"
/*
  ==============================================================================

    juce_SimpleWebSocket.cpp
    Created: 17 Jun 2020 11:22:54pm
    Author:  bkupe

  ==============================================================================
*/

WebSocket::WebSocket() :
    Thread("Web socket"),
    port(0)
{
    ws.onNewConnectionFunc = std::bind(&WebSocket::onConnect, this, std::placeholders::_1);
    ws.onConnectionCloseFunc = std::bind(&WebSocket::onClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);;
    ws.onMessageFunc = std::bind(&WebSocket::onMessage, this, std::placeholders::_1, std::placeholders::_2);;
    ws.onErrorFunc = std::bind(&WebSocket::onError, this, std::placeholders::_1, std::placeholders::_2);;
}

WebSocket::~WebSocket()
{
    stop();
}

void WebSocket::start(int port)
{
    startThread();
}

void WebSocket::send(const String& message, const String& connectionId)
{
    ws.send(message.toStdString(), connectionId.toStdString());
}

void WebSocket::stop()
{
    ws.stop();
}

void WebSocket::closeConnection(const String& id)
{
    ws.closeConnection(id.toStdString());
}

void WebSocket::run()
{
    ws.start();
    while (!threadShouldExit()) {}
}

void WebSocket::onConnect(const std::string& id)
{
    webSocketListeners.call(&Listener::connectionOpened, id);
}

void WebSocket::onMessage(const std::string& id, const std::string& message)
{
    webSocketListeners.call(&Listener::messageReceived, id, message);

}

void WebSocket::onClose(const std::string& id, int status, const std::string& reason)
{
    webSocketListeners.call(&Listener::connectionClosed, id,status, reason);
}

void WebSocket::onError(const std::string& id, const std::string& message)
{
    webSocketListeners.call(&Listener::connectionError, id, message);
}
