
#pragma once

#include "../models/Device.h"

#include <string>
#include <vector>

struct TopologyNode {
    std::string ip;
    std::string mac;
    std::string hostname;

    bool isGateway = false;
    bool isLocalMachine = false;
};

struct TopologyEdge {
    std::string from;
    std::string to;
};

class TopologyBuilder {
public:

    void build(
        const std::vector<Device>& devices
    );

    void printTopology() const;

    const std::vector<TopologyNode>& getNodes() const;

    const std::vector<TopologyEdge>& getEdges() const;

private:

    std::vector<TopologyNode> nodes;

    std::vector<TopologyEdge> edges;
};
