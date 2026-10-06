#include "NetworkInterface.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace {

std::string getMacAddress(const std::string& interfaceName) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    if (fd < 0) {
        return "Unknown";
    }

    struct ifreq ifr {};
    std::snprintf(ifr.ifr_name, IFNAMSIZ, "%s", interfaceName.c_str());

    if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0) {
        close(fd);
        return "Unknown";
    }

    close(fd);

    unsigned char* mac =
        reinterpret_cast<unsigned char*>(ifr.ifr_hwaddr.sa_data);

    std::ostringstream result;

    result << std::hex << std::setfill('0')
           << std::setw(2) << static_cast<int>(mac[0]) << ":"
           << std::setw(2) << static_cast<int>(mac[1]) << ":"
           << std::setw(2) << static_cast<int>(mac[2]) << ":"
           << std::setw(2) << static_cast<int>(mac[3]) << ":"
           << std::setw(2) << static_cast<int>(mac[4]) << ":"
           << std::setw(2) << static_cast<int>(mac[5]);

    return result.str();
}

std::string getGateway() {
    std::ifstream routeFile("/proc/net/route");

    if (!routeFile.is_open()) {
        return "Unknown";
    }

    std::string line;

    // Skip header
    std::getline(routeFile, line);

    while (std::getline(routeFile, line)) {
        std::istringstream stream(line);

        std::string interface;
        std::string destination;
        std::string gatewayHex;
        std::string flags;

        stream >> interface >> destination >> gatewayHex >> flags;

        // Destination 00000000 = default route
        if (destination == "00000000" && gatewayHex != "00000000") {

            unsigned long gateway =
                std::stoul(gatewayHex, nullptr, 16);

            struct in_addr address {};
            address.s_addr = gateway;

            return inet_ntoa(address);
        }
    }

    return "Unknown";
}

std::string calculateNetworkAddress(
    const std::string& ip,
    const std::string& netmask) {

    struct in_addr ipAddr {};
    struct in_addr maskAddr {};

    if (inet_pton(AF_INET, ip.c_str(), &ipAddr) != 1) {
        return "Unknown";
    }

    if (inet_pton(AF_INET, netmask.c_str(), &maskAddr) != 1) {
        return "Unknown";
    }

    struct in_addr network {};

    network.s_addr = ipAddr.s_addr & maskAddr.s_addr;

    return inet_ntoa(network);
}

} // namespace

NetworkInfo NetworkInterface::getNetworkInfo() {

    NetworkInfo info;

    struct ifaddrs* interfaces = nullptr;

    if (getifaddrs(&interfaces) == -1) {
        throw std::runtime_error("Unable to get network interfaces");
    }

    for (struct ifaddrs* current = interfaces;
         current != nullptr;
         current = current->ifa_next) {

        if (current->ifa_addr == nullptr) {
            continue;
        }

        if (current->ifa_addr->sa_family != AF_INET) {
            continue;
        }

        std::string interfaceName = current->ifa_name;

        // Ignore loopback
        if (interfaceName == "lo") {
            continue;
        }

        struct sockaddr_in* address =
            reinterpret_cast<struct sockaddr_in*>(
                current->ifa_addr);

        struct sockaddr_in* netmask =
            reinterpret_cast<struct sockaddr_in*>(
                current->ifa_netmask);

        char ipBuffer[INET_ADDRSTRLEN];
        char maskBuffer[INET_ADDRSTRLEN];

        inet_ntop(
            AF_INET,
            &address->sin_addr,
            ipBuffer,
            sizeof(ipBuffer));

        inet_ntop(
            AF_INET,
            &netmask->sin_addr,
            maskBuffer,
            sizeof(maskBuffer));

        info.interfaceName = interfaceName;
        info.ipAddress = ipBuffer;
        info.netmask = maskBuffer;

        info.networkAddress =
            calculateNetworkAddress(
                info.ipAddress,
                info.netmask);

        info.gateway = getGateway();

        info.macAddress =
            getMacAddress(info.interfaceName);

        break;
    }

    freeifaddrs(interfaces);

    return info;
}