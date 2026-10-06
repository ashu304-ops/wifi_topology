#pragma once

#include "../models/Device.h"

#include <string>
#include <vector>

class ARPScanner {
public:
    std::vector<Device> scan(
        const std::string& interfaceName,
        const std::string& localIp,
        const std::string& network,
        const std::string& netmask
    );
};