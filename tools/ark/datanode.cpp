#include "datanode.h"

#include <bit>
#include <utility>

namespace Tools::Ark {

DataNode DataNode::makeInt(std::int32_t value) {
    DataNode node;
    node.value = static_cast<std::uint32_t>(value);
    return node;
}

DataNode DataNode::makeSymbol(std::string symbol) {
    DataNode node;
    node.type = Type::Symbol;
    node.symbol = std::move(symbol);
    return node;
}

DataNode DataNode::makeFloat(float value) {
    DataNode node;
    node.type = Type::Float;
    node.value = std::bit_cast<std::uint32_t>(value);
    return node;
}

DataNode DataNode::makeArray(std::uint16_t line, std::int32_t file) {
    DataNode node;
    node.type = Type::Array;
    node.line = line;
    node.file = file;
    return node;
}

bool DataNode::isKeyed(std::string_view key) const {
    return type == Type::Array && !children.empty() && children.front().type == Type::Symbol &&
           children.front().symbol == key;
}

DataNode *DataNode::findArray(std::string_view key) {
    const auto index = indexOf(key);
    return index < 0 ? nullptr : &children[static_cast<std::size_t>(index)];
}

const DataNode *DataNode::findArray(std::string_view key) const {
    const auto index = indexOf(key);
    return index < 0 ? nullptr : &children[static_cast<std::size_t>(index)];
}

int DataNode::indexOf(std::string_view key) const {
    for (std::size_t i = 0; i < children.size(); ++i) {
        if (children[i].isKeyed(key)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

} // namespace Tools::Ark
