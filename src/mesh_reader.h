#pragma once
#include <vector>
#include <string>
#include <map>
#include "mesh_reader_structs.h"

// Convert msh element type to elements number of faces
extern std::map<int,int> msh_Nfaces;

class mesh_reader
{
    private:
    std::string supported_version = "4.1 0 8";
    std::vector<std::string> split(std::string line, std::string delimiter);
    physical_domain read_domain(std::string line);
    entity read_entity(std::string line);
    msh_node read_node_line(std::string line);
    msh_element read_element_line(std::string line);

    void count_elements(msh_data& data);

    public:
    msh_data read_msh(std::string file_path);
    msh_data read_msh4(std::string file_path);
};