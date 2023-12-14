#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <optional>

namespace duckpg {
class Session : public std::enable_shared_from_this<Session> {
  public:
    Session(boost::asio::ip::tcp::socket &&socket);
    ~Session();

    void start();

  private:
    boost::asio::ip::tcp::socket _socket;
    boost::asio::streambuf _streambuf;
};

class Server {
  public:
    Server(boost::asio::io_context &io_context,
           boost::asio::ip::tcp::endpoint endpoint);
    void start();

  private:
    void async_accept();

  private:
    boost::asio::io_context &_io_context;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::optional<boost::asio::ip::tcp::socket> _socket;
};
} // namespace duckpg
