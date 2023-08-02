#pragma once

#include <stdint.h>

#include <pugixml/pugixml.hpp>

inline uint32_t kwasutils_get_xml_child_count(pugi::xml_node* node, const char* child_name)
{
    return node->select_nodes(child_name).size();
}

inline uint8_t kwasuitls_does_node_exists(pugi::xml_document& doc, const char* node_name)
{
    pugi::xml_node node = doc.child(node_name);
    return !node.empty();
}