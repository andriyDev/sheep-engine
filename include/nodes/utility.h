
#pragma once

#include <memory>
#include <vector>

#include "nodes/node.h"

// Constructs a pre-order traversal of `root`.
std::vector<std::shared_ptr<Node>> CollectPreOrderNodes(
    const std::shared_ptr<Node>& root);

// Constructs a post-order traversal of `root`.
std::vector<std::shared_ptr<Node>> CollectPostOrderNodes(
    const std::shared_ptr<Node>& root);
