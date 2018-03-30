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
    dispatcher_.want_write(socket_, [this, buffer, handler = std::move(handler)](auto&& error){ do_write(error, buffer, handler); });
}

void tcp::session::read(mutable_buffer const& buffer, completion_handler&& handler)
{
    dispatcher_.want_read(socket_, [this, buffer, handler = std::move(handler)](auto&& error){ do_read(error, buffer, handler); });
}

void tcp::session::close()
{
    dispatcher_.cancel_writes(socket_);
    dispatcher_.cancel_reads(socket_);

    socket_.close();
}

void tcp::session::do_write(std::error_code error, const_buffer buffer, completion_handler const& handler)
{
    std::size_t sent = 0;

    if(!error)
    {
        try
        {
            sent = socket_.write(buffer);
        }
        catch(std::system_error const& e)
        {
            error = e.code();
        }
    }

    handler(error, sent);
}

void tcp::session::do_read(std::error_code error, mutable_buffer buffer, completion_handler const& handler)
{
    std::size_t read = 0;

    if(!error)
    {
        try
        {
            read = socket_.read(buffer);

            if(read == 0)
            {
                close();
            }
        }
        catch(std::system_error const& e)
        {
            error = e.code();
        }
    }

    handler(error, read);
}
