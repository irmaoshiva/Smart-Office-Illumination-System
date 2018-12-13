#include <unistd.h>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

using boost::asio::ip::tcp;

class client
{
public:
    client(boost::asio::io_service& io_service)
            : socket_(io_service),
              input_(io_service, ::dup(STDIN_FILENO)),
              console_buffer_(100000)
    {
    }
    void start(tcp::resolver::iterator endpoint_iter) {
        start_connect(endpoint_iter);
        start_read_console();
    }

private:
    void start_connect(tcp::resolver::iterator endpoint_iter) {
            socket_.async_connect(endpoint_iter->endpoint(),
                                  boost::bind(&client::handle_connect,
                                              this, _1, endpoint_iter));
    }

    void handle_connect(const boost::system::error_code& ec,
                        tcp::resolver::iterator endpoint_iter) {
            start_read();
    }

    void start_read() {
        boost::asio::async_read_until(socket_, input_buffer_, '\n',
                                      boost::bind(&client::handle_read, this, _1, _2));
    }

    void handle_read(const boost::system::error_code& ec, std::size_t length) {
        if (!ec) {
            std::string line;
            std::istream is(&input_buffer_);
            std::getline(is, line);
            if (!line.empty()) {
                std::cout << "Received: " << line << "\n";
            }
            start_read();
        }
        else {
            std::cout << "Error on receive: " << ec.message() << "\n";
        }
    }

    void start_read_console() {
        boost::asio::async_read_until(input_, console_buffer_, '\n',
                                      boost::bind(&client::handle_read_console, this, _1, _2));
    }
    void handle_read_console(const boost::system::error_code& ec, std::size_t length) {
        if (!ec) {
            std::string line, terminated_line;
            std::istream is(&console_buffer_);
            std::getline(is, line);
            if (!line.empty()) { // Empty messages are ignored.
                std::cout << "Sending: " << line << "\n";
                terminated_line = line + std::string("\n");
                std::size_t n = terminated_line.size();
                terminated_line.copy(send_buffer_, n);
                boost::asio::async_write(socket_, boost::asio::buffer(send_buffer_,n),
                                         boost::bind(&client::handle_send, this, _1, _2));
            }
            start_read_console();
        }
        else {
            std::cout << "Error on handle_read_console: " << ec.message() << "\n";
        }
    }
    void handle_send(const boost::system::error_code& ec, std::size_t length)
    {
        std::cout << "Sent " << length << " bytes" << std::endl;
    }
private:
    tcp::socket socket_;
    boost::asio::streambuf input_buffer_;
    boost::asio::posix::stream_descriptor input_;
    boost::asio::streambuf console_buffer_;
    enum {max_length=1024};
    char send_buffer_[max_length];
};

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: client <host> <port>\n";
            return 1;
        }
        boost::asio::io_service io_service;
        tcp::resolver r(io_service);
        client c(io_service);
        c.start(r.resolve(tcp::resolver::query(argv[1], argv[2], boost::asio::ip::resolver_query_base::numeric_service)));
        io_service.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}