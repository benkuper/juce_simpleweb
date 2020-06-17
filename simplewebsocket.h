/*
  ==============================================================================

    simplewebsocket.h
    Created: 17 Jun 2020 8:43:10pm
    Author:  bkupe

  ==============================================================================
*/

#pragma once

#include <memory>
#include <functional>
#include <string>

#include "websocket/server_ws.hpp"
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

class SimpleWebSocket
{
public:
    SimpleWebSocket();
    ~SimpleWebSocket();


    std::function<void(const std::string& connectionId, const std::string msg)> onMessageFunc;
    std::function<void(const std::string& connectionId)> onNewConnectionFunc;
    std::function<void(const std::string& connectionId, int status, const std::string& reason)> onConnectionCloseFunc;
    std::function<void(const std::string& connectionId, const std::string& message)> onErrorFunc;

    void start(int port = 8080);
    void stop();

    void closeConnection(const std::string& id);

    void send(const std::string& message, const std::string& connectionId = "");

protected:
    WsServer server;
    std::map<std::string, std::shared_ptr<WsServer::Connection>> connectionMap;

    void onMessage(std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message);
    void onNewConnection(std::shared_ptr<WsServer::Connection> connection);
    void onConnectionClose(std::shared_ptr<WsServer::Connection> connection, int status, const std::string& /*reason*/);
    void onError(std::shared_ptr<WsServer::Connection> connection, const SimpleWeb::error_code& ec);

    std::string getConnectionString(std::shared_ptr<WsServer::Connection> connection) const;
    
};