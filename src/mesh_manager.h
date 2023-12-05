#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "mesh_reader.h"
#include "mesh_reader_structs.h"

#define MAX_CHUNK_SIZE 8;

extern std::map<int, std::vector<int>> element_type_to_props;

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
    int Dimension = 0;          // Mesh dimension
    int N_mesh_blocks = 1;      // Number of blocks in mesh

    int N_points;                                           // 0D elements
    int N_lines;                                            // 1D elements
    int N_triangles, N_quads;                               // 2D elements
    int N_tetrahedra, N_prisms, N_pyramids, N_hexahedra;    // 3D elements

    int N_elements;                 // Number of all elements
    int N_faces;                    // Number of internal faces in mesh
    int N_element_vertices;         // Number of all vertices for all elements
    int N_nodes;                    // Number of mesh nodes
    int N_boundary_elements;        // Number of boundary elements faces/lines

    double* node_pos_array;         // Node coordinates

    double *V_array;                        // Element volume array
    uint8_t *Element_type_array;            // Array of element types (GMSH types)  
    uint8_t *Phys_idx_array;                // Physical index of each element
    int32_t *Element_vertices_idx_array;    // Element vertices
    int32_t *Element_vertices_idx_offsets;  // Array of indices where element vertex data starts
    uint32_t *Boundary_idxs_array;           // Index array of boundary elements

    uint32_t *Face_vertices_idx_array;       // 
    uint32_t *Face_vertices_idx_offsets;     // 
    uint32_t *Face_ON_idx;

    std::vector<uint8_t> Element_types;         // Which elements are solved 2D=trigs/quads 3D=(tetra,hexa,prisms...)
    std::vector<uint8_t> Face_element_types;    // Which elements are faces 2D=lines 3D=(triangles,quads)

    std::vector<mesh_block> blocks;         // Mesh blocks

    // Func
    void free_data();
    mesh_struct();
    ~mesh_struct();
};

class mesh_manager
{
    private:
    void print_info();

    // Basic
    void mesh_dimension(const msh_data& data);

    // Parsing
    void parse_mesh_boundary(const msh_data& data);
    void parse_mesh_nodes(const msh_data& data);
    void parse_mesh_elements(const msh_data& data);

    // Boundary
    msh_element add_ghost_element(const msh_element& element, const int where);

    // Face construction and manipulation
    void construct_internal_faces();
    void find_adjency_structure(int32_t** _xadj, int32_t** _adjncy, int n_common);
    void find_unique_faces(int32_t** _xadj, int32_t** _adjncy);
    void find_face_nodes();

    // Partitioning
    void partition_mesh();

    public:
    mesh_struct mesh;
    
    //constructors
    mesh_manager();
    ~mesh_manager();

    void read_mesh(std::string file_path);
    void compute_volumes();
    void export_mesh_VTK(std::string file_path);
};