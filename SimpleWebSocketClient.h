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
	public juce::Thread
{
public:
	SimpleWebSocketClientBase();

	virtual ~SimpleWebSocketClientBase();

	juce::String serverPath;
	bool isConnected;
	bool isClosing;

	virtual void start(const juce::String& _serverPath);

	virtual void send(const juce::String& message) {}
	virtual void send(const char* data, int numData) {}

	void send(const juce::MemoryBlock& data);

	void stop();
	virtual void stopInternal() {}
	virtual void run();

	virtual void initWS() {}


	void handleNewConnectionCallback();
	void handleConnectionClosedCallback(int status, const juce::String& reason);
	void handleErrorCallback(const juce::String& message);

	class Listener
	{
	public:
		virtual ~Listener() {}
		virtual void connectionOpened() {}
		virtual void messageReceived(const juce::String& message) {}
		virtual void dataReceived(const juce::MemoryBlock& data) {}
		virtual void connectionClosed(int status, const juce::String& reason) {}
		virtual void connectionError(const juce::String& message) {}
	};

	juce::ListenerList<Listener> webSocketListeners;
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

	void send(const juce::String& message) override;
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

	void send(const juce::String& message) override;
	void send(const char* data, int numData) override;
	void stopInternal() override;

	void initWS() override;

	void onMessageCallback(std::shared_ptr<WssClient::Connection> connection, std::shared_ptr<WssClient::InMessage> in_message);
	void onNewConnectionCallback(std::shared_ptr<WssClient::Connection> _connection);
	void onConnectionCloseCallback(std::shared_ptr<WssClient::Connection> /*_connection*/, int status, const std::string& reason);
	void onErrorCallback(std::shared_ptr<WssClient::Connection> /*_connection*/, const SimpleWeb::error_code& ec);
};
#endif
