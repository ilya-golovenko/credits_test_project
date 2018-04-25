#include <event/dispatcher.hpp>
#include <net/tcp/client.hpp>
#include <transport/client.hpp>
#include <logging/logger.hpp>

#include <iostream>
#include <string>
#include <random>
#include <thread>
#include <atomic>
#include <chrono>


char const alphabet[] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};

std::atomic_bool stopped(false);

void send_transactions(transport::client& transport)
{
    using namespace std::chrono_literals;

    try
    {
        std::default_random_engine gen(std::random_device{}());

        std::uniform_int_distribution<std::uint32_t> dis32;
        std::uniform_int_distribution<std::uint64_t> dis64;

        std::uniform_int_distribution<> dis_abc(0, std::size(alphabet) - 1);

        auto rand_char = [&]{ return alphabet[dis_abc(gen)]; };

        std::size_t counter = 0;

        std::vector<transaction> transactions(100, transaction{});

        auto start = std::chrono::high_resolution_clock::now();

        while(!stopped)
        {
            for(transaction& txn : transactions)
            {
                std::generate_n(txn.source, std::size(txn.source) - 1, rand_char);
                std::generate_n(txn.target, std::size(txn.target) - 1, rand_char);

                std::generate_n(txn.currency, std::size(txn.currency) - 1, rand_char);

                txn.amount  = dis32(gen);
                txn.amount1 = dis64(gen);

                txn.number  = counter++;
            }

            transport.send(transactions);
        }

        auto end = std::chrono::high_resolution_clock::now();

        std::chrono::duration<double, std::milli> time = end - start;

        std::cout << "sent " << counter << " transactions, " << (counter / time.count() * 1000) << " txn/s" << std::endl;
    }
    catch(std::exception const& e)
    {
        logging::error("caught std::exception: ", e.what());
    }
    catch(...)
    {
        logging::error("caught unexpected exception");
    }
}

void process_transaction(transaction const& txn)
{
    logging::info("received transaction: ", txn);
}

int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cerr << "USAGE: client <server> <port>" << std::endl;
        return EXIT_SUCCESS;
    }

    logging::set_log_file("client.log");

    try
    {
        event::dispatcher dispatcher;
        net::tcp::client  client(dispatcher);
        transport::client transport(client, process_transaction);

        transport.connect(argv[1], std::stoul(argv[2]));

        std::thread thread(&send_transactions, std::ref(transport));

        std::cin.get();

        stopped = true;
        thread.join();
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
