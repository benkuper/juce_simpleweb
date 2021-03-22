
/*
  ==============================================================================

	juce_SimpleWebSocketServer.cpp
	Created: 17 Jun 2020 11:22:54pm
	Author:  bkupe

  ==============================================================================
*/

#include "MIMETypes.h"
#include "SimpleWebSocketServer.h"

SimpleWebSocketServerBase::SimpleWebSocketServerBase() :
	Thread("Web socket"),
	port(0),
	handler(nullptr)
{

}

SimpleWebSocketServerBase::~SimpleWebSocketServerBase()
{
	stopThread(2000);
}

void SimpleWebSocketServerBase::start(int _port, const String& _wsSuffix)
{
	port = _port;
	wsSuffix = _wsSuffix;
	startThread();
}



void SimpleWebSocketServerBase::send(const MemoryBlock& data)
{
	send((const char*)data.getData(), (int)data.getSize());
}

void SimpleWebSocketServerBase::stop()
{
	stopInternal();

#if JUCE_DEBUG //don't know why the order is not the same for debug and release...
	if (Thread::getCurrentThreadId() != this->getThreadId()) stopThread(100);
#endif

}

void SimpleWebSocketServerBase::closeConnection(const String& id, int code, const String& reason)
{
	closeConnectionInternal(id, code, reason);
}

void SimpleWebSocketServerBase::run()
{
	//HTTP init
	isConnected = false;

	initServer();
}



//SIMPLE


SimpleWebSocketServer::SimpleWebSocketServer()
{
}

SimpleWebSocketServer::~SimpleWebSocketServer()
{
	if (ioService != nullptr) ioService->stop();
	if (isConnected) ws.stop();
}

void SimpleWebSocketServer::send(const String& message)
{
	HashMap<String, std::shared_ptr<WsServer::Connection>>::Iterator it(connectionMap);
	while (it.next()) it.getValue()->send(message.toStdString());
}

void SimpleWebSocketServer::send(const char* data, int numData)
{
	std::shared_ptr<WsServer::OutMessage> out_message = std::make_shared<WsServer::OutMessage>();
	out_message->write(data, numData);
	HashMap<String, std::shared_ptr<WsServer::Connection>>::Iterator it(connectionMap);
	while (it.next())
	{
		it.getValue()->send(out_message, nullptr, 130); //130 = binary
	}
}

void SimpleWebSocketServer::sendTo(const String& message, const String& id)
{
	if (connectionMap.contains(id)) connectionMap[id]->send(message.toStdString());
	else
	{
		DBG("[Dashboard] Websocket connection not found : " << id);
	}
}

void SimpleWebSocketServer::sendTo(const MemoryBlock& data, const String& id)
{
	std::shared_ptr<WsServer::OutMessage> out_message = std::make_shared<WsServer::OutMessage>();
	out_message->write((const char*)data.getData(), data.getSize());
	if (connectionMap.contains(id)) connectionMap[id]->send(out_message, nullptr, 130); //130 = binary
	else
	{
		DBG("[Dashboard] Websocket connection not found : " << id);
	}
}

void SimpleWebSocketServer::sendExclude(const String& message, const StringArray excludeIds)
{
	HashMap<String, std::shared_ptr<WsServer::Connection>>::Iterator it(connectionMap);
	while (it.next())
	{
		if (excludeIds.contains(it.getKey())) continue;
		it.getValue()->send(message.toStdString());
	}
}

void SimpleWebSocketServer::sendExclude(const MemoryBlock& data, const StringArray excludeIds)
{
	std::shared_ptr<WsServer::OutMessage> out_message = std::make_shared<WsServer::OutMessage>();
	out_message->write((const char*)data.getData(), data.getSize());

	HashMap<String, std::shared_ptr<WsServer::Connection>>::Iterator it(connectionMap);
	while (it.next())
	{
		if (excludeIds.contains(it.getKey())) continue;
		it.getValue()->send(out_message, nullptr, 130); //130 = binary
	}
}

void SimpleWebSocketServer::stopInternal()
{
	std::unordered_set<std::shared_ptr<WsServer::Connection>> connections = ws.get_connections();
	for (auto& c : connections) c->send_close(1000, "Server destroyed");
	connectionMap.clear();

#if !JUCE_DEBUG
	if (Thread::getCurrentThreadId() != this->getThreadId()) stopThread(100);
#endif

	ws.stop();
	if (ioService != nullptr) ioService->stop();
	http.stop();
	ioService.reset();
}

void SimpleWebSocketServer::closeConnectionInternal(const String& id, int code, const String& reason)
{
	if (!connectionMap.contains(id)) return;
	connectionMap[id]->send_close(code, reason.toStdString());
}

void SimpleWebSocketServer::initServer()
{
	ioService = std::make_shared<asio::io_service>();
	http.config.port = port;
	http.io_service = ioService;

	http.default_resource["GET"] = std::bind(&SimpleWebSocketServer::httpDefaultCallback, this, std::placeholders::_1, std::placeholders::_2);
	http.on_upgrade = std::bind(&SimpleWebSocketServer::onHTTPUpgrade, this, std::placeholders::_1, std::placeholders::_2);

	//WebSocket init
	auto& wsEndpoint = ws.endpoint[("^" + wsSuffix + "/?$").toStdString()];

	wsEndpoint.on_message = std::bind(&SimpleWebSocketServer::onMessageCallback, this, std::placeholders::_1, std::placeholders::_2);
	wsEndpoint.on_error = std::bind(&SimpleWebSocketServer::onErrorCallback, this, std::placeholders::_1, std::placeholders::_2);
	wsEndpoint.on_open = std::bind(&SimpleWebSocketServer::onNewConnectionCallback, this, std::placeholders::_1);
	wsEndpoint.on_close = std::bind(&SimpleWebSocketServer::onConnectionCloseCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	http.config.timeout_request = 1;
	http.config.timeout_content = 2;
	http.config.max_request_streambuf_size = 1000000;
	http.config.thread_pool_size = 2;
	http.start(std::bind(&SimpleWebSocketServer::httpStartCallback, this, std::placeholders::_1));
	if (ioService != nullptr) ioService->run();
}

int SimpleWebSocketServer::getNumActiveConnections() const
{
	return connectionMap.size();
}


String SimpleWebSocketServer::getConnectionString(std::shared_ptr<WsServer::Connection> connection) const
{
	return String(connection->remote_endpoint().address().to_string()) + ":" + String(connection->remote_endpoint().port());
}



void SimpleWebSocketServer::onMessageCallback(std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message)
{
	String id = getConnectionString(connection);
	if (in_message->fin_rsv_opcode == 129) webSocketListeners.call(&Listener::messageReceived, id, String(in_message->string()));
	else if (in_message->fin_rsv_opcode == 130) webSocketListeners.call(&Listener::dataReceived, id, in_message->binary);
	else if (in_message->fin_rsv_opcode == 136)
	{
		DBG("Connection ended for " << id);
	}
}

void SimpleWebSocketServer::onNewConnectionCallback(std::shared_ptr<WsServer::Connection> connection)
{
	String id = getConnectionString(connection);
	connectionMap.set(id, connection);
	webSocketListeners.call(&Listener::connectionOpened, id);
}

void SimpleWebSocketServer::onConnectionCloseCallback(std::shared_ptr<WsServer::Connection> connection, int status, const std::string& reason)
{
	String id = getConnectionString(connection);
	connectionMap.remove(id);
	webSocketListeners.call(&Listener::connectionClosed, id, status, reason);
}

void SimpleWebSocketServer::onErrorCallback(std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code& ec)
{
	String id = getConnectionString(connection);
	connectionMap.remove(id);
	webSocketListeners.call(&Listener::connectionError, id, ec.message());
}

void SimpleWebSocketServer::httpStartCallback(unsigned short _port)
{
	isConnected = port == _port;
}

void SimpleWebSocketServer::onHTTPUpgrade(std::unique_ptr<SimpleWeb::HTTP>& socket, std::shared_ptr<HttpServer::Request> request)
{
	auto connection = std::make_shared<WsServer::Connection>(std::move(socket));
	connection->method = std::move(request->method);
	connection->path = std::move(request->path);
	connection->http_version = std::move(request->http_version);
	connection->header = std::move(request->header);
	ws.upgrade(connection);
}

void SimpleWebSocketServer::httpDefaultCallback(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
{
	if (handler != nullptr)
	{

		bool handled = handler->handleHTTPRequest(response, request);
		if (handled) return;
	}

	if (rootPath.exists() && rootPath.isDirectory())
	{
		//String content = "";
		String contentType = "text/html";
		File f;

		String path = request->path.substr(1);
		if (path.isEmpty()) path = "index.html";
		f = rootPath.getChildFile(path); //substr to remove the first "/"

		if (f.exists() && f.isDirectory()) f = f.getChildFile("index.html");

		if (f.existsAsFile())
		{
			contentType = MIMETypes::getMIMEType(f.getFileExtension());

			bool fileIsText = contentType.contains("text") || contentType.contains("json") || contentType.contains("script") || contentType.contains("css");

			SimpleWeb::CaseInsensitiveMultimap header;
			header.emplace("Content-Length", String(f.getSize()).toStdString());
			header.emplace("Content-Type", contentType.toStdString());
			header.emplace("Accept-range", "bytes");
			header.emplace("Access-Control-Allow-Origin", "*");

			response->write(SimpleWeb::StatusCode::success_ok, header);

			if (fileIsText)
			{
				*response << f.loadFileAsString().toStdString();
			}
			else
			{
				MemoryBlock b;
				std::unique_ptr<FileInputStream> fs = f.createInputStream();
				fs->readIntoMemoryBlock(b);
				response->write((const char*)b.getData(), b.getSize());
			}

			return;
		}
		else
		{
			DBG("WebServer requested file not found : " << f.getFullPathName());
		}
	}

	*response << "HTTP/1.1 404 Not Found";
}




//SECURE

SecureWebSocketServer::SecureWebSocketServer(const String& certFile, const String& privateKeyFile, const String& verifyFile) :
	ws(certFile.toStdString(), privateKeyFile.toStdString(), verifyFile.toStdString()),
	http(certFile.toStdString(), privateKeyFile.toStdString(), verifyFile.toStdString())
{
}

SecureWebSocketServer::~SecureWebSocketServer()
{
	if (ioService != nullptr) ioService->stop();
	if (isConnected) ws.stop();
	stopThread(1000);
}

void SecureWebSocketServer::send(const String& message)
{
	HashMap<String, std::shared_ptr<WssServer::Connection>>::Iterator it(connectionMap);
	while (it.next()) it.getValue()->send(message.toStdString());
}

void SecureWebSocketServer::send(const char* data, int numData)
{
	std::shared_ptr<WssServer::OutMessage> out_message = std::make_shared<WssServer::OutMessage>();
	out_message->write(data, numData);
	HashMap<String, std::shared_ptr<WssServer::Connection>>::Iterator it(connectionMap);
	while (it.next())
	{
		it.getValue()->send(out_message, nullptr, 130); //130 = binary
	}
}

void SecureWebSocketServer::sendTo(const String& message, const String& id)
{
	if (connectionMap.contains(id)) connectionMap[id]->send(message.toStdString());
	else
	{
		DBG("[Dashboard] Websocket connection not found : " << id);
	}
}

void SecureWebSocketServer::sendTo(const MemoryBlock& data, const String& id)
{
	std::shared_ptr<WssServer::OutMessage> out_message = std::make_shared<WssServer::OutMessage>();
	out_message->write((const char*)data.getData(), data.getSize());
	if (connectionMap.contains(id)) connectionMap[id]->send(out_message, nullptr, 130); //130 = binary
	else
	{
		DBG("[Dashboard] Websocket connection not found : " << id);
	}
}

void SecureWebSocketServer::sendExclude(const String& message, const StringArray excludeIds)
{
	HashMap<String, std::shared_ptr<WssServer::Connection>>::Iterator it(connectionMap);
	while (it.next())
	{
		if (excludeIds.contains(it.getKey())) continue;
		it.getValue()->send(message.toStdString());
	}
}

void SecureWebSocketServer::sendExclude(const MemoryBlock& data, const StringArray excludeIds)
{
	std::shared_ptr<WssServer::OutMessage> out_message = std::make_shared<WssServer::OutMessage>();
	out_message->write((const char*)data.getData(), data.getSize());

	HashMap<String, std::shared_ptr<WssServer::Connection>>::Iterator it(connectionMap);
	while (it.next())
	{
		if (excludeIds.contains(it.getKey())) continue;
		it.getValue()->send(out_message, nullptr, 130); //130 = binary
	}
}

void SecureWebSocketServer::stopInternal()
{
	std::unordered_set<std::shared_ptr<WssServer::Connection>> connections = ws.get_connections();
	for (auto& c : connections) c->send_close(1000, "Server destroyed");
	connectionMap.clear();

#if !JUCE_DEBUG
	if (Thread::getCurrentThreadId() != this->getThreadId()) stopThread(100);
#endif

	ws.stop();
	if (ioService != nullptr) ioService->stop();
	http.stop();
	ioService.reset();
}

void SecureWebSocketServer::closeConnectionInternal(const String& id, int code, const String& reason)
{
	if (!connectionMap.contains(id)) return;
	connectionMap[id]->send_close(code, reason.toStdString());
}

void SecureWebSocketServer::initServer()
{
	ioService = std::make_shared<asio::io_service>();
	http.config.port = port;
	http.io_service = ioService;

	http.default_resource["GET"] = std::bind(&SecureWebSocketServer::httpDefaultCallback, this, std::placeholders::_1, std::placeholders::_2);
	http.on_upgrade = std::bind(&SecureWebSocketServer::onHTTPUpgrade, this, std::placeholders::_1, std::placeholders::_2);

	//WebSocket init
	auto& wsEndpoint = ws.endpoint[("^" + wsSuffix + "/?$").toStdString()];

	wsEndpoint.on_message = std::bind(&SecureWebSocketServer::onMessageCallback, this, std::placeholders::_1, std::placeholders::_2);
	wsEndpoint.on_error = std::bind(&SecureWebSocketServer::onErrorCallback, this, std::placeholders::_1, std::placeholders::_2);
	wsEndpoint.on_open = std::bind(&SecureWebSocketServer::onNewConnectionCallback, this, std::placeholders::_1);
	wsEndpoint.on_close = std::bind(&SecureWebSocketServer::onConnectionCloseCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	http.config.timeout_request = 1;
	http.config.timeout_content = 2;
	http.config.max_request_streambuf_size = 1000000;
	http.config.thread_pool_size = 2;
	http.start(std::bind(&SecureWebSocketServer::httpStartCallback, this, std::placeholders::_1));
	if (ioService != nullptr) ioService->run();
}

int SecureWebSocketServer::getNumActiveConnections() const
{
	return connectionMap.size();
}


String SecureWebSocketServer::getConnectionString(std::shared_ptr<WssServer::Connection> connection) const
{
	return String(connection->remote_endpoint().address().to_string()) + ":" + String(connection->remote_endpoint().port());
}


void SecureWebSocketServer::onMessageCallback(std::shared_ptr<WssServer::Connection> connection, std::shared_ptr<WssServer::InMessage> in_message)
{
	String id = getConnectionString(connection);
	if (in_message->fin_rsv_opcode == 129) webSocketListeners.call(&Listener::messageReceived, id, String(in_message->string()));
	else if (in_message->fin_rsv_opcode == 130) webSocketListeners.call(&Listener::dataReceived, id, in_message->binary);
	else if (in_message->fin_rsv_opcode == 136)
	{
		DBG("Connection ended for " << id);
	}
}

void SecureWebSocketServer::onNewConnectionCallback(std::shared_ptr<WssServer::Connection> connection)
{
	String id = getConnectionString(connection);
	connectionMap.set(id, connection);
	webSocketListeners.call(&Listener::connectionOpened, id);
}

void SecureWebSocketServer::onConnectionCloseCallback(std::shared_ptr<WssServer::Connection> connection, int status, const std::string& reason)
{
	String id = getConnectionString(connection);
	connectionMap.remove(id);
	webSocketListeners.call(&Listener::connectionClosed, id, status, reason);
}

void SecureWebSocketServer::onErrorCallback(std::shared_ptr<WssServer::Connection> connection, const SimpleWeb::error_code& ec)
{
	String id = getConnectionString(connection);
	connectionMap.remove(id);
	webSocketListeners.call(&Listener::connectionError, id, ec.message());
}

void SecureWebSocketServer::httpStartCallback(unsigned short _port)
{
	isConnected = port == _port;
}

void SecureWebSocketServer::onHTTPUpgrade(std::unique_ptr<SimpleWeb::HTTPS>& socket, std::shared_ptr<HttpsServer::Request> request)
{
	auto connection = std::make_shared<WssServer::Connection>(std::move(socket));
	connection->method = std::move(request->method);
	connection->path = std::move(request->path);
	connection->http_version = std::move(request->http_version);
	connection->header = std::move(request->header);
	ws.upgrade(connection);
}

void SecureWebSocketServer::httpDefaultCallback(std::shared_ptr<HttpsServer::Response> response, std::shared_ptr<HttpsServer::Request> request)
{
	if (handler != nullptr)
	{
		bool handled = handler->handleHTTPSRequest(response, request);
		if (handled) return;
	}

	if (rootPath.exists() && rootPath.isDirectory())
	{
		//String content = "";
		String contentType = "text/html";
		File f;

		String path = request->path.substr(1);
		if (path.isEmpty()) path = "index.html";
		f = rootPath.getChildFile(path); //substr to remove the first "/"

		if (f.exists() && f.isDirectory()) f = f.getChildFile("index.html");

		if (f.existsAsFile())
		{
			contentType = MIMETypes::getMIMEType(f.getFileExtension());

			bool fileIsText = contentType.contains("text") || contentType.contains("json") || contentType.contains("script") || contentType.contains("css");

			SimpleWeb::CaseInsensitiveMultimap header;
			header.emplace("Content-Length", String(f.getSize()).toStdString());
			header.emplace("Content-Type", contentType.toStdString());
			header.emplace("Accept-range", "bytes");
			header.emplace("Access-Control-Allow-Origin", "*");

			response->write(SimpleWeb::StatusCode::success_ok, header);

			if (fileIsText)
			{
				*response << f.loadFileAsString().toStdString();
			}
			else
			{
				MemoryBlock b;
				std::unique_ptr<FileInputStream> fs = f.createInputStream();
				fs->readIntoMemoryBlock(b);
				response->write((const char*)b.getData(), b.getSize());
			}

			return;
		}
		else
		{
			DBG("WebServer requested file not found : " << f.getFullPathName());
		}
	}

	*response << "HTTP/1.1 404 Not Found";
}
