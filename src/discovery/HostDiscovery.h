#pragma once

#include "../models/Device.h"
#include "../network/ARPScanner.h"
#include "../network/NetworkInterface.h"

#include <vector>

class HostDiscovery {
public:
    std::vector<Device> discover(
        const NetworkInfo& networkInfo
    );

private:
    std::string resolveHostname(
        const std::string& ip
    );
};