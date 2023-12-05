#include "mesh_manager.h"
#include "mesh_reader.h"
#include "mesh_reader_structs.h"
#include <string>
#include <fstream>
#include <algorithm>
#include <math.h>
#include <iterator>
#include <metis.h>
#include <set>
#include "helper_functions.h"

// Maps element type to {N_vertices,N_faces}
std::map<int, std::vector<int>> element_type_to_props  = 
                                   {{1,std::vector<int>{2,1}},      // Line
                                    {2,std::vector<int>{3,3}},      // Triangle
                                    {3,std::vector<int>{4,4}},      // Quadrangle
                                    {4,std::vector<int>{4,4}},      // Tetrahedron
                                    {5,std::vector<int>{8,6}},      // Hexahedron
                                    {6,std::vector<int>{6,5}},      // Prism
                                    {7,std::vector<int>{5,5}}};     // Pyramid

mesh_struct::mesh_struct()
{
    std::cout << "Mesh struct constructor\n";
    node_pos_array = nullptr;
    V_array = nullptr;
    Element_type_array = nullptr;
    Phys_idx_array = nullptr;
    Boundary_idxs_array = nullptr;
    Element_vertices_idx_array = nullptr;
    Element_vertices_idx_offsets = nullptr;

    Face_vertices_idx_array = nullptr;
    Face_vertices_idx_offsets = nullptr;
    Face_ON_idx = nullptr;    
}

void mesh_struct::free_data()
{
    free(node_pos_array);
    free(V_array);
    free(Element_type_array);
    free(Phys_idx_array);
    free(Boundary_idxs_array);
    free(Element_vertices_idx_array);
    free(Element_vertices_idx_offsets);

    free(Face_vertices_idx_array);
    free(Face_vertices_idx_offsets);
    free(Face_ON_idx);
}

mesh_struct::~mesh_struct()
{
    std::cout << "Freeing mesh struct\n";
    free_data();
}

mesh_manager::mesh_manager(){}

mesh_manager::~mesh_manager(){}

// Prints some info to terminal
void mesh_manager::print_info()
{ 
    // std::cout << "Physical domains: \t" << data.physical_domains.size() << "\n";
    // std::cout << "Entity information\n";
    // std::cout << "Point entities:\t" << data.msh_entities.dim_counts[0] << "\n";
    // std::cout << "Line entities:\t" << data.msh_entities.dim_counts[1] << "\n"; 
    // std::cout << "Surface entities:\t" << data.msh_entities.dim_counts[2] << "\n";
    // std::cout << "Volume entities:\t" << data.msh_entities.dim_counts[3] << "\n";

    std::cout << "Elements: " << mesh.N_elements << "\n";
    std::cout << "Vertices: " << mesh.N_element_vertices << "\n";

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

// Computes mesh dimension, element counts, face element types, volume element types and boundary size
void mesh_manager::mesh_dimension(const msh_data& data)
{
   
    mesh.N_lines = data.N_lines;
    mesh.N_points = data.N_points;
    mesh.N_triangles = data.N_triangles;
    mesh.N_quads = data.N_quads;
    mesh.N_tetrahedra = data.N_tetrahedra;
    mesh.N_prisms = data.N_prisms;
    mesh.N_pyramids = data.N_pyramids;
    mesh.N_hexahedra = data.N_hexahedra;

    const int N_3D = mesh.N_tetrahedra+mesh.N_prisms+mesh.N_pyramids+mesh.N_hexahedra;
    const int N_2D = mesh.N_triangles+mesh.N_quads;
    const int N_1D = mesh.N_lines;

    if (N_2D > 0 && N_3D == 0)
    {
        mesh.Dimension = 2;
        mesh.Face_element_types = std::vector<uint8_t>{1};
        mesh.Element_types = std::vector<uint8_t>{2,3};

        mesh.N_elements = N_2D+N_1D;
        mesh.N_boundary_elements = N_1D;

        mesh.N_element_vertices = mesh.N_lines*3+mesh.N_triangles*3+mesh.N_quads*4;
    }
    else if(N_3D > 0 && N_2D > 0)
    {
        mesh.Dimension = 3;
        mesh.Face_element_types = std::vector<uint8_t>{2,3};
        mesh.Element_types = std::vector<uint8_t>{4,5,6,7};

        mesh.N_elements = N_3D;// add boundary elements
        mesh.N_boundary_elements = N_2D;

        mesh.N_element_vertices = mesh.N_tetrahedra*4+mesh.N_prisms*6+mesh.N_pyramids*5+mesh.N_hexahedra*8; // add boundary elements
    }
    else
    {
        std::cout << "Mesh dimension could not be set, exiting...\n";
        exit(1);
    }

    mesh.N_nodes = data.N_nodes+mesh.N_boundary_elements;
    std::cout << "Mesh dimension is:\t" << mesh.Dimension << "\n";
}

// Read and parse mesh
void mesh_manager::read_mesh(std::string file_path)
{
    mesh_reader reader;
    msh_data read_mesh = reader.read_msh4(file_path, std::vector<int>{15});

    mesh_dimension(read_mesh);      // Get mesh dimension
    
    // parse_mesh_boundary(read_mesh); // Parse boundary data
    parse_mesh_nodes(read_mesh);    // Parse nodes 
    parse_mesh_elements(read_mesh); // Parse elements
    construct_internal_faces();
    // compute_volumes();
    // partition_mesh(mesh);
    print_info();                   // Print info to terminal


    for(int i = 0; i < mesh.N_faces; i++)
    {
        std::cout << i << " " << mesh.Face_ON_idx[2*i] << " " << mesh.Face_ON_idx[2*i+1] << "\n";
    }

    // for(int i = 0; i < mesh.N_nodes; i++)
    // {
    //     std::cout << mesh.node_pos_array[3*i] << " ";
    //     std::cout << mesh.node_pos_array[3*i+1] << " ";
    //     std::cout << mesh.node_pos_array[3*i+2] << "\n";
    // }


    // for(int i = 0; i < mesh.N_boundary_elements; i++)
    // {
    //     const int element_idx = mesh.Boundary_idxs_array[i];

    //     std::cout << element_idx << ":\t";

    //     for(int j = mesh.Element_vertices_idx_offsets[element_idx]; j < mesh.Element_vertices_idx_offsets[element_idx+1];j++)
    //     {
    //         std::cout << mesh.Element_vertices_idx_array[j] << " ";
    //     }
    //     std::cout << "\n";
    // }

    // for(int i = 0; i < mesh.N_elements; i++)
    // {
    //     std::cout << i << ":\t";

    //     for(int j = mesh.Element_vertices_idx_offsets[i]; j < mesh.Element_vertices_idx_offsets[i+1];j++)
    //     {
    //         std::cout << mesh.Element_vertices_idx_array[j] << " ";
    //     }
    //     std::cout << "\n";
    // }

}

// Export to legacy VTK format
void mesh_manager::export_mesh_VTK(std::string file_path){}

// Parse mesh boundary, deprecated
void mesh_manager::parse_mesh_boundary(const msh_data& data)
{
    if(mesh.Dimension == 2)
    {
        const int N_boundary_face_indices = data.N_lines*2;
        mesh.Face_vertices_idx_array = (uint32_t*)malloc(N_boundary_face_indices*sizeof(uint32_t));
        mesh.Face_vertices_idx_offsets = (uint32_t*)malloc(mesh.N_boundary_elements*sizeof(uint32_t));
    }
    else if (mesh.Dimension == 3)
    {
        const int N_boundary_face_indices = data.N_triangles*2+data.N_quads*4;
        mesh.Face_vertices_idx_array = (uint32_t*)malloc(N_boundary_face_indices*sizeof(uint32_t));
        mesh.Face_vertices_idx_offsets = (uint32_t*)malloc(mesh.N_boundary_elements*sizeof(uint32_t));
    }
    else{exit(1);}

    int i = 0, j = 0;
    for(auto const& element : data.msh_elements)
    {
        if(contains(mesh.Face_element_types,(uint8_t)element.element_type))
        {
            mesh.Face_vertices_idx_offsets[i] = j;
            i++;

            for(auto const E_vertex : element.node_idxs)
            {
                mesh.Face_vertices_idx_array[j] = E_vertex;
                j++;
            }
        }
    }
}

// Allocate mesh node coor. arrays and parse data
void mesh_manager::parse_mesh_nodes(const msh_data& data)
{
    std::cout << "Parsing mesh nodes\n";
    const int N = mesh.N_nodes;

    // Check mesh data if allocated
    check_if_allocated<double>(mesh.node_pos_array);

    // Allocate memory for node pos data
    mesh.node_pos_array = (double*)malloc(3*(N)*sizeof(double));

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

// Alocate mesh element idx and offset data
void mesh_manager::parse_mesh_elements(const msh_data& data)
{
    std::cout << "Parsing mesh elements\n";
    const int N_elements = mesh.N_elements;
    const int N_element_vertices = mesh.N_element_vertices;
    const int N_element_offsets = N_elements+1;

    mesh.Element_type_array = (uint8_t*)malloc(N_elements*sizeof(uint8_t));                          // Element type array
    mesh.Phys_idx_array = (uint8_t*)malloc(N_elements*sizeof(uint8_t));                              // Physical index of element
    mesh.Element_vertices_idx_array = (int32_t*)malloc(N_element_vertices*sizeof(uint32_t));         // List of vertex nodes idxs for all elements
    mesh.Element_vertices_idx_offsets = (int32_t*)malloc(N_element_offsets*sizeof(uint32_t));           // Where data for vertices starts for given element
    mesh.Boundary_idxs_array = (uint32_t*)malloc(mesh.N_boundary_elements*sizeof(uint32_t));

    int i = 0, j = 0, k = 0;
    for(const auto& element : data.msh_elements)
    {
        if(!contains(mesh.Element_types,(uint8_t)element.element_type))
        {
            // Create and add ghost element
            auto ghost = add_ghost_element(element,k);

            mesh.Element_type_array[i] = ghost.element_type;
            mesh.Phys_idx_array[i] = ghost.physical_idx;
            mesh.Element_vertices_idx_offsets[i] = j;
            i++;
            
            for(auto const E_vertex : ghost.node_idxs)
            {
                mesh.Element_vertices_idx_array[j] = E_vertex;
                j++;
            }

            mesh.Boundary_idxs_array[k] = i;
            k++;
            continue;
        }

        mesh.Element_type_array[i] = element.element_type;
        mesh.Phys_idx_array[i] = element.physical_idx;
        mesh.Element_vertices_idx_offsets[i] = j;
        i++;
        
        for(auto const E_vertex : element.node_idxs)
        {
            mesh.Element_vertices_idx_array[j] = E_vertex;
            j++;
        }
    }

    mesh.Element_vertices_idx_offsets[N_element_offsets-1] = N_element_vertices;

    std::cout << "Parsing mesh nodes done...\n";
}

//
void mesh_manager::compute_volumes()
{
    const int N_elements = mesh.N_elements;

    mesh.V_array = (double*)malloc(N_elements*sizeof(double));

    int k = 0;
    for(int i = 0; i < N_elements; i++)
    {
        const int n = element_type_to_props[mesh.Element_type_array[i]][0];

        for(int j = 0; j < n; j++)
        {
            std::cout << mesh.Element_vertices_idx_array[k] << " ";
            k++;
        }
    }
}

// Adjust boundary elements from file (adds a node)
msh_element mesh_manager::add_ghost_element(const msh_element& element, const int where)
{
    const int i = (mesh.N_nodes-1-where); // Where to write

    
    double x=0,y=0,z=0;

    int n_nodes = 0;
    for(auto const& node : element.node_idxs)
    {
        x += mesh.node_pos_array[3*node];
        y += mesh.node_pos_array[3*node+1];
        z += mesh.node_pos_array[3*node+2];
        n_nodes++;
    }   

    x = x/n_nodes;
    y = y/n_nodes;
    z = z/n_nodes;

    mesh.node_pos_array[3*i] = x;
    mesh.node_pos_array[3*i+1] = y;
    mesh.node_pos_array[3*i+2] = z;

    msh_element ghost;
    ghost.N_faces = 1;
    ghost.element_type = element.element_type;
    ghost.physical_idx = element.physical_idx;

    auto ghost_vertices = element.node_idxs;
    ghost_vertices.push_back(i);

    ghost.node_idxs = ghost_vertices;

    return ghost;
}

void mesh_manager::find_adjency_structure(idx_t** _xadj, idx_t** _adjncy, int n_common)
{
    idx_t N_nodes = mesh.N_nodes;
    idx_t N_elements = mesh.N_elements;

    idx_t numFlag = 0;
    idx_t nCommon = 2;

    // idx_t *xadj, *adjncy;
    idx_t *eptr, *eind;

    eind = mesh.Element_vertices_idx_array;
    eptr = mesh.Element_vertices_idx_offsets;

    auto output = METIS_MeshToDual(&N_elements,&N_nodes,eptr,eind,&nCommon,&numFlag,_xadj,_adjncy);

    if(output == METIS_OK)
    {
        std::cout << "METIS ok\n";
    }
}

void mesh_manager::find_unique_faces(idx_t** _xadj, idx_t** _adjncy)
{
    std::set<std::pair<int, int>> uniquePairs;

    // Extract pairs and store in the set
    for (int i = 0; i < mesh.N_elements; ++i) {
        for (int j = (*_xadj)[i]; j < (*_xadj)[i + 1]; ++j) {
            int source = i; // Current node
            int target = (*_adjncy)[j]; // Adjacent node

            // Ensure the pair is stored in a consistent order
            std::pair<int, int> currentPair = std::make_pair(std::min(source, target), std::max(source, target));
            uniquePairs.insert(currentPair);
        }
    }



    mesh.N_faces = uniquePairs.size();
    mesh.Face_ON_idx = (uint32_t*)malloc(2*mesh.N_faces*sizeof(uint32_t));

    int index = 0;
    for (const auto& pair : uniquePairs) {
        mesh.Face_ON_idx[index++] = pair.first;
        mesh.Face_ON_idx[index++] = pair.second;
    }
}

void mesh_manager::find_face_nodes()
{
    int preallocate_size;
    if(mesh.Dimension == 2) preallocate_size = 2;
    else preallocate_size = 4;

    std::vector<int32_t> vec1;
    std::vector<int32_t> vec2;
    vec1.reserve(preallocate_size);
    vec2.reserve(preallocate_size);
    
    int owner_idx, neighbour_idx; 
    int32_t *owner_p, *neighbour_p;
    for(int i = 0; i < mesh.N_faces; i++)
    {
        owner_idx = mesh.Face_ON_idx[2*i];
        neighbour_idx = mesh.Face_ON_idx[2*i+1];

        owner_p = &mesh.Element_vertices_idx_offsets[owner_idx];
        neighbour_p = &mesh.Element_vertices_idx_offsets[neighbour_idx];

        
    }
}

// Calls metis for adjency structure WIP
void mesh_manager::construct_internal_faces()
{
    int n_common;

    if(mesh.Dimension == 2) n_common = 2;
    else if (mesh.Dimension == 3) n_common = 3;
    else exit(1);

    idx_t *xadj, *adjncy;
    find_adjency_structure(&xadj,&adjncy,n_common);
    find_unique_faces(&xadj,&adjncy);
    
    free(xadj);
    free(adjncy);

    find_face_nodes();

}