
#include "websocket/websocket_client.h"
#include "core/logger.h"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <iostream>

namespace websocket {

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

WebSocketClient::WebSocketClient(std::shared_ptr<core::Config> config)
    : config_(config), connected_(false)
{
    // Parse WebSocket URL from config
    std::string url = config_->getWebSocketEndpoint();
    size_t protocolEnd = url.find("://");
    if (protocolEnd == std::string::npos) {
        throw std::runtime_error("Invalid WebSocket URL format");
    }

    size_t hostStart = protocolEnd + 3;
    size_t pathStart = url.find('/', hostStart);
    if (pathStart == std::string::npos) {
        pathStart = url.length();
    }

    host_ = url.substr(hostStart, pathStart - hostStart);
    path_ = url.substr(pathStart);
    port_ = "443"; // Default to secure WebSocket port

}

WebSocketClient::~WebSocketClient() {
    disconnect();
}

bool WebSocketClient::connect() {
    try {
        // Create I/O context
        ioc_ = std::make_unique<net::io_context>();
        ctx_ = std::make_unique<ssl::context>(ssl::context::sslv23_client);

        tcp::resolver resolver{*ioc_};
        auto const results = resolver.resolve(host_, port_);

        ws_ = std::make_unique<websocket::stream<beast::ssl_stream<tcp::socket>>>(beast::ssl_stream<tcp::socket>{*ioc_, *ctx_});

        net::connect(ws_->next_layer().next_layer(), results.begin(), results.end());

        ws_->next_layer().handshake(ssl::stream_base::client);

        if (!SSL_set_tlsext_host_name(ws_->next_layer().native_handle(), host_.c_str())) {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            throw beast::system_error{ec};
        }

        // Perform WebSocket handshake
        ws_->handshake(host_, path_);
        std::cout << "[Connected] WebSocket handshake completed.\n";

        for (;;) {
            beast::flat_buffer buffer;
            ws_->read(buffer);

            std::cout << "[Received] " << beast::make_printable(buffer.data()) << "\n";
        }

        return true;

    } catch (const std::exception& e) {
        core::Logger::getInstance().error("WebSocket connection failed: {}", e.what());
        connected_ = false;
        // emit connectionStateChanged(false);
        return false;
    }
}

void WebSocketClient::disconnect() {
    if (ws_ && connected_) {
        try {
            ws_->close(websocket::close_code::normal);
        } catch (...) {
        }
        connected_ = false;
        // emit connectionStateChanged(false);
    }
}

bool WebSocketClient::isConnected() const {
    return connected_;
}

bool WebSocketClient::send(const std::string& message) {
    if (!ws_ || !connected_) {
        return false;
    }
    try {
        ws_->write(net::buffer(message));
        return true;
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Failed to send message: {}", e.what());
        return false;
    }
}

void WebSocketClient::setMessageHandler(std::function<void(const std::string&)> handler) {
    messageHandler_ = handler;
    // For a real async client, you would start an async_read loop here.
    // For now, you can implement a synchronous receive method if needed.
}

} // namespace websocket 