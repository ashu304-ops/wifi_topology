#include "ARPScanner.h"

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

namespace {

struct ARPHeader {
    uint16_t hardwareType;
    uint16_t protocolType;
    uint8_t hardwareLength;
    uint8_t protocolLength;
    uint16_t operation;

    unsigned char senderMac[6];
    unsigned char senderIp[4];

    unsigned char targetMac[6];
    unsigned char targetIp[4];
};

std::string macToString(const unsigned char* mac) {

    std::ostringstream result;

    result << std::hex;

    for (int i = 0; i < 6; ++i) {

        if (i != 0) {
            result << ":";
        }

        result << static_cast<int>(mac[i]);
    }

    return result.str();
}

uint32_t ipToInteger(const std::string& ip) {

    struct in_addr address {};

    if (inet_pton(AF_INET, ip.c_str(), &address) != 1) {
        throw std::runtime_error(
            "Invalid IPv4 address: " + ip
        );
    }

    return ntohl(address.s_addr);
}

} // namespace


std::vector<Device> ARPScanner::scan(
    const std::string& interfaceName,
    const std::string& localIp,
    const std::string& network,
    const std::string& netmask) {

    std::vector<Device> devices;

    /*
     * AF_PACKET + SOCK_RAW allows us to create
     * Ethernet/ARP packets directly.
     *
     * This requires root privileges or CAP_NET_RAW.
     */
    int sock = socket(
        AF_PACKET,
        SOCK_RAW,
        htons(ETH_P_ARP)
    );

    if (sock < 0) {

        perror("socket");

        return devices;
    }


    // --------------------------------------------------
    // Get interface index
    // --------------------------------------------------

    struct ifreq ifr {};

    std::strncpy(
        ifr.ifr_name,
        interfaceName.c_str(),
        IFNAMSIZ - 1
    );

    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {

        perror("SIOCGIFINDEX");

        close(sock);

        return devices;
    }

    int interfaceIndex = ifr.ifr_ifindex;


    // --------------------------------------------------
    // Get local MAC address
    // --------------------------------------------------

    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {

        perror("SIOCGIFHWADDR");

        close(sock);

        return devices;
    }

    unsigned char localMac[6];

    std::memcpy(
        localMac,
        ifr.ifr_hwaddr.sa_data,
        6
    );


    // --------------------------------------------------
    // Calculate network range
    // --------------------------------------------------

    uint32_t networkAddress =
        ipToInteger(network);

    uint32_t mask =
        ipToInteger(netmask);

    uint32_t firstHost =
        (networkAddress & mask) + 1;

    uint32_t lastHost =
        (networkAddress | (~mask)) - 1;


    std::cout << "\nScanning network: "
              << network
              << "/24\n\n";


    // --------------------------------------------------
    // Broadcast MAC address
    // FF:FF:FF:FF:FF:FF
    // --------------------------------------------------

    unsigned char broadcastMac[6];

    std::memset(
        broadcastMac,
        0xff,
        sizeof(broadcastMac)
    );


    // --------------------------------------------------
    // Convert OUR IP
    // --------------------------------------------------

    uint32_t localIpInteger =
        ipToInteger(localIp);

    uint32_t localIpNetwork =
        htonl(localIpInteger);


    // --------------------------------------------------
    // Send ARP request to every host
    // --------------------------------------------------

    for (uint32_t target = firstHost;
         target <= lastHost;
         ++target) {

        unsigned char packet[42] {};

        /*
         * Ethernet Header
         *
         * Destination MAC : Broadcast
         * Source MAC      : Our MAC
         * EtherType       : ARP
         */

        std::memcpy(
            packet,
            broadcastMac,
            6
        );

        std::memcpy(
            packet + 6,
            localMac,
            6
        );

        // EtherType = ARP
        packet[12] = 0x08;
        packet[13] = 0x06;


        /*
         * ARP Header starts after
         * 14-byte Ethernet header.
         */

        ARPHeader* arp =
            reinterpret_cast<ARPHeader*>(
                packet + 14
            );


        // Ethernet
        arp->hardwareType =
            htons(1);

        // IPv4
        arp->protocolType =
            htons(0x0800);

        // MAC = 6 bytes
        arp->hardwareLength = 6;

        // IPv4 = 4 bytes
        arp->protocolLength = 4;

        // ARP REQUEST
        arp->operation =
            htons(1);


        // --------------------------------------------------
        // Sender
        // --------------------------------------------------

        std::memcpy(
            arp->senderMac,
            localMac,
            6
        );

        std::memcpy(
            arp->senderIp,
            &localIpNetwork,
            4
        );


        // --------------------------------------------------
        // Target
        // --------------------------------------------------

        std::memset(
            arp->targetMac,
            0,
            6
        );

        uint32_t targetNetwork =
            htonl(target);

        std::memcpy(
            arp->targetIp,
            &targetNetwork,
            4
        );


        // --------------------------------------------------
        // Destination
        // --------------------------------------------------

        struct sockaddr_ll destination {};

        destination.sll_family =
            AF_PACKET;

        destination.sll_ifindex =
            interfaceIndex;

        destination.sll_halen =
            6;

        std::memcpy(
            destination.sll_addr,
            broadcastMac,
            6
        );


        // --------------------------------------------------
        // Send packet
        // --------------------------------------------------

        ssize_t sent =
            sendto(
                sock,
                packet,
                sizeof(packet),
                0,
                reinterpret_cast<struct sockaddr*>(
                    &destination
                ),
                sizeof(destination)
            );

        if (sent < 0) {
            perror("sendto");
        }
    }


    // --------------------------------------------------
    // Wait for ARP replies
    // --------------------------------------------------

    timeval timeout {};

    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    setsockopt(
        sock,
        SOL_SOCKET,
        SO_RCVTIMEO,
        &timeout,
        sizeof(timeout)
    );


    unsigned char buffer[2048];


    while (true) {

        ssize_t received =
            recvfrom(
                sock,
                buffer,
                sizeof(buffer),
                0,
                nullptr,
                nullptr
            );

        if (received <= 0) {
            break;
        }

        // Ethernet + ARP header
        if (received < 42) {
            continue;
        }


        ARPHeader* arp =
            reinterpret_cast<ARPHeader*>(
                buffer + 14
            );


        // We only want ARP replies
        if (ntohs(arp->operation) != 2) {
            continue;
        }


        // --------------------------------------------------
        // Convert sender IP to string
        // --------------------------------------------------

        char ipBuffer[INET_ADDRSTRLEN] {};

        inet_ntop(
            AF_INET,
            arp->senderIp,
            ipBuffer,
            sizeof(ipBuffer)
        );


        // --------------------------------------------------
        // Create device
        // --------------------------------------------------

        Device device;

        device.ip =
            ipBuffer;

        device.mac =
            macToString(
                arp->senderMac
            );


        devices.push_back(device);
    }


    close(sock);

    return devices;
}