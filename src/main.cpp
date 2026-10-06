
#include "network/NetworkInterface.h"
#include "discovery/HostDiscovery.h"
#include "scanner/PortScanner.h"
#include "topology/TopologyBuilder.h"
#include "topology/TopologyExporter.h"

#include <iostream>
#include <vector>

int main() {

    try {

        // --------------------------------------------
        // Network information
        // --------------------------------------------

        NetworkInterface network;

        NetworkInfo info =
            network.getNetworkInfo();


        std::cout << "\n";

        std::cout
            << "====================================\n";

        std::cout
            << "       WiFiScope Network Mapper\n";

        std::cout
            << "====================================\n\n";


        std::cout
            << "Interface       : "
            << info.interfaceName
            << '\n';

        std::cout
            << "IP Address      : "
            << info.ipAddress
            << '\n';

        std::cout
            << "Subnet Mask     : "
            << info.netmask
            << '\n';

        std::cout
            << "Network Address : "
            << info.networkAddress
            << '\n';

        std::cout
            << "Gateway         : "
            << info.gateway
            << '\n';

        std::cout
            << "MAC Address     : "
            << info.macAddress
            << '\n';


        // --------------------------------------------
        // Host discovery
        // --------------------------------------------

        HostDiscovery discovery;

        std::vector<Device> devices =
            discovery.discover(info);


        // --------------------------------------------
        // Port scanning
        // --------------------------------------------

        PortScanner portScanner;

        // Common TCP ports
        std::vector<int> commonPorts = {

            22,      // SSH
            23,      // Telnet
            53,      // DNS
            80,      // HTTP
            110,     // POP3
            139,     // NetBIOS
            443,     // HTTPS
            445,     // SMB
            3306,    // MySQL
            5432,    // PostgreSQL
            8080     // HTTP Alternative
        };


        std::cout << "\n";

        std::cout
            << "====================================\n";

        std::cout
            << "          PORT SCANNING\n";

        std::cout
            << "====================================\n";


        for (auto& device : devices) {

            device.ports =
                portScanner.scan(
                    device.ip,
                    commonPorts
                );
        }


        // --------------------------------------------
        // Device discovery results
        // --------------------------------------------

        std::cout << "\n";

        std::cout
            << "====================================\n";

        std::cout
            << "          DEVICE DISCOVERY\n";

        std::cout
            << "====================================\n\n";


        for (const auto& device : devices) {

            if (device.isGateway) {

                std::cout
                    << "[ROUTER / GATEWAY]\n";
            }

            else if (device.isLocalMachine) {

                std::cout
                    << "[LOCAL MACHINE]\n";
            }

            else {

                std::cout
                    << "[DEVICE]\n";
            }


            std::cout
                << "IP       : "
                << device.ip
                << '\n';

            std::cout
                << "MAC      : "
                << device.mac
                << '\n';

            std::cout
                << "Hostname : "
                << device.hostname
                << '\n';


            // ----------------------------------------
            // Open ports
            // ----------------------------------------

            std::cout
                << "Open Ports: ";


            if (device.ports.empty()) {

                std::cout
                    << "None";
            }

            else {

                for (const auto& port :
                     device.ports) {

                    std::cout
                        << port.port
                        << " ";
                }
            }


            std::cout << "\n\n";
        }


        // --------------------------------------------
        // Summary
        // --------------------------------------------

        std::cout
            << "------------------------------------\n";

        std::cout
            << "Devices discovered: "
            << devices.size()
            << '\n';

        std::cout
            << "------------------------------------\n";
        TopologyBuilder topology;

        topology.build(devices);

        topology.printTopology();
        TopologyExporter exporter;

        exporter.exportDot(
        "network.dot",
        topology.getNodes(),
        topology.getEdges()
       );
    }


    catch (const std::exception& e) {

        std::cerr
            << "Error: "
            << e.what()
            << '\n';

        return 1;
    }




    return 0;
}
