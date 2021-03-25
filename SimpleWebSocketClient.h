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

using WsClient = SimpleWeb::SocketClient<SimpleWeb::WS>;

class SimpleWebSocketClientBase :
	public Thread
{
public:
	SimpleWebSocketClientBase();

	virtual ~SimpleWebSocketClientBase();

	String serverPath;
	bool isConnected;
	bool isClosing;

	virtual void start(const String& _serverPath);

	virtual void send(const String& message) {}
	virtual void send(const char* data, int numData) {}

	void send(const MemoryBlock& data);

	void stop();
	virtual void stopInternal() {}
	virtual void run();

	virtual void initWS() {}


	void handleNewConnectionCallback();
	void handleConnectionClosedCallback(int status, const String &reason);
	void handleErrorCallback(const String& message);

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void connectionOpened() {}
		virtual void messageReceived(const String& message) {}
		virtual void dataReceived(const MemoryBlock& data) {}
		virtual void connectionClosed(int status, const String& reason) {}
		virtual void connectionError(const String& message) {}
	};

	ListenerList<Listener> webSocketListeners;
	void addWebSocketListener(Listener* newListener) { webSocketListeners.add(newListener); }
	void removeWebSocketListener(Listener* listener) { webSocketListeners.remove(listener); }
};

class SimpleWebSocketClient :
	public SimpleWebSocketClientBase
{

public:

	std::unique_ptr<WsClient> ws;
	std::shared_ptr<WsClient::Connection> connection;

	SimpleWebSocketClient();
	~SimpleWebSocketClient();

	void send(const String& message) override;
	void send(const char* data, int numData) override;
	void stopInternal() override;

	void initWS() override;

	void onMessageCallback(std::shared_ptr<WsClient::Connection> connection, std::shared_ptr<WsClient::InMessage> in_message);
	void onNewConnectionCallback(std::shared_ptr<WsClient::Connection> _connection);
	void onConnectionCloseCallback(std::shared_ptr<WsClient::Connection> /*_connection*/, int status, const std::string& reason);
	void onErrorCallback(std::shared_ptr<WsClient::Connection> /*_connection*/, const SimpleWeb::error_code& ec);
};

#if SIMPLEWEB_SECURE_SUPPORTED
using WssClient = SimpleWeb::SocketClient<SimpleWeb::WSS>;

class SecureWebSocketClient :
	public SimpleWebSocketClientBase
{

public:
	std::unique_ptr<WssClient> ws;
	std::shared_ptr<WssClient::Connection> connection;

	SecureWebSocketClient();
	~SecureWebSocketClient();

	void send(const String& message) override;
	void send(const char* data, int numData) override;
	void stopInternal() override;

	void initWS() override;

	void onMessageCallback(std::shared_ptr<WssClient::Connection> connection, std::shared_ptr<WssClient::InMessage> in_message);
	void onNewConnectionCallback(std::shared_ptr<WssClient::Connection> _connection);
	void onConnectionCloseCallback(std::shared_ptr<WssClient::Connection> /*_connection*/, int status, const std::string& reason);
	void onErrorCallback(std::shared_ptr<WssClient::Connection> /*_connection*/, const SimpleWeb::error_code& ec);
};
#endif
