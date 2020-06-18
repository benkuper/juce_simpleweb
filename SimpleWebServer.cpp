#if USE_WEBSERVER

SimpleWebServer::SimpleWebServer(RequestHandler* handler) :
    Thread("SimpleWebServer"),
    handler(handler)
{
}

SimpleWebServer::~SimpleWebServer()
{
}

void SimpleWebServer::start(int port)
{
    /*
    server.config.port = 8080;

    server.default_resource["GET"] = std::bind(&SimpleWebServer::handleDefaultResource, this, std::placeholders::_1, std::placeholders::_2);
    server.start();
    */
}

void SimpleWebServer::stop()
{
    //server.stop();
}

void SimpleWebServer::run()
{
}

/*
void SimpleWebServer::handleDefaultResource(std::shared_ptr<HttpServer::Response> response, std::shared_ptr<HttpServer::Request> request)
{
    String content = "Sorry not sorry";
   
   if (handler != nullptr) content = handler->handleWebRequest(request->content.string());
    else if(rootPath.exists() && rootPath.isDirectory())
    {
        File f = rootPath.getChildFile(request->path);
        if (f.isDirectory()) f = f.getChildFile("index.html");
        if(f.existsAsFile()) content = f.loadFileAsString();
    }
   
    SimpleWeb::CaseInsensitiveMultimap header;
    
    header.emplace("Content-Length", content.length()); 
    response->write(content.toStdString(), header);
   
}

void SimpleWebServer::handleOnError(std::shared_ptr<HttpServer::Request>, const SimpleWeb::error_code& ec)
{
    DBG("Error : " << ec.message());
    if (handler != nullptr) handler->handleError(ec.message());
}
*/

#endif