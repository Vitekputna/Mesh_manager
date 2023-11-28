#include "mesh_manager.h"
// #include "mesh_reader.h"

int main()
{
    mesh_manager manager;
    manager.read_mesh("triquad_physical.msh");

    // manager.compute_distance();
    // std::cout << manager.mesh.blocks[0].chunks[0].N_elements << "\n";
    // std::cout << manager.mesh.blocks[1].chunks[0].N_elements << "\n";

    // for(int i = 0; i < manager.composition.N_triangles+manager.composition.N_quads;i++){std::cout << manager.mesh.E_type[i] << "\n";}
    // for(int i = 0; i < manager.composition.N_triangles+manager.composition.N_quads;i++){std::cout << manager.mesh.Phys_idx[i] << "\n";}

    // for(int i = 0; i < manager.composition.N_triangles*3+manager.composition.N_quads*4;i++)
    // {
    //     std::cout << manager.mesh.Vertex_node_idxs[i] << "\n";
    // }
}