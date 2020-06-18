#pragma once

#if USE_WEBSERVER
//#include "webserver/server_http.hpp"

//using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

class SimpleWebServer :
    public Thread
{
public:
    class RequestHandler
    {
    public:
        virtual ~RequestHandler() {}
        virtual String handleWebRequest(String request) = 0;
        virtual String handleError(String errorCode) {};
    };

    SimpleWebServer(RequestHandler * handler);  
    ~SimpleWebServer();

    void start(int port=8080);
    void stop();
    void run();

    File rootPath;

protected:
   // HttpServer server;
    RequestHandler* handler;

  //  void handleDefaultResource(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request);
    //void handleOnError(std::shared_ptr<HttpServer::Request> /*request*/, const SimpleWeb::error_code& /*ec*/);
};

#endif