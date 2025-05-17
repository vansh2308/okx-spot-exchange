#pragma once 

#include <memory>
#include <string>
#include <functional>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>

#include "core/config.h"


namespace websocket {

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

class WebSocketClient {
    // Q_OBJECT

public:
    explicit WebSocketClient(std::shared_ptr<core::Config> config);
    ~WebSocketClient();

    bool connect();
    void disconnect();
    bool isConnected() const;
    bool send(const std::string& message);
    void setMessageHandler(std::function<void(const std::string&)> handler);

// signals:
//     void connectionStateChanged(bool connected);

private:
    std::shared_ptr<core::Config> config_;
    std::unique_ptr<net::io_context> ioc_;
    std::unique_ptr<websocket::stream<beast::ssl_stream<tcp::socket>>> ws_;
    std::unique_ptr<ssl::context> ctx_;
    std::function<void(const std::string&)> messageHandler_;
    bool connected_;
    std::string host_;
    std::string port_;
    std::string path_;
};

}

