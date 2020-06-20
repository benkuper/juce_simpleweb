/*
  ==============================================================================

    juce_SimpleWebSocket.h
    Created: 17 Jun 2020 11:22:54pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#define USE_STANDALONE_ASIO 1
#define NOGDI
#define ASIO_DISABLE_SERIAL_PORT 1
#include "websocket/client_ws.hpp"
//#include "webserver/server_http.hpp"

using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;
//using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

class SimpleWebSocketClient :
	public Thread
{
public:

	SimpleWebSocketClient();
	~SimpleWebSocketClient();

	String serverPath;
	bool isConnected;

	void start(const String & serverPath);

	void send(const String& message);
	void send(const MemoryBlock& data);

	void stop();

	void run() override;

	class  Listener
	{
	public:
		/** Destructor. */
		virtual ~Listener() {}
		virtual void connectionOpened() {}
		virtual void messageReceived(const String& message) {}
		virtual void connectionClosed(int status, const String& reason) {}
		virtual void connectionError(const String& message) {}
	};



	ListenerList<Listener> webSocketListeners;
	void addWebSocketListener(Listener* newListener) { webSocketListeners.add(newListener); }
	void removeWebSocketListener(Listener* listener) { webSocketListeners.remove(listener); }


protected:
	std::unique_ptr<WsClient> ws;
	std::shared_ptr<WsClient::Connection> connection;

	void onMessageCallback(std::shared_ptr<WsClient::Connection> connection, std::shared_ptr<WsClient::InMessage> in_message);
	void onNewConnectionCallback(std::shared_ptr<WsClient::Connection> connection);
	void onConnectionCloseCallback(std::shared_ptr<WsClient::Connection> connection, int status, const std::string& /*reason*/);
	void onErrorCallback(std::shared_ptr<WsClient::Connection> connection, const SimpleWeb::error_code& ec);
};
