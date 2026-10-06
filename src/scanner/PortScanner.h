#pragma once

#include "../models/Device.h"

#include <string>
#include <vector>

class PortScanner {
public:

    std::vector<PortInfo> scan(
        const std::string& ip,
        const std::vector<int>& ports
    );

private:

    bool isPortOpen(
        const std::string& ip,
        int port
    );
};