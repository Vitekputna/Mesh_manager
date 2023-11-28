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
    // Mesh dimension
    int dimension = 2;

    // Number of blocks in mesh
    int N_mesh_blocks = 1;

    // Node coordinates
    double* node_pos;

    // Element properties
    double *V;
    int *N_faces, *Phys_idx;

    // Element vertices
    int *Vertex_node_idxs;

    // Mesh blocks
    std::vector<mesh_block> blocks;

    // Func
    void free_data();
    mesh_struct();
    ~mesh_struct();
};

struct mesh_composition
{
    //assumes ordered data faces->trigs->quads
    int N_nodes;
    int N_faces, N_triangles, N_quads;
    int N_boundary_faces;
};

class mesh_manager
{
    private:
    mesh_composition separate_elements(const msh_data& data);
    void partition_mesh(const msh_data& data);

    public:
    mesh_composition composition;
    mesh_struct mesh;
    
    //constructors
    mesh_manager();
    ~mesh_manager();

    void read_mesh(std::string file_path);
    void parse_mesh_nodes(const msh_data& data);
    void parse_mesh_elements(const msh_data& data);
    
    void compute_volumes();
    
    void export_mesh_VTK(std::string file_path);
};