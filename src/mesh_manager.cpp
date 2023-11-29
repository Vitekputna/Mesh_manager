#include "mesh_manager.h"
#include "mesh_reader.h"
#include "mesh_reader_structs.h"
#include <string>
#include <fstream>
#include <algorithm>
#include <math.h>
#include <iterator>
#include "helper_functions.h"

mesh_struct::mesh_struct()
{
    std::cout << "Mesh struct constructor\n";
    node_pos_array = nullptr;
    V_array = nullptr;
    N_faces_array = nullptr;
    Phys_idx_array = nullptr;
    Vertex_node_idxs_array = nullptr;
}

void mesh_struct::free_data()
{
    free(node_pos_array);
    free(V_array);
    free(N_faces_array);
    free(Phys_idx_array);
    free(Vertex_node_idxs_array);
}

mesh_struct::~mesh_struct()
{
    std::cout << "Freeing mesh struct\n";
    free_data();
}

mesh_manager::mesh_manager(){}

mesh_manager::~mesh_manager(){}

void mesh_manager::print_info()
{   
    // std::cout << "Physical domains: \t" << data.physical_domains.size() << "\n";
    // std::cout << "Entity information\n";
    // std::cout << "Point entities:\t" << data.msh_entities.dim_counts[0] << "\n";
    // std::cout << "Line entities:\t" << data.msh_entities.dim_counts[1] << "\n"; 
    // std::cout << "Surface entities:\t" << data.msh_entities.dim_counts[2] << "\n";
    // std::cout << "Volume entities:\t" << data.msh_entities.dim_counts[3] << "\n";

    std::cout << "Faces:\t" << mesh.N_boundary_elements << "\n";
    std::cout << "Nodes:\t" << mesh.N_nodes << "\n";

    std::cout << "1D element counts\n";
    std::cout << "Lines: \t\t" << mesh.N_lines << "\n";
    std::cout << "2D element counts\n";
    std::cout << "Trigs:\t\t" << mesh.N_triangles << "\n";
    std::cout << "Quads:\t\t" << mesh.N_quads << "\n";
    std::cout << "3D element counts\n";
    std::cout << "Tetrahedra:\t" << mesh.N_tetrahedra << "\n";
    std::cout << "Prisms:\t\t" << mesh.N_prisms << "\n";
    std::cout << "Pyramids:\t" << mesh.N_pyramids << "\n";
    std::cout << "Hexahedra:\t" << mesh.N_hexahedra << "\n";
}

void mesh_manager::init_size(const msh_data& data)
{
    mesh.N_nodes = data.N_nodes;
    mesh.N_lines = data.N_lines;
    mesh.N_points = data.N_points;
    mesh.N_triangles = data.N_triangles;
    mesh.N_quads = data.N_quads;
    mesh.N_tetrahedra = data.N_tetrahedra;
    mesh.N_prisms = data.N_prisms;
    mesh.N_pyramids = data.N_pyramids;
    mesh.N_hexahedra = data.N_hexahedra;

    mesh.N_elements = mesh.N_triangles+mesh.N_quads+mesh.N_tetrahedra
                      +mesh.N_prisms+mesh.N_pyramids+mesh.N_hexahedra;

    mesh.N_element_vertices = mesh.N_triangles*3+mesh.N_quads*4+mesh.N_tetrahedra*4
                      +mesh.N_prisms*6+mesh.N_pyramids*5+mesh.N_hexahedra*8;                     
}

void mesh_manager::mesh_dimension()
{
    const int N_3D = mesh.N_tetrahedra+mesh.N_prisms+mesh.N_pyramids+mesh.N_hexahedra;
    const int N_2D = mesh.N_triangles+mesh.N_quads;

    if (N_2D > 0 && N_3D == 0)
    {
        mesh.dimension = 2;
    }
    else if(N_3D > 0 && N_2D > 0)
    {
        mesh.dimension = 3;
    }
    else
    {
        std::cout << "Mesh dimension could not be set, exiting...\n";
        exit(1);
    }

    std::cout << "Mesh dimension is:\t" << mesh.dimension << "\n";
    
}

void mesh_manager::parse_mesh_boundary(const msh_data& data)
{
    int N_boundary = 0;
    std::vector<int> element_types;

    if(mesh.dimension == 2)
    {
        element_types = std::vector<int>{1};
    }
    else if (mesh.dimension == 3)
    {
        element_types = std::vector<int>{2,3};
    }
    else{exit(1);}

    for(auto const& type : element_types)
    {
        auto it = std::vector<msh_element>::const_iterator();
        auto pos = data.msh_elements.begin();

        do
        {
            it = std::find_if(pos,data.msh_elements.end(),[type](const msh_element& el){return el.element_type == type;});
            auto idx = std::distance(data.msh_elements.begin(),it);

            int i = idx;
            while(data.msh_elements[i].element_type == type)
            {
                N_boundary++;
                std::advance(it,1);
                i++;
            }

            pos = it;

        } while(it != data.msh_elements.end());
    }

    mesh.N_boundary_elements = N_boundary;
}

void mesh_manager::read_mesh(std::string file_path)
{
    mesh_reader reader;
    msh_data mesh = reader.read_msh4(file_path);

    // Count elemets and print
    init_size(mesh);
    // Get mesh dimension
    mesh_dimension();
    //partition mesh in blocks and chunks
    parse_mesh_boundary(mesh);
    // parse_mesh_nodes(mesh);
    // parse_mesh_elements(mesh);
    // compute_volumes();
    // partition_mesh(mesh);
    print_info();
}

void mesh_manager::export_mesh_VTK(std::string file_path){}

void mesh_manager::parse_mesh_nodes(const msh_data& data)
{
    std::cout << "Parsing mesh nodes\n";
    const int N = data.N_nodes;

    // Check mesh data if allocated
    check_if_allocated<double>(mesh.node_pos_array);

    // Allocate memory for node pos data
    mesh.node_pos_array = (double*)malloc(3*N*sizeof(double));

    // Write to node pos memory
    int node_idx = 0;
    for(const auto& node : data.msh_nodes)
    {
        mesh.node_pos_array[node_idx] = node.x;
        mesh.node_pos_array[node_idx+1] = node.y;
        mesh.node_pos_array[node_idx+2] = node.z;

        node_idx += 3;
    }
    std::cout << "Parsing mesh nodes done...\n";
}

void mesh_manager::parse_mesh_elements(const msh_data& data)
{
    std::cout << "Parsing mesh elements\n";
    const int N_elements = mesh.N_elements;
    const int N_element_vertices = mesh.N_element_vertices;

    mesh.N_faces_array = (int*)malloc(N_elements*sizeof(int));                    // Number of faces element has
    mesh.Phys_idx_array = (int*)malloc(N_elements*sizeof(int));                   // Physical index of element
    mesh.Vertex_node_idxs_array = (int*)malloc(N_element_vertices*sizeof(int));   // List of vertex nodes idxs for all elements

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

        std::cout << element.element_type-2 << "\n";
        
        mesh.blocks[element.element_type-2].chunks[0].N_elements++;

        mesh.N_faces_array[i] = element.N_faces;
        mesh.Phys_idx_array[i] = element.physical_idx;
        i++;
        
        for(auto const E_vertex : element.node_idxs)
        {
            mesh.Vertex_node_idxs_array[j] = E_vertex;
            j++;
        }
    }
    std::cout << "Parsing mesh nodes done...\n";
}

void mesh_manager::compute_volumes()
{
    const int N_elements = mesh.N_elements;

    mesh.V_array = (double*)malloc(N_elements*sizeof(double));

    int k = 0;
    for(int i = 0; i < N_elements; i++)
    {
        const int n = mesh.N_faces_array[i];

        for(int j = 0; j < n; j++)
        {
            // std::cout << mesh.Vertex_node_idxs_array[k] << " ";
            k++;
        }
    }
}
