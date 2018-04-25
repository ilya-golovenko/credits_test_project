#include <event/dispatcher.hpp>
#include <net/tcp/server.hpp>
#include <transport/server.hpp>
#include <logging/logger.hpp>

#include <iostream>
#include <string>


void process_transaction(transaction const& txn)
{
    //logging::info("received transaction: ", txn);
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cerr << "USAGE: server <address> <port>" << std::endl;
        return EXIT_SUCCESS;
    }

    logging::set_log_file("server.log");

    try
    {
        event::dispatcher dispatcher;
        net::tcp::server  server(dispatcher);
        transport::server transport(server, process_transaction);

        transport.listen(argv[1], std::stoul(argv[2]));

        std::cin.get();
    }
    catch(std::exception const& e)
    {
        logging::error("caught std::exception: ", e.what());
        return EXIT_FAILURE;
    }
    catch(...)
    {
        logging::error("caught unexpected exception");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
