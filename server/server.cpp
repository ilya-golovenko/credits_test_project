#include <log/logger.hpp>
#include <network/dispatcher.hpp>
#include <network/server.hpp>
#include <transport/server.hpp>

#include <iostream>
#include <string>


void process_transaction(transaction const& txn)
{
    log::info("received transaction: ", txn);
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cerr << "USAGE: server <address> <port>" << std::endl;
        return EXIT_SUCCESS;
    }

    log::set_log_file("server.log");

    try
    {
        tcp::dispatcher   dispatcher;
        tcp::server       server(dispatcher);
        transport::server transport(server, process_transaction);

        dispatcher.start();

        transport.listen(argv[1], std::stoul(argv[2]));

        std::cin.get();
    }
    catch(std::exception const& e)
    {
        log::error("caught std::exception: ", e.what());
        return EXIT_FAILURE;
    }
    catch(...)
    {
        log::error("caught unexpected exception");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
