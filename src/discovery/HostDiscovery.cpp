#include "HostDiscovery.h"

#include <arpa/inet.h>
#include <netdb.h>

#include <iostream>

std::string HostDiscovery::resolveHostname(
    const std::string& ip) {

    sockaddr_in address{};

    address.sin_family = AF_INET;

    if (inet_pton(
            AF_INET,
            ip.c_str(),
            &address.sin_addr) != 1) {

        return "Unknown";
    }

    char hostname[NI_MAXHOST]{};

    int result = getnameinfo(
        reinterpret_cast<sockaddr*>(&address),
        sizeof(address),
        hostname,
        sizeof(hostname),
        nullptr,
        0,
        NI_NAMEREQD
    );

    if (result != 0) {
        return "Unknown";
    }

    return hostname;
}


std::vector<Device> HostDiscovery::discover(
    const NetworkInfo& networkInfo) {

    ARPScanner arpScanner;

    std::vector<Device> devices =
        arpScanner.scan(
            networkInfo.interfaceName,
            networkInfo.ipAddress,
            networkInfo.networkAddress,
            networkInfo.netmask
        );


    /*
     * Add our own machine.
     *
     * ARP scanning normally does not return
     * our own machine because we are the sender.
     */
    Device localDevice;

    localDevice.ip =
        networkInfo.ipAddress;

    localDevice.mac =
        networkInfo.macAddress;

    localDevice.hostname =
        resolveHostname(
            networkInfo.ipAddress
        );

    localDevice.isLocalMachine = true;

    devices.insert(
        devices.begin(),
        localDevice
    );


    /*
     * Enrich discovered devices.
     */
    for (auto& device : devices) {

        if (device.isLocalMachine) {
            continue;
        }

        device.hostname =
            resolveHostname(
                device.ip
            );


        if (device.ip ==
            networkInfo.gateway) {

            device.isGateway = true;
        }
    }


    return devices;
}