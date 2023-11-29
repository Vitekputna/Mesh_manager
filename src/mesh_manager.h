#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "mesh_reader.h"
#include "mesh_reader_structs.h"

#define MAX_CHUNK_SIZE 8;

//cache blocking
//Has to contain only one type of elements
struct mesh_chunk
{
    int N_elements = 0;
    // //face_areas
    double* Face_areas;
    //volumes
    double *Volumes;

    // //face_N/O info
    // int* Neighbour_indexes;
    // int* Owner_indexes;
    // //face_normals
    // double* xf_norm;
    // double* yf_norm;
    // double* zf_norm;
};

//core partitions
struct mesh_block
{
    int N_chunks_in_block;
    std::vector<int> Element_type_in_chunk;

    std::vector<mesh_chunk> chunks;
};

//array of mesh blocks (whole mesh)
struct mesh_struct
{
    int dimension = 0;  // Mesh dimension

    int N_points;                                           // 0D elements
    int N_lines;                                            // 1D elements
    int N_triangles, N_quads;                               // 2D elements
    int N_tetrahedra, N_prisms, N_pyramids, N_hexahedra;    // 3D elements

    int N_elements;                 // Number of all elements
    int N_faces;                    // Number of internal faces in mesh
    int N_element_vertices;         // Number of all vertices for all elements

    int N_nodes;                    // Number of mesh nodes
    int N_boundary_elements;        // Number of boundary elements faces/lines

    // Number of blocks in mesh
    int N_mesh_blocks = 1;

    // Node coordinates
    double* node_pos_array;

    // Element properties
    double *V_array;
    int *N_faces_array, *Phys_idx_array;

    // Element vertices
    int *Vertex_node_idxs_array;

    // Mesh blocks
    std::vector<mesh_block> blocks;

    // Func
    void free_data();
    mesh_struct();
    ~mesh_struct();
};

class mesh_manager
{
    private:
    void init_size(const msh_data& data);
    void print_info();
    void mesh_dimension();
    void parse_mesh_boundary(const msh_data& data);
    void parse_mesh_nodes(const msh_data& data);
    void parse_mesh_elements(const msh_data& data);
    void partition_mesh(const msh_data& data);

    public:
    mesh_struct mesh;
    
    //constructors
    mesh_manager();
    ~mesh_manager();

    void read_mesh(std::string file_path);
    void compute_volumes();
    void export_mesh_VTK(std::string file_path);
};