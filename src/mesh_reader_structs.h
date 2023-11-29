#pragma once
#include <vector>
#include <string>

typedef float position_type;

//Holds one node data
struct msh_node
{
    int idx;
    position_type x,y,z;
};

//This struct holds one element data
struct msh_element
{
    int idx;
    int N_faces, physical_idx, element_type;
    std::vector<int> node_idxs;
};

struct physical_domain
{
    char idx, type;
    std::string name;
};

//Entity struct for msh ver.4
struct entity
{
    int idx, phys_tag;
    int dim;
    // position_type x_min,y_min,z_min;
    // position_type x_max,y_max,z_max;
};

//Holds all entity data of given mesh
struct entities
{
    int N_entities=0;
    std::vector<entity> entity_vector;
    std::vector<int> dim_counts = {0,0,0,0};
};

//Holds data for whole mesh for return and next operations
struct msh_data
{
    int N_nodes, N_physicals;   // Number of nodes and physical domains (line surface and volume)
    int N_elements;             // Number of all elements

    // Number of all domain elements
    int N_points=0;                                           // 0D elements
    int N_lines=0;                                            // 1D elements
    int N_triangles=0, N_quads=0;                               // 2D elements
    int N_tetrahedra=0, N_prisms=0, N_pyramids=0, N_hexahedra=0;    // 3D elements

    // Boundary elements
    int N_boundary_elements=0;

    //list of entities
    entities msh_entities;

    //list of physical domains
    std::vector<physical_domain> physical_domains;

    //nodes
    std::vector<msh_node> msh_nodes;
    
    //elements
    std::vector<msh_element> msh_elements;
};