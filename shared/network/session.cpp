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
    write_queue_.emplace(buffer, std::move(handler));
    dispatcher_.want_write(socket_, [this]{ do_write(); });
}

void tcp::session::read(mutable_buffer const& buffer, completion_handler&& handler)
{
    read_queue_.emplace(buffer, std::move(handler));
    dispatcher_.want_read(socket_, [this]{ do_read(); });
}

void tcp::session::close()
{
    socket_.close();
}

void tcp::session::do_write()
{
    auto [buffer, handler] = write_queue_.front();

    try
    {
        handler(std::error_code(), socket_.write(buffer));
    }
    catch(std::system_error const& e)
    {
        handler(e.code(), 0);
    }

    if(buffer.empty())
    {
        write_queue_.pop();
    }

    if(!write_queue_.empty())
    {
        dispatcher_.want_write(socket_, [this]{ do_write(); });
    }
}

void tcp::session::do_read()
{
    auto [buffer, handler] = read_queue_.front();

    read_queue_.pop();

    try
    {
        std::size_t read = socket_.read(buffer);

        if(read == 0)
        {
            read_queue().swap(read_queue_);
            socket_.close();
        }

        handler(std::error_code(), read);
    }
    catch(std::system_error const& e)
    {
        handler(e.code(), 0);
    }

    if(!read_queue_.empty())
    {
        dispatcher_.want_read(socket_, [this]{ do_read(); });
    }
}
