#pragma once

#include <string>

struct NetworkInfo {
    std::string interfaceName;
    std::string ipAddress;
    std::string netmask;
    std::string networkAddress;
    std::string gateway;
    std::string macAddress;
};

class NetworkInterface {
public:
    NetworkInfo getNetworkInfo();
};