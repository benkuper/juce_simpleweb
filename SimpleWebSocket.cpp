
/*
  ==============================================================================

	juce_SimpleWebSocket.cpp
	Created: 17 Jun 2020 11:22:54pm
	Author:  bkupe

  ==============================================================================
*/

#include "MIMETypes.h"

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

void SimpleWebSocket::send(const String& message)
{
    HashMap<String, std::shared_ptr<WsServer::Connection>>::Iterator it(connectionMap);
    while (it.next())
    {
		it.getValue()->send(message.toStdString());
	}

}

void SimpleWebSocket::send(const MemoryBlock& data)
{
	std::shared_ptr<WsServer::OutMessage> out_message = std::make_shared<WsServer::OutMessage>();
	out_message->write((const char*)data.getData(), data.getSize()); 
    HashMap<String, std::shared_ptr<WsServer::Connection>>::Iterator it(connectionMap);
    while (it.next())
    {
		it.getValue()->send(out_message, nullptr, 130); //130 = binary
	}
}

void SimpleWebSocket::sendTo(const String& message, const String& id)
{
	if (connectionMap.contains(id)) connectionMap[id]->send(message.toStdString());
	else
	{
		DBG("[Dashboard] Websocket connection not found : " << id);
	}
}

void SimpleWebSocket::sendTo(const MemoryBlock& data, const String& id)
{
	std::shared_ptr<WsServer::OutMessage> out_message = std::make_shared<WsServer::OutMessage>();
	out_message->write((const char*)data.getData(), data.getSize());
	if (connectionMap.contains(id)) connectionMap[id]->send(out_message, nullptr, 130); //130 = binary
	else
	{
		DBG("[Dashboard] Websocket connection not found : " << id);
	}
}

void SimpleWebSocket::sendExclude(const String& message, const StringArray excludeIds)
{
	HashMap<String, std::shared_ptr<WsServer::Connection>>::Iterator it(connectionMap);
	while(it.next())
	{
		if(excludeIds.contains(it.getKey())) continue;
		it.getValue()->send(message.toStdString());
	}
}

void SimpleWebSocket::sendExclude(const MemoryBlock& data, const StringArray excludeIds)
{
	std::shared_ptr<WsServer::OutMessage> out_message = std::make_shared<WsServer::OutMessage>();
	out_message->write((const char* )data.getData(), data.getSize());
	
	HashMap<String, std::shared_ptr<WsServer::Connection>>::Iterator it(connectionMap);
	while (it.next())
	{
		if (excludeIds.contains(it.getKey())) continue;
		it.getValue()->send(out_message, nullptr, 130); //130 = binary
	}
}

void SimpleWebSocket::stop()
{
	ioService->stop();
	http.stop();
	ws.stop();
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
	if (handler != nullptr)
	{
		*response << handler->handleHTTPRequest(request->content.string());
		return;
	}
	
	if (rootPath.exists() && rootPath.isDirectory())
	{
		String content = "";
		String contentType = "text/html";
		int contentSize = 0;
		File f; 
		
		String path = request->path.substr(1);
		if (path.isEmpty()) path = "index.html";
		f = rootPath.getChildFile(path); //substr to remove the first "/"

		if (f.exists() && f.isDirectory()) f = f.getChildFile("index.html");

		if (f.existsAsFile())
		{
			contentSize = (int)f.getSize();
			contentType = MIMETypes::getMIMEType(f.getFileExtension());
			content = f.loadFileAsString();

			String result = "HTTP/1.1 200 OK";

			if (contentSize == content.length()) result += "\r\nContent-Length: " + String(contentSize);
			else
			{
				DBG("Content size and length mismatch (" << f.getFullPathName() << ") :" << contentSize << " <> " << content.length());
			}
			result += "\r\nContent-Type: " + contentType;
			result += "\r\n\r\n";

			result += content;

			*response << result;
			return;
		}
		else
		{
			DBG("WebServer requested file not found : " << f.getFullPathName());
		}
	}

	*response << "HTTP/1.1 404 Not Found";
}

String SimpleWebSocket::getConnectionString(std::shared_ptr<WsServer::Connection> connection) const
{
	return String(connection->remote_endpoint().address().to_string()) + ":" + String(connection->remote_endpoint().port());
}
