/*
  ==============================================================================

    juce_SimpleWebSocket.h
    Created: 17 Jun 2020 11:22:54pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include "simplewebsocket.h"

class WebSocket :
    public Thread
{
public:
	WebSocket();
	~WebSocket();

	int port;

	void start(int port = 8080);
	void send(const String& message, const String& connectionId = "");
	void stop();
	void closeConnection(const String& id);

	void run() override;

	SimpleWebSocket ws;

	void onConnect(const std::string& id);
	void onMessage(const std::string& id, const std::string& message);
	void onClose(const std::string& id, int status, const std::string& reason);
	void onError(const std::string& id, const std::string& message);

	class  Listener
	{
	public:
		/** Destructor. */
		virtual ~Listener() {}
		virtual void connectionOpened(const String &id) {}
		virtual void messageReceived(const String& id, const String &message) {}
		virtual void connectionClosed(const String& id, int status, const String &reason) {}
		virtual void connectionError(const String& id, const String & message) {}
	};

	ListenerList<Listener> webSocketListeners;
	void addWebSocketListener(Listener* newListener) { webSocketListeners.add(newListener); }
	void removeWebSocketListener(Listener* listener) { webSocketListeners.remove(listener); }
};
