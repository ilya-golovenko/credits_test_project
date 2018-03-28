#include <log/logger.hpp>
#include <network/dispatcher.hpp>
#include <network/client.hpp>
#include <transport/client.hpp>

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
        std::cerr << "USAGE: client <server> <port>" << std::endl;
        return EXIT_SUCCESS;
    }

    log::set_log_file("client.log");

    try
    {
        tcp::dispatcher   dispatcher;
        tcp::client       client(dispatcher);
        transport::client transport(client, process_transaction);

        dispatcher.start();

        transport.connect(argv[1], std::stoul(argv[2]));

        transaction txn1 = { "abcdef", "fedcba", 123, 456, "usd", 1 };
        transaction txn2 = { "fedcba", "abcdef", 321, 654, "eur", 2 };

        transport.send({ txn1, txn2 });

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
