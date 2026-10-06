#include "TopologyExporter.h"

#include <fstream>
#include <iostream>

void TopologyExporter::exportDot(
    const std::string& filename,
    const std::vector<TopologyNode>& nodes,
    const std::vector<TopologyEdge>& edges) const {

    std::ofstream file(filename);

    if (!file) {
        std::cerr
            << "Failed to create DOT file: "
            << filename
            << '\n';

        return;
    }

    file << "graph WiFiScope {\n\n";

    // ==============================
    // Graph configuration
    // ==============================

    file << "    graph [\n";
    file << "        layout=neato,\n";
    file << "        overlap=false,\n";
    file << "        splines=true,\n";
    file << "        bgcolor=\"white\",\n";
    file << "        label=\"WiFiScope - Wi-Fi Network Topology\",\n";
    file << "        labelloc=t,\n";
    file << "        fontsize=24,\n";
    file << "        fontname=\"Arial\"\n";
    file << "    ];\n\n";

    // ==============================
    // Default node configuration
    // ==============================

    file << "    node [\n";
    file << "        fontname=\"Arial\",\n";
    file << "        fontsize=11,\n";
    file << "        style=\"filled,rounded\",\n";
    file << "        margin=0.2\n";
    file << "    ];\n\n";

    // ==============================
    // Connection configuration
    // ==============================

    file << "    edge [\n";
    file << "        penwidth=2,\n";
    file << "        color=\"gray40\"\n";
    file << "    ];\n\n";

    // ==============================
    // Internet node
    // ==============================

    std::string gatewayIp;

    for (const auto& node : nodes) {

        if (node.isGateway) {
            gatewayIp = node.ip;
            break;
        }
    }

    if (!gatewayIp.empty()) {

        file << "    internet [\n";
        file << "        label=\"INTERNET\",\n";
        file << "        shape=cloud,\n";
        file << "        style=filled,\n";
        file << "        fontsize=14\n";
        file << "    ];\n\n";

        file << "    internet -- \""
             << gatewayIp
             << "\" [penwidth=3];\n\n";
    }

    // ==============================
    // Device nodes
    // ==============================

    for (const auto& node : nodes) {

        std::string label;

        if (node.isGateway) {

            label =
                "ROUTER / GATEWAY"
                "\\n"
                + node.hostname
                + "\\n"
                + node.ip
                + "\\n"
                + node.mac;

            file
                << "    \""
                << node.ip
                << "\" [\n";

            file
                << "        label=\""
                << label
                << "\",\n";

            file
                << "        shape=box3d,\n";
            file << "        fontsize=12\n";
            file << "    ];\n";
        }

        else if (node.isLocalMachine) {

            label =
                "LOCAL MACHINE"
                "\\n"
                + node.hostname
                + "\\n"
                + node.ip
                + "\\n"
                + node.mac;

            file
                << "    \""
                << node.ip
                << "\" [\n";

            file
                << "        label=\""
                << label
                << "\",\n";

            file << "        shape=box,\n";
            file << "        fontsize=12\n";
            file << "    ];\n";
        }

        else {

            label =
                "DEVICE"
                "\\n"
                + node.hostname
                + "\\n"
                + node.ip
                + "\\n"
                + node.mac;

            file
                << "    \""
                << node.ip
                << "\" [\n";

            file
                << "        label=\""
                << label
                << "\",\n";

            file << "        shape=ellipse\n";
            file << "    ];\n";
        }

        file << '\n';
    }

    // ==============================
    // Connections
    // ==============================

    for (const auto& edge : edges) {

        file
            << "    \""
            << edge.from
            << "\" -- \""
            << edge.to
            << "\";\n";
    }

    file << "\n}\n";

    file.close();

    std::cout
        << "\nTopology exported to: "
        << filename
        << '\n';
}