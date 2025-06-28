#include "redis_server.h"
#include "redis_database.h"

#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char* argv[]) {
    int port = 6379;
    if (argc >= 2) port = std::stoi(argv[1]);

    if (RedisDatabase::getInstance().load("dump.my_rdb")) {
        std::cout << "Database loaded from dump.my_rdb\n";
    } else {
        std::cout << "No dump found or load failed , starting with an empty database\n";
    }

    RedisServer server(port);

    // Save the database every 5 mins, persistance storage
    std::thread persistanceThread([](){
        while (true) {
            std::this_thread::sleep_for(std::chrono::seconds(300));

            // Save db
            if (!RedisDatabase::getInstance().dump("dump.my_rdb")) {
                std::cerr << "Error dumping database\n";
            } else {
                std::cout << "Database dumped to dump.my_rdb";
            }
        }
    });

    persistanceThread.detach();

    server.run();

    return 0;
}