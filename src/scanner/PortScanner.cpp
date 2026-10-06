#include "PortScanner.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <future>
#include <iostream>
#include <mutex>
#include <thread>
#include <algorithm>
bool PortScanner::isPortOpen(
    const std::string& ip,
    int port) {

    int sock = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (sock < 0) {
        return false;
    }

    sockaddr_in address{};

    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(
            AF_INET,
            ip.c_str(),
            &address.sin_addr) <= 0) {

        close(sock);
        return false;
    }

    timeval timeout{};

    timeout.tv_sec = 0;
    timeout.tv_usec = 200000;

    setsockopt(
        sock,
        SOL_SOCKET,
        SO_SNDTIMEO,
        &timeout,
        sizeof(timeout)
    );

    int result = connect(
        sock,
        reinterpret_cast<sockaddr*>(&address),
        sizeof(address)
    );

    close(sock);

    return result == 0;
}


std::vector<PortInfo> PortScanner::scan(
    const std::string& ip,
    const std::vector<int>& ports) {

    std::vector<PortInfo> results;

    std::mutex resultsMutex;

    std::vector<std::future<void>> tasks;

    /*
     * Limit the number of concurrent scans.
     */

    const unsigned int maxThreads = 4;

    std::vector<int> openPorts;

    std::mutex openPortsMutex;

    auto scanPort =
        [&](int port) {

            bool open =
                isPortOpen(ip, port);

            if (open) {

                std::lock_guard<std::mutex> lock(
                    openPortsMutex
                );

                openPorts.push_back(port);
            }
        };


    std::cout
        << "\nScanning ports on "
        << ip
        << "...\n";


    /*
     * Submit ports in batches.
     */

    for (std::size_t i = 0;
         i < ports.size();
         i += maxThreads) {

        tasks.clear();

        std::size_t end =
            std::min(
                i + maxThreads,
                ports.size()
            );


        for (std::size_t j = i;
             j < end;
             ++j) {

            tasks.push_back(
                std::async(
                    std::launch::async,
                    scanPort,
                    ports[j]
                )
            );
        }


        /*
         * Wait for this batch.
         */

        for (auto& task : tasks) {
            task.get();
        }
    }


    /*
     * Convert open ports into PortInfo objects.
     */

    for (int port : openPorts) {

        std::cout
            << "  [OPEN] "
            << port
            << '\n';

        results.push_back({
            port,
            true
        });
    }


    return results;
}