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

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

#if SIMPLEWEB_SECURE_SUPPORTED
using WssServer = SimpleWeb::SocketServer<SimpleWeb::WSS>;
using HttpsServer = SimpleWeb::Server<SimpleWeb::HTTPS>;
#endif

class SimpleWebSocketServerBase :
	public juce::Thread
{
public:
	SimpleWebSocketServerBase();
	virtual ~SimpleWebSocketServerBase();

	juce::File rootPath;
	int port;
	juce::String wsSuffix;
	bool isConnected;

	juce::CriticalSection serverLock;

	void start(int port = 8080, const juce::String& wsSuffix = "");

	virtual void send(const juce::String& message) {}
	virtual void send(const char* data, int numData) {}
	void send(const juce::MemoryBlock& data);
	virtual void sendTo(const juce::String& message, const juce::String& id) {}
	virtual void sendTo(const juce::MemoryBlock& data, const juce::String& id) {}
	virtual void sendExclude(const juce::String& message, const juce::StringArray excludeIds) {}
	virtual void sendExclude(const juce::MemoryBlock& data, const juce::StringArray excludeIds) {}

	void serveFile(const juce::File& file, std::shared_ptr<HttpServer::Response> response);
	void serveFile(const juce::File& file, std::shared_ptr<HttpsServer::Response> response);

	void stop();
	void closeConnection(const juce::String& id, int code = 1000, const juce::String& reason = "YouKnowWhy");

	virtual void stopInternal() {}
	virtual void closeConnectionInternal(const juce::String& id, int code, const juce::String& reason) {}

	virtual int getNumActiveConnections() const { return 0; }

	void run() override;

	virtual void initServer() {}



	class  Listener
	{
	public:
		/** Destructor. */
		virtual ~Listener() {}
		virtual void connectionOpened(const juce::String& id) {}
		virtual void messageReceived(const juce::String& id, const juce::String& message) {}
		virtual void dataReceived(const juce::String& id, const juce::MemoryBlock& data) {}
		virtual void connectionClosed(const juce::String& id, int status, const juce::String& reason) {}
		virtual void connectionError(const juce::String& id, const juce::String& message) {}
	};


	juce::ListenerList<Listener> webSocketListeners;
	void addWebSocketListener(Listener* newListener) { webSocketListeners.add(newListener); }
	void removeWebSocketListener(Listener* listener) { webSocketListeners.remove(listener); }

	class RequestHandler
	{
	public:
		virtual ~RequestHandler() {}
		virtual bool handleHTTPRequest(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request) { return false; };

#if SIMPLEWEB_SECURE_SUPPORTED
		virtual bool handleHTTPSRequest(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request) { return false; };
#endif
	};

	RequestHandler* handler;

};


class SimpleWebSocketServer :
	public SimpleWebSocketServerBase
{
public:
	SimpleWebSocketServer();
	~SimpleWebSocketServer();

	std::unique_ptr<WsServer> ws;
	std::unique_ptr<HttpServer> http;

	std::shared_ptr<asio::io_service> ioService;
	juce::HashMap<juce::String, std::shared_ptr<WsServer::Connection>, juce::DefaultHashFunctions, juce::CriticalSection> connectionMap;

	virtual void send(const juce::String& message) override;
	virtual void send(const char* data, int numData) override;
	virtual void sendTo(const juce::String& message, const juce::String& id) override;
	virtual void sendTo(const juce::MemoryBlock& data, const juce::String& id) override;
	virtual void sendExclude(const juce::String& message, const juce::StringArray excludeIds) override;
	virtual void sendExclude(const juce::MemoryBlock& data, const juce::StringArray excludeIds) override;

	virtual void stopInternal() override;
	virtual void closeConnectionInternal(const juce::String& id, int code, const juce::String& reason) override;

	void initServer() override;

	void onMessageCallback(std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message);
	void onNewConnectionCallback(std::shared_ptr<WsServer::Connection> connection);
	void onConnectionCloseCallback(std::shared_ptr<WsServer::Connection> connection, int status, const std::string& /*reason*/);
	void onErrorCallback(std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code& ec);

	void httpStartCallback(unsigned short port);
	void onHTTPUpgrade(std::unique_ptr<SimpleWeb::HTTP>& socket, std::shared_ptr<HttpServer::Request> request);

	void httpDefaultCallback(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);
	juce::String getConnectionString(std::shared_ptr<WsServer::Connection> connection) const;


	virtual int getNumActiveConnections() const override;
};


#if SIMPLEWEB_SECURE_SUPPORTED

class SecureWebSocketServer :
	public SimpleWebSocketServerBase
{
public:
	SecureWebSocketServer(const juce::String& certFile, const juce::String& privateKeyFile, const juce::String& verifyFile = juce::String());
	~SecureWebSocketServer();

	juce::String certFile;
	juce::String keyFile;
	juce::String verifyFile;

	std::unique_ptr<WssServer> ws;
	std::unique_ptr<HttpsServer> http;

	std::shared_ptr<asio::io_service> ioService;
	juce::HashMap<juce::String, std::shared_ptr<WssServer::Connection>> connectionMap;

	virtual void send(const juce::String& message) override;
	virtual void send(const char* data, int numData) override;
	virtual void sendTo(const juce::String& message, const juce::String& id) override;
	virtual void sendTo(const juce::MemoryBlock& data, const juce::String& id) override;
	virtual void sendExclude(const juce::String& message, const juce::StringArray excludeIds) override;
	virtual void sendExclude(const juce::MemoryBlock& data, const juce::StringArray excludeIds) override;

	virtual void stopInternal() override;
	virtual void closeConnectionInternal(const juce::String& id, int code, const juce::String& reason) override;


	void initServer() override;

	void onMessageCallback(std::shared_ptr<WssServer::Connection> connection, std::shared_ptr<WssServer::InMessage> in_message);
	void onNewConnectionCallback(std::shared_ptr<WssServer::Connection> connection);
	void onConnectionCloseCallback(std::shared_ptr<WssServer::Connection> connection, int status, const std::string& /*reason*/);
	void onErrorCallback(std::shared_ptr<WssServer::Connection> connection, const SimpleWeb::error_code& ec);

	void httpStartCallback(unsigned short port);
	void onHTTPUpgrade(std::unique_ptr<SimpleWeb::HTTPS>& socket, std::shared_ptr<HttpsServer::Request> request);

	void httpDefaultCallback(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request);
	juce::String getConnectionString(std::shared_ptr<WssServer::Connection> connection) const;

	virtual int getNumActiveConnections() const override;
};

#endif
