
#include "TopologyBuilder.h"

#include <iostream>

void TopologyBuilder::build(
    const std::vector<Device>& devices) {

    nodes.clear();
    edges.clear();

    std::string gatewayIp;

    // Create graph nodes
    for (const auto& device : devices) {

        TopologyNode node;

        node.ip = device.ip;
        node.mac = device.mac;
        node.hostname = device.hostname;
        node.isGateway = device.isGateway;
        node.isLocalMachine = device.isLocalMachine;

        nodes.push_back(node);

        if (device.isGateway) {
            gatewayIp = device.ip;
        }
    }

    // Create logical connections
    if (!gatewayIp.empty()) {

        for (const auto& device : devices) {

            if (device.isGateway) {
                continue;
            }

            TopologyEdge edge;

            edge.from = gatewayIp;
            edge.to = device.ip;

            edges.push_back(edge);
        }
    }
}

void TopologyBuilder::printTopology() const {

    std::cout
        << "\n====================================\n";

    std::cout
        << "          NETWORK TOPOLOGY\n";

    std::cout
        << "====================================\n\n";

    std::cout
        << "Nodes:\n";

    for (const auto& node : nodes) {

        std::cout << "  ";

        if (node.isGateway) {
            std::cout << "[GATEWAY] ";
        }
        else if (node.isLocalMachine) {
            std::cout << "[LOCAL]   ";
        }
        else {
            std::cout << "[DEVICE]  ";
        }

        std::cout
            << node.ip
            << " | "
            << node.hostname
            << " | "
            << node.mac
            << '\n';
    }

    std::cout << "\nConnections:\n";

    for (const auto& edge : edges) {

        std::cout
            << "  "
            << edge.from
            << "  --->  "
            << edge.to
            << '\n';
    }

    std::cout
        << "\n------------------------------------\n";

    std::cout
        << "Nodes       : "
        << nodes.size()
        << '\n';

    std::cout
        << "Connections : "
        << edges.size()
        << '\n';

    std::cout
        << "------------------------------------\n";
}

const std::vector<TopologyNode>&
TopologyBuilder::getNodes() const {

    return nodes;
}

const std::vector<TopologyEdge>&
TopologyBuilder::getEdges() const {

    return edges;
}
