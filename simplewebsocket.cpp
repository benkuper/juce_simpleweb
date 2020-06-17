/*
  ==============================================================================

    simplewebsocket.cpp
    Created: 17 Jun 2020 8:43:10pm
    Author:  bkupe

  ==============================================================================
*/

#include "simplewebsocket.h"
#include "websocket/server_ws.hpp"
#include <future>

SimpleWebSocket::SimpleWebSocket()
{
}

SimpleWebSocket::~SimpleWebSocket()
{
    server.stop();
}

void SimpleWebSocket::start(int port)
{
    //This should should be called from another thread than the UI !!

    auto& endpoint = server.endpoint["^/ws/?$"];

    // Can modify handshake response headers here if needed
    endpoint.on_handshake = [](std::shared_ptr<WsServer::Connection> /*connection*/, SimpleWeb::CaseInsensitiveMultimap& /*response_header*/) {
        return SimpleWeb::StatusCode::information_switching_protocols; // Upgrade to websocket
    };

    endpoint.on_message = std::bind(&SimpleWebSocket::onMessage, this, std::placeholders::_1, std::placeholders::_2);
    endpoint.on_error = std::bind(&SimpleWebSocket::onError, this, std::placeholders::_1, std::placeholders::_2);
    endpoint.on_open = std::bind(&SimpleWebSocket::onNewConnection, this, std::placeholders::_1);
    endpoint.on_close = std::bind(&SimpleWebSocket::onConnectionClose, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

    server.config.port = (short)port;

    std::promise<unsigned short> server_port;
    this->server.start([&server_port](unsigned short port) {
        server_port.set_value(port);
    });
}

void SimpleWebSocket::stop()
{
    server.stop();
}

void SimpleWebSocket::closeConnection(const std::string& id)
{
    connectionMap[id]->send_close(0, "Goodbye");
}

void SimpleWebSocket::send(const std::string& message, const std::string& connectionId)
{
    if (connectionId.size() > 0) connectionMap.at(connectionId)->send(message);
    else for (auto& c : connectionMap) c.second->send(message);
}

void SimpleWebSocket::onMessage(std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message)
{
    onMessageFunc(getConnectionString(connection), in_message->string());
}

void SimpleWebSocket::onNewConnection(std::shared_ptr<WsServer::Connection> connection)
{
    std::string cString = getConnectionString(connection);
    connectionMap.emplace(cString, connection);
    onNewConnectionFunc(cString);
}

void SimpleWebSocket::onConnectionClose(std::shared_ptr<WsServer::Connection> connection, int status, const std::string& reason)
{
    std::string cString = getConnectionString(connection);
    connectionMap.erase(cString);
    onConnectionCloseFunc(cString, status, reason);
}

void SimpleWebSocket::onError(std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code& ec)
{
    onErrorFunc(getConnectionString(connection), ec.message());
}

std::string SimpleWebSocket::getConnectionString(std::shared_ptr<WsServer::Connection> connection) const
{
    return connection->remote_endpoint().address().to_string() + ":" + std::to_string(connection->remote_endpoint().port());
}
