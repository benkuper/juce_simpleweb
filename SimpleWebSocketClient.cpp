
/*
  ==============================================================================

	juce_SimpleWebSocket.cpp
	Created: 17 Jun 2020 11:22:54pm
	Author:  bkupe

  ==============================================================================
*/

#include "MIMETypes.h"
#include "SimpleWebSocketClient.h"

SimpleWebSocketClient::SimpleWebSocketClient() :
	Thread("Web socket client"),
	isConnected(false),
	ws(nullptr)
{

}

SimpleWebSocketClient::~SimpleWebSocketClient()
{
	stop();
}

void SimpleWebSocketClient::start(const String& _serverPath)
{
	serverPath = _serverPath;
	startThread();
}

void SimpleWebSocketClient::send(const String& message)
{
	if (connection != nullptr) connection->send(message.toStdString());
}

void SimpleWebSocketClient::send(const MemoryBlock& data)
{
	send((const char*)data.getData(), (int)data.getSize());
}

void SimpleWebSocketClient::send(const char* data, int numData)
{
	std::shared_ptr<WsClient::OutMessage> out_message = std::make_shared<WsClient::OutMessage>();
	out_message->write(data, numData);
	if (connection != nullptr) connection->send(out_message, nullptr, 130);
}


void SimpleWebSocketClient::stop()
{
	if (connection != nullptr) connection->send_close(1000, "Time to split my friend");
	if(ws != nullptr) ws->stop();
	if (Thread::getCurrentThreadId() != this->getThreadId()) stopThread(1000);
}


void SimpleWebSocketClient::run()
{
	isConnected = false; 
	ws.reset(new WsClient(serverPath.toStdString()));

	ws->on_message = std::bind(&SimpleWebSocketClient::onMessageCallback, this, std::placeholders::_1, std::placeholders::_2);
	ws->on_error = std::bind(&SimpleWebSocketClient::onErrorCallback, this, std::placeholders::_1, std::placeholders::_2);
	ws->on_open = std::bind(&SimpleWebSocketClient::onNewConnectionCallback, this, std::placeholders::_1);
	ws->on_close = std::bind(&SimpleWebSocketClient::onConnectionCloseCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	ws->start();

	//end thread
	isConnected = false;
}


void SimpleWebSocketClient::onMessageCallback(std::shared_ptr<WsClient::Connection> connection, std::shared_ptr<WsClient::InMessage> in_message)
{
	webSocketListeners.call(&Listener::messageReceived, String(in_message->string()));
}

void SimpleWebSocketClient::onNewConnectionCallback(std::shared_ptr<WsClient::Connection> _connection)
{
	isConnected = true;
	connection = _connection;
	webSocketListeners.call(&Listener::connectionOpened);
}

void SimpleWebSocketClient::onConnectionCloseCallback(std::shared_ptr<WsClient::Connection> /*_connection*/, int status, const std::string& reason)
{
	webSocketListeners.call(&Listener::connectionClosed, status, reason);
}

void SimpleWebSocketClient::onErrorCallback(std::shared_ptr<WsClient::Connection> /*_connection*/, const SimpleWeb::error_code& ec)
{
	isConnected = false;
	webSocketListeners.call(&Listener::connectionError, ec.message());
}
