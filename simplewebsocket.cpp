#include "SimpleWebSocket.h"
/*
  ==============================================================================

	juce_SimpleWebSocket.cpp
	Created: 17 Jun 2020 11:22:54pm
	Author:  bkupe

  ==============================================================================
*/

SimpleWebSocket::SimpleWebSocket() :
	Thread("Web socket"),
	port(0),
	handler(nullptr)
{

}

SimpleWebSocket::~SimpleWebSocket()
{
	stop();
}

void SimpleWebSocket::start(int _port)
{
	port = _port;
	startThread();
}

void SimpleWebSocket::send(const String& message, const String& connectionId)
{
	if (connectionId.isNotEmpty())
	{
		if (connectionMap.contains(connectionId)) connectionMap[connectionId]->send(message.toStdString());
		else
		{
			DBG("Not found !");
		}
		return;
	}
	for (auto& c : connectionMap)
	{
		c->send(message.toStdString());
	}

}

void SimpleWebSocket::stop()
{
	http.stop();
	if (Thread::getCurrentThreadId() != this->getThreadId()) stopThread(100);
}

void SimpleWebSocket::closeConnection(const String& id, int status, const String& reason)
{
	if (!connectionMap.contains(id)) return;
	connectionMap[id]->send_close(status, reason.toStdString());
}

void SimpleWebSocket::run()
{
	//HTTP init
	ioService = std::make_shared<asio::io_service>();
	http.config.port = port;
	http.io_service = ioService;

	http.default_resource["GET"] = std::bind(&SimpleWebSocket::httpDefaultCallback, this, std::placeholders::_1, std::placeholders::_2);
	http.on_upgrade = std::bind(&SimpleWebSocket::onHTTPUpgrade, this, std::placeholders::_1, std::placeholders::_2);

	//WebSocket init
	auto& wsEndpoint = ws.endpoint["^/ws/?$"];
	
	wsEndpoint.on_message = std::bind(&SimpleWebSocket::onMessageCallback, this, std::placeholders::_1, std::placeholders::_2);
	wsEndpoint.on_error = std::bind(&SimpleWebSocket::onErrorCallback, this, std::placeholders::_1, std::placeholders::_2);
	wsEndpoint.on_open = std::bind(&SimpleWebSocket::onNewConnectionCallback, this, std::placeholders::_1);
	wsEndpoint.on_close = std::bind(&SimpleWebSocket::onConnectionCloseCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	DBG("Start HTTP");
	http.start();

	DBG("Start io service");
	ioService->run();

	DBG("HTTP Service started on " << port);
}


void SimpleWebSocket::onMessageCallback(std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message)
{
	String id = getConnectionString(connection);
	webSocketListeners.call(&Listener::messageReceived, id, String(in_message->string()));
}

void SimpleWebSocket::onNewConnectionCallback(std::shared_ptr<WsServer::Connection> connection)
{
	String id = getConnectionString(connection);
	connectionMap.set(id, connection);
	webSocketListeners.call(&Listener::connectionOpened, id);
}

void SimpleWebSocket::onConnectionCloseCallback(std::shared_ptr<WsServer::Connection> connection, int status, const std::string& reason)
{
	String id = getConnectionString(connection);
	connectionMap.remove(id);
	webSocketListeners.call(&Listener::connectionClosed, id, status, reason);
}

void SimpleWebSocket::onErrorCallback(std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code& ec)
{
	String id = getConnectionString(connection);
	webSocketListeners.call(&Listener::connectionError, id, ec.message());
}

void SimpleWebSocket::onHTTPUpgrade(std::unique_ptr<SimpleWeb::HTTP>& socket, std::shared_ptr<HttpServer::Request> request)
{
	DBG("Http updgrade here");
	auto connection = std::make_shared<WsServer::Connection>(std::move(socket));
	connection->method = std::move(request->method);
	connection->path = std::move(request->path);
	connection->http_version = std::move(request->http_version);
	connection->header = std::move(request->header);
	ws.upgrade(connection);
}

void SimpleWebSocket::httpDefaultCallback(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
{
	DBG("Http callback here");
	String html = "Sorry not sorry";
	if (handler != nullptr) html = handler->handleHTTPRequest(request->content.string());
	else if (rootPath.exists() && rootPath.isDirectory())
	{
		String path = request->path.substr(1);
		if (path.isEmpty()) path = "index.html";
		File f = rootPath.getChildFile(path); //substr to remove the first "/"
		if (f.existsAsFile()) html = f.loadFileAsString();
	}

	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << html.length() << "\r\n\r\n" << html;
}

String SimpleWebSocket::getConnectionString(std::shared_ptr<WsServer::Connection> connection) const
{
	return String(connection->remote_endpoint().address().to_string()) + ":" + String(connection->remote_endpoint().port());
}