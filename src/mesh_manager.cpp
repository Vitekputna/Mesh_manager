#include "mesh_manager.h"
#include "mesh_reader.h"
#include <string>
#include <fstream>
#include <math.h>

mesh_struct::mesh_struct()
{
    std::cout << "Mesh struct constructor\n";
    node_pos = nullptr;
    V = nullptr;
    N_faces = nullptr;
    Phys_idx = nullptr;
    Vertex_node_idxs = nullptr;
}

void mesh_struct::free_data()
{
    free(node_pos);
    free(V);
    free(N_faces);
    free(Phys_idx);
    free(Vertex_node_idxs);
}

mesh_struct::~mesh_struct()
{
    std::cout << "Freeing mesh struct\n";
    free_data();
    // node_pos = nullptr;
    // V = nullptr;
    // E_type = nullptr;
    // Phys_idx = nullptr;
    // Vertex_node_idxs = nullptr;
}

mesh_composition mesh_manager::separate_elements(const msh_data& data)
{
    //find diferent elements in element data
    int counts[4] = {0,0,0,0};

    // for(auto element : data.msh_elements)
    // {
    //     // std::cout << int(element.element_type) << "\n"; 
    //     switch (element.element_type)
    //     {
    //     case 3:
    //         counts[1]++;
    //         break;

    //     case 4:
    //         counts[2]++;
    //         break;

    //     default:
    //         break;
    //     }
    // }

    std::cout << "Physical domains: \t" << data.physical_domains.size() << "\n";
    std::cout << "Point entities:\t" << data.msh_entities.dim_counts[0] << "\n";
    std::cout << "Line entities:\t" << data.msh_entities.dim_counts[1] << "\n";
    std::cout << "Surface entities:\t" << data.msh_entities.dim_counts[2] << "\n";
    std::cout << "Volume entities:\t" << data.msh_entities.dim_counts[3] << "\n";
    std::cout << "Nodes:\t" << data.N_nodes << "\n";
    std::cout << "Faces:\t" << counts[1] << "\n";
    std::cout << "Trigs:\t" << counts[2] << "\n";
    std::cout << "Quads:\t" << counts[3] << "\n";

    mesh_composition output;

    output.N_nodes = data.N_nodes;
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
    parse_mesh_nodes(mesh);
    parse_mesh_elements(mesh);
    compute_volumes();
    // partition_mesh(mesh);

}

void mesh_manager::export_mesh_VTK(std::string file_path){}

void mesh_manager::parse_mesh_nodes(const msh_data& data)
{
    std::cout << "Parsing mesh nodes\n";
    const int N = data.N_nodes;

    // Check mesh data if allocated
    if(mesh.node_pos != nullptr)
    {
        std::cout << "Rewriting existing mesh, exiting\n";
        exit(1);
    }

    // Allocate memory for node pos data
    mesh.node_pos = (double*)malloc(3*N*sizeof(double));

    // Write to node pos memory
    int node_idx = 0;
    for(const auto& node : data.msh_nodes)
    {
        mesh.node_pos[node_idx] = node.x;
        mesh.node_pos[node_idx+1] = node.y;
        mesh.node_pos[node_idx+2] = node.z;

        node_idx += 3;
    }
    std::cout << "Parsing mesh nodes done...\n";
}

void mesh_manager::parse_mesh_elements(const msh_data& data)
{
    std::cout << "Parsing mesh elements\n";
    const int N_elements = composition.N_quads+composition.N_triangles;
    const int N_element_vertices = composition.N_quads*4+composition.N_triangles*3;

    mesh.N_faces = (int*)malloc(N_elements*sizeof(int));
    mesh.Phys_idx = (int*)malloc(N_elements*sizeof(int));
    mesh.Vertex_node_idxs = (int*)malloc(N_element_vertices*sizeof(int)); 

    //WIP
    mesh.blocks.resize(2);

    mesh.blocks[0].N_chunks_in_block = 1;
    mesh.blocks[0].chunks.resize(1);

    mesh.blocks[1].N_chunks_in_block = 1;
    mesh.blocks[1].chunks.resize(1);

    int i = 0, j = 0;
    for(const auto& element : data.msh_elements)
    {
        //Skiping point elements
        if(element.element_type == 15 || element.element_type == 1){continue;}

        mesh.blocks[element.element_type-2].chunks[0].N_elements++;

        mesh.N_faces[i] = int(element.element_type);
        mesh.Phys_idx[i] = int(element.physical_idx);
        i++;
        
        for(auto const E_vertex : element.node_idxs)
        {
            mesh.Vertex_node_idxs[j] = E_vertex;
            j++;
        }
    }
    std::cout << "Parsing mesh nodes done...\n";
}

void mesh_manager::compute_volumes()
{
    const int N_elements = composition.N_triangles+composition.N_quads;

    mesh.V = (double*)malloc(N_elements*sizeof(double));

    int k = 0;
    for(int i = 0; i < N_elements; i++)
    {
        const int n = mesh.N_faces[i];
        std::cout << n << " ";

        for(int j = 0; j < n; j++)
        {
            std::cout << mesh.Vertex_node_idxs[k] << " ";
            k++;
        }
        std::cout << "\n";
    }
}
