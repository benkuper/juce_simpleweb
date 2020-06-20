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
#include "websocket/server_ws.hpp"
#include "webserver/server_http.hpp"

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using HttpServer= SimpleWeb::Server<SimpleWeb::HTTP>;

class SimpleWebSocket :
    public Thread
{
public:

	SimpleWebSocket();
	~SimpleWebSocket();


	File rootPath;
	int port;

	void start(int port = 8080);

	void send(const String& message);
	void send(const MemoryBlock& data);
	void sendTo(const String& message, const String& id);
	void sendTo(const MemoryBlock& data, const String& id);
	void sendExclude(const String& message, const StringArray excludeIds);
	void sendExclude(const MemoryBlock& data, const StringArray excludeIds);

	void stop();
	void closeConnection(const String& id, int code = 0, const String &reason = "YouKnowWhy");

	void run() override;

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

	class RequestHandler
	{
	public:
		virtual ~RequestHandler() {}
		virtual String handleHTTPRequest(const String& request) = 0;
	};

	RequestHandler* handler;

protected:
	WsServer ws;
	HttpServer http;
	
	std::shared_ptr<asio::io_service> ioService;
	HashMap<String, std::shared_ptr<WsServer::Connection>> connectionMap;

	const StringArray imageExtensions{"css","csv","html","javascrit","xml"};
	const StringArray appExtensions{ "ogg","pdf","json","xml", "zip" };
	const StringArray videoExtensions{"mpeg","mp4","webm" };


	void onMessageCallback(std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message);
	void onNewConnectionCallback(std::shared_ptr<WsServer::Connection> connection);
	void onConnectionCloseCallback(std::shared_ptr<WsServer::Connection> connection, int status, const std::string& /*reason*/);
	void onErrorCallback(std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code& ec);

	void onHTTPUpgrade(std::unique_ptr<SimpleWeb::HTTP>& socket, std::shared_ptr<HttpServer::Request> request);

	void httpDefaultCallback(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);

	String getConnectionString(std::shared_ptr<WsServer::Connection> connection) const;
};
