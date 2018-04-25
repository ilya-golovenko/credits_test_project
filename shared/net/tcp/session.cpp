#include "session.hpp"


net::tcp::session::session(socket&& socket, event::dispatcher& dispatcher) :
    socket_(std::move(socket)),
    dispatcher_(dispatcher)
{}

net::tcp::session::session(event::dispatcher& dispatcher) :
    dispatcher_(dispatcher)
{}

net::tcp::session::~session()
{
    close();
}

net::tcp::socket& net::tcp::session::get_socket()
{
    return socket_;
}

void net::tcp::session::write(const_buffer_sequence const& buffers, completion_handler handler)
{
    dispatcher_.want_write(socket_, [=](auto&& error){ do_write(error, {buffers, handler, 0}); });
}

void net::tcp::session::read(mutable_buffer_sequence const& buffers, completion_handler handler)
{
    dispatcher_.want_read(socket_, [=](auto&& error){ do_read(error, {buffers, handler, 0}); });
}

void net::tcp::session::write(const_buffer const& buffer, completion_handler handler)
{
    dispatcher_.want_write(socket_, [=](auto&& error){ do_write(error, {{buffer}, handler, 0}); });
}

void net::tcp::session::read(mutable_buffer const& buffer, completion_handler handler)
{
    dispatcher_.want_read(socket_, [=](auto&& error){ do_read(error, {{buffer}, handler, 0}); });
}

void net::tcp::session::close()
{
    if(socket_.valid())
    {
        dispatcher_.cancel_writes(socket_);
        dispatcher_.cancel_reads(socket_);

        detail::shutdown_socket(socket_);

        socket_.close();
    }
}

net::tcp::session::operator int() const
{
    return socket_.get();
}

void net::tcp::session::do_write(std::error_code error, write_operation op)
{
    if(!error)
    {
        try
        {
            op.size += socket_.write(op.buffers);
        }
        catch(std::system_error const& e)
        {
            error = e.code();
        }
    }

    if(!error)
    {
        if(!op.buffers.empty())
        {
            return dispatcher_.want_write(socket_, [this, op](auto&& error){ do_write(error, op); });
        }
    }

    op.handler(error, op.size);
}

void net::tcp::session::do_read(std::error_code error, read_operation op)
{
    if(!error)
    {
        try
        {
            std::size_t size = socket_.read(op.buffers);

            if(size > 0)
            {
                op.size += size;
            }
            else
            {
                close();
            }
        }
        catch(std::system_error const& e)
        {
            error = e.code();
        }
    }

    op.handler(error, op.size);
}
