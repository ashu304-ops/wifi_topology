#pragma once

#include "TopologyBuilder.h"

#include <string>

class TopologyExporter {
public:

    void exportDot(
        const std::string& filename,
        const std::vector<TopologyNode>& nodes,
        const std::vector<TopologyEdge>& edges
    ) const;
};