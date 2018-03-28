#include "session.hpp"


tcp::session::session(socket&& socket, dispatcher& dispatcher) :
    socket_(std::move(socket)),
    dispatcher_(dispatcher)
{}

tcp::session::session(dispatcher& dispatcher) :
    dispatcher_(dispatcher)
{}

tcp::session::~session()
{
    close();
}

tcp::socket& tcp::session::get_socket()
{
    return socket_;
}

void tcp::session::write(const_buffer const& buffer, completion_handler&& handler)
{
    dispatcher_.want_write(socket_, [=, handler = std::move(handler)]{ do_write(buffer, handler); });
}

void tcp::session::read(mutable_buffer const& buffer, completion_handler&& handler)
{
    dispatcher_.want_read(socket_, [=, handler = std::move(handler)]{ do_read(buffer, handler); });
}

void tcp::session::close()
{
    dispatcher_.cancel_write(socket_);
    dispatcher_.cancel_read(socket_);

    socket_.close();
}

void tcp::session::do_write(const_buffer buffer, completion_handler const& handler)
{
    std::size_t sent = 0;

    std::error_code error;

    try
    {
        sent = socket_.write(buffer);
    }
    catch(std::system_error const& e)
    {
        error = e.code();
    }

    handler(error, sent);
}

void tcp::session::do_read(mutable_buffer buffer, completion_handler const& handler)
{
    std::size_t read = 0;

    std::error_code error;

    try
    {
        read = socket_.read(buffer);

        if(read == 0)
        {
            socket_.close();
        }
    }
    catch(std::system_error const& e)
    {
        error = e.code();
    }

    handler(error, read);
}
