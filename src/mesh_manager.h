#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "mesh_reader.h"

#define MAX_CHUNK_SIZE 8;

//cache blocking
struct mesh_chunk
{
    //x_coor
    //y_coor
    //y_coor

    //face_areas
    //face_N/O info
    //face_normals
    //volumes
};

//core partitions
struct mesh_block
{
    std::vector<mesh_chunk> chunks;
};

//array of mesh blocks (whole mesh)
struct mesh
{
    std::vector<mesh_block> blocks;
};

struct mesh_composition
{
    //assumes ordered data faces->trigs->quads
    int N_faces, N_triangles, N_quads;
    int N_boundary_faces;
};

class mesh_manager
{
    private:
    mesh_composition separate_elements(msh_data data);
    void partition_mesh(msh_data data);

    public:
    mesh_composition composition;
    
    //constructors
    mesh_manager();
    ~mesh_manager();

    void read_mesh(std::string file_path);
    void export_mesh_VTK(std::string file_path);
};