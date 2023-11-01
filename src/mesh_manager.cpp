#include "mesh_manager.h"
#include "mesh_reader.h"
#include <string>
#include <fstream>

mesh_composition mesh_manager::separate_elements(msh_data data)
{
    //find diferent elements in element data
    int counts[4] = {0,0,0,0};

    for(auto element : data.msh_elements)
    {
        switch (element.element_type)
        {
        case 15:
            counts[0]++;
            break;

        case 1:
            counts[1]++;
            break;

        case 2:
            counts[2]++;
            break;

        case 3:
            counts[3]++;
            break;

        default:
            break;
        }
    }

    std::cout << "Physical domains: \t" << data.physical_domains.size() << "\n";
    std::cout << "Point entities:\t" << data.msh_entities.dim_counts[0] << "\n";
    std::cout << "Line entities:\t" << data.msh_entities.dim_counts[1] << "\n";
    std::cout << "Surface entities:\t" << data.msh_entities.dim_counts[2] << "\n";
    std::cout << "Volume entities:\t" << data.msh_entities.dim_counts[3] << "\n";
    std::cout << "Nodes:\t" << counts[0] << "\n";
    std::cout << "Faces:\t" << counts[1] << "\n";
    std::cout << "Trigs:\t" << counts[2] << "\n";
    std::cout << "Quads:\t" << counts[3] << "\n";

    mesh_composition output;

    output.N_faces = counts[1];
    output.N_triangles = counts[2];
    output.N_quads = counts[3];

    return output;
}

mesh_manager::mesh_manager(){}

mesh_manager::~mesh_manager(){}

void mesh_manager::read_mesh(std::string file_path)
{
    mesh_reader reader;
    msh_data mesh = reader.read_msh4(file_path);

    //separate face elements
    composition = separate_elements(mesh);
    //partition mesh in blocks and chunks
    // partition_mesh(mesh);
}

void mesh_manager::export_mesh_VTK(std::string file_path)
{
    
}
