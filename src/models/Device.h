#pragma once

#include <string>
#include <vector>

struct PortInfo {
    int port;
    bool open;
};

struct Device {
    std::string ip;
    std::string mac;
    std::string hostname;

    bool isGateway = false;
    bool isLocalMachine = false;

    std::vector<PortInfo> ports;
};