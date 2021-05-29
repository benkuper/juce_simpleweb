SimpleWebSocketClientBase::SimpleWebSocketClientBase() :
	Thread("Web socket client"),
	isConnected(false),
	isClosing(false)
{
}

SimpleWebSocketClientBase::~SimpleWebSocketClientBase()
{
	stop();
}

void SimpleWebSocketClientBase::start(const String& _serverPath)
{
	this->serverPath = _serverPath;
	startThread();
}

void SimpleWebSocketClientBase::send(const MemoryBlock& data)
{
	send((const char*)data.getData(), (int)data.getSize());
}

void SimpleWebSocketClientBase::stop()
{
	this->isClosing = true;
	
	stopInternal();

	if (Thread::getCurrentThreadId() != this->getThreadId()) stopThread(1000);
	this->isClosing = false;
}

void SimpleWebSocketClientBase::run()
{
	this->isConnected = false;
	String s = this->serverPath;

	initWS();

	//end thread
	this->isConnected = false;
}

void SimpleWebSocketClientBase::handleNewConnectionCallback()
{
	this->isConnected = true;
	this->webSocketListeners.call(&Listener::connectionOpened);
}

void SimpleWebSocketClientBase::handleConnectionClosedCallback(int status, const String& reason)
{
	this->webSocketListeners.call(&Listener::connectionClosed, status, reason);
}

void SimpleWebSocketClientBase::handleErrorCallback(const String& message)
{
	this->isConnected = false;
	if (!this->isClosing) this->webSocketListeners.call(&Listener::connectionError, message);
}


// SIMPLE WS


SimpleWebSocketClient::SimpleWebSocketClient() :
	ws(nullptr)
{

}

SimpleWebSocketClient::~SimpleWebSocketClient()
{
	stop();
}

void SimpleWebSocketClient::send(const String& message)
{
	if (this->connection != nullptr) this->connection->send(message.toStdString());
}

void SimpleWebSocketClient::send(const char* data, int numData)
{
	std::shared_ptr<WsClient::OutMessage> out_message = std::make_shared<WsClient::OutMessage>();
	out_message->write(data, numData);
	if (this->connection != nullptr) this->connection->send(out_message, nullptr, 130);
}

void SimpleWebSocketClient::stopInternal()
{
	if (this->connection != nullptr) this->connection->send_close(1000, "Time to split my friend");
	if (ws != nullptr) ws->stop();

}

void SimpleWebSocketClient::initWS()
{
	ws.reset(new WsClient(serverPath.toStdString()));

	ws->config.timeout_request = 1000;
	ws->config.timeout_idle = 1000;

	ws->on_message = std::bind(&SimpleWebSocketClient::onMessageCallback, this, std::placeholders::_1, std::placeholders::_2);
	ws->on_error = std::bind(&SimpleWebSocketClient::onErrorCallback, this, std::placeholders::_1, std::placeholders::_2);
	ws->on_open = std::bind(&SimpleWebSocketClient::onNewConnectionCallback, this, std::placeholders::_1);
	ws->on_close = std::bind(&SimpleWebSocketClient::onConnectionCloseCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	ws->start();
}

void SimpleWebSocketClient::onMessageCallback(std::shared_ptr<WsClient::Connection> connection, std::shared_ptr<WsClient::InMessage> in_message)
{
	if (in_message->fin_rsv_opcode == 129) webSocketListeners.call(&Listener::messageReceived, String(in_message->string()));
	else if (in_message->fin_rsv_opcode == 130)
	{
		MemoryBlock b(in_message->string().c_str(), in_message->size());
		webSocketListeners.call(&Listener::dataReceived, b);
	}
	else if (in_message->fin_rsv_opcode == 136)
	{
		DBG("Connection ended");
	}
}

void SimpleWebSocketClient::onNewConnectionCallback(std::shared_ptr<WsClient::Connection> _connection)
{
	this->connection = _connection;
	handleNewConnectionCallback();
}

void SimpleWebSocketClient::onConnectionCloseCallback(std::shared_ptr<WsClient::Connection>, int status, const std::string& reason)
{
	handleConnectionClosedCallback(status, reason);
}

void SimpleWebSocketClient::onErrorCallback(std::shared_ptr<WsClient::Connection>, const SimpleWeb::error_code& ec)
{
	handleErrorCallback(ec.message());
}


//SECURE

#if SIMPLEWEB_SECURE_SUPPORTED
SecureWebSocketClient::SecureWebSocketClient() :
	ws(nullptr)
{

}

SecureWebSocketClient::~SecureWebSocketClient()
{

}

void SecureWebSocketClient::send(const String& message)
{
	if (this->connection != nullptr) this->connection->send(message.toStdString());
}

void SecureWebSocketClient::send(const char* data, int numData)
{
	std::shared_ptr<WssClient::OutMessage> out_message = std::make_shared<WssClient::OutMessage>();
	out_message->write(data, numData);
	if (this->connection != nullptr) this->connection->send(out_message, nullptr, 130);
}

void SecureWebSocketClient::stopInternal()
{
	if (this->connection != nullptr) this->connection->send_close(1000, "Time to split my friend");
	if (ws != nullptr) ws->stop();

}

void SecureWebSocketClient::initWS()
{
	ws.reset(new WssClient(serverPath.toStdString(), false));

	ws->config.timeout_request = 1000;
	ws->config.timeout_idle = 1000;

	ws->on_message = std::bind(&SecureWebSocketClient::onMessageCallback, this, std::placeholders::_1, std::placeholders::_2);
	ws->on_error = std::bind(&SecureWebSocketClient::onErrorCallback, this, std::placeholders::_1, std::placeholders::_2);
	ws->on_open = std::bind(&SecureWebSocketClient::onNewConnectionCallback, this, std::placeholders::_1);
	ws->on_close = std::bind(&SecureWebSocketClient::onConnectionCloseCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

	ws->start();
}

void SecureWebSocketClient::onMessageCallback(std::shared_ptr<WssClient::Connection> connection, std::shared_ptr<WssClient::InMessage> in_message)
{
	if (in_message->fin_rsv_opcode == 129) webSocketListeners.call(&Listener::messageReceived, String(in_message->string()));
	else if (in_message->fin_rsv_opcode == 130) webSocketListeners.call(&Listener::dataReceived, in_message->binary);
	else if (in_message->fin_rsv_opcode == 136)
	{
		DBG("Connection ended");
	}
}

void SecureWebSocketClient::onNewConnectionCallback(std::shared_ptr<WssClient::Connection> _connection)
{
	this->connection = _connection;
	handleNewConnectionCallback();
}

void SecureWebSocketClient::onConnectionCloseCallback(std::shared_ptr<WssClient::Connection>, int status, const std::string& reason)
{
	handleConnectionClosedCallback(status, reason);
}

void SecureWebSocketClient::onErrorCallback(std::shared_ptr<WssClient::Connection>, const SimpleWeb::error_code& ec)
{
	handleErrorCallback(ec.message());
}
#endif
