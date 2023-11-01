#include "mesh_reader.h"
#include "mesh_reader_structs.h"
#include "fstream"
#include <vector>
#include <string>
#include <iostream>

std::vector<std::string> mesh_reader::split(std::string line, std::string delimiter)
{
    auto output = std::vector<std::string>();

    size_t pos = 0;
    std::string token;
    while ((pos = line.find(delimiter)) != std::string::npos) {
        token = line.substr(0, pos);
        output.push_back(token);
        line.erase(0, pos + delimiter.length());
    }
    output.push_back(line);
    return output;
}

physical_domain mesh_reader::read_domain(std::string line)
{
    auto list = split(line," ");

    physical_domain output;
    output.idx = std::stoi(list[1]);
    output.name = list[2];
    output.type = std::stoi(list[0]);

    return output;
}

msh_node mesh_reader::read_node_line(std::string line)
{
    auto list = split(line," ");

    msh_node node;
    node.idx = std::stoi(list[0])-1;
    node.x = std::stod(list[1]);
    node.y = std::stod(list[2]); 
    node.z = std::stod(list[3]);
    return node;
}

msh_element mesh_reader::read_element_line(std::string line)
{
    auto list = split(line," ");

    msh_element element;
    element.idx = stoi(list[0])-1;
    element.element_type = stoi(list[1]);
    element.physical_idx = stoi(list[3]);

    int n_points = list.size()-5;
    element.node_idxs.resize(n_points);
    for(int i = 0; i < n_points; i++)
    {
        element.node_idxs[i] = stoi(list[5+i])-1;
    }

    return element;
}

msh_data mesh_reader::read_msh(std::string file_path)
{
    std::ifstream stream;
    stream.open(file_path);

    msh_data mesh;

    //check if file opened
    if(!stream){std::cout << "File not found\n";}

    //read file
    std::string buffer;
    while(getline(stream,buffer))
    {
        //read version
        if(buffer == "$MeshFormat")
        {
            getline(stream,buffer);
            if(buffer != supported_version)
            {
                std::cout << "msh version " + buffer + " not supported\n";
                break;
            }
            else
            {
                std::cout << "msh version " + buffer + " ok\n";
                getline(stream,buffer);
            }
        }

        //read physicalnames
        if(buffer == "$PhysicalNames")
        {
            getline(stream,buffer);
            mesh.N_physicals = std::stoi(buffer);
            mesh.physical_domains.resize(mesh.N_physicals);

            for(int i = 0; i < mesh.N_physicals; i++)
            {
                getline(stream,buffer);
                mesh.physical_domains[i] = read_domain(buffer);
            }
            getline(stream,buffer);
        }

        //read nodes
        if(buffer == "$Nodes")
        {
            getline(stream,buffer);
            mesh.N_nodes = std::stoi(buffer);
            mesh.msh_nodes.resize(mesh.N_nodes);

            for(int i = 0; i < mesh.N_nodes; i++)
            {
                getline(stream,buffer);
                mesh.msh_nodes[i] = read_node_line(buffer);
            }
        }

        //read elements
        if(buffer == "$Elements")
        {
            getline(stream,buffer);
            mesh.N_elements = std::stoi(buffer);
            mesh.msh_elements.resize(mesh.N_elements);

            for(int i = 0; i < mesh.N_elements; i++)
            {
                getline(stream,buffer);
                mesh.msh_elements[i] = read_element_line(buffer);
            }
        }
    }

    return mesh;
}

msh_data mesh_reader::read_msh4(std::string file_path)
{
    std::ifstream stream;
    stream.open(file_path);

    msh_data mesh;

    //check if file opened
    if(!stream){std::cout << "File not found\n";}

    std::string buffer;
    std::vector<std::string> line;
    while(getline(stream,buffer))
    {
        //read version
        if(buffer == "$MeshFormat")
        {
            getline(stream,buffer);
            if(buffer != supported_version)
            {
                std::cout << "msh version " + buffer + " not supported\n";
                break;
            }
            else
            {
                std::cout << "msh version " + buffer + " ok\n";
                getline(stream,buffer);
            }
        }

        //read physicalnames
        if(buffer == "$PhysicalNames")
        {
            getline(stream,buffer);
            mesh.N_physicals = std::stoi(buffer);
            mesh.physical_domains.resize(mesh.N_physicals);

            for(int i = 0; i < mesh.N_physicals; i++)
            {
                getline(stream,buffer);
                mesh.physical_domains[i] = read_domain(buffer);
            }
            getline(stream,buffer);
        }

        if(buffer == "$Entities")
        {
            getline(stream,buffer);
            line = split(buffer," ");
            
            int i = 0;
            for(auto word : line)
            {
                const int n = stoi(word);
                mesh.msh_entities.dim_counts[i] += n;
                mesh.msh_entities.N_entities += n;
                i++;
            }

            mesh.msh_entities.entity_vector.resize(mesh.msh_entities.N_entities);
            int global_idx = 0;
            for(int dim = 0; dim <= 3; dim++)
            {
                for(i = 0; i < mesh.msh_entities.dim_counts[dim]; i++)
                {
                    getline(stream,buffer);
                    line = split(buffer," ");

                    if(dim == 0)
                    {
                        mesh.msh_entities.entity_vector[global_idx].idx = std::stoi(line[0]);
                        mesh.msh_entities.entity_vector[global_idx].dim = 0;
                    }
                    else if (dim == 1)
                    {
                        mesh.msh_entities.entity_vector[global_idx].idx = std::stoi(line[0]);
                        mesh.msh_entities.entity_vector[global_idx].dim = 1;
                        mesh.msh_entities.entity_vector[global_idx].phys_tag = std::stoi(line[8]);
                    }
                    else if (dim == 2)
                    {
                        mesh.msh_entities.entity_vector[global_idx].idx = std::stoi(line[0]);
                        mesh.msh_entities.entity_vector[global_idx].dim = 2;
                        mesh.msh_entities.entity_vector[global_idx].phys_tag = std::stoi(line[8]);
                    }
                    else if (dim == 3)
                    {
                        mesh.msh_entities.entity_vector[global_idx].idx = std::stoi(line[0]);
                        mesh.msh_entities.entity_vector[global_idx].dim = 3;
                        mesh.msh_entities.entity_vector[global_idx].phys_tag = std::stoi(line[8]);
                    }

                    global_idx++;
                }
            }

        }
    
        if(buffer == "$Nodes")
        {
            getline(stream,buffer);
            line = split(buffer," ");

            mesh.N_nodes = std::stoi(line[1]);
            mesh.msh_nodes.resize(mesh.N_nodes);

            int N_nodes_read = 0;
            std::vector<int> idx_vector;
            while(N_nodes_read < mesh.N_nodes)
            {
                getline(stream,buffer);
                line = split(buffer," ");

                int N_nodes_to_read = std::stoi(line[3]);

                idx_vector.clear();
                idx_vector.reserve(N_nodes_to_read);

                for(int i = 0; i < N_nodes_to_read; i++)
                {
                    getline(stream,buffer);
                    line = split(buffer," ");

                    idx_vector.push_back(std::stoi(line[0])-1);
                }

                for(auto idx : idx_vector)
                {
                    getline(stream,buffer);
                    line = split(buffer," ");

                    mesh.msh_nodes[idx].idx = idx;
                    mesh.msh_nodes[idx].x = std::stod(line[0]);
                    mesh.msh_nodes[idx].y = std::stod(line[1]);
                    mesh.msh_nodes[idx].z = std::stod(line[2]);
                }

                N_nodes_read += N_nodes_to_read;
            }
        }

        if(buffer == "$Elements")
        {
            getline(stream,buffer);
            line = split(buffer," ");

            mesh.N_elements = std::stoi(line[1]);
            mesh.msh_elements.resize(mesh.N_elements);

            int N_elements_read = 0;
            std::vector<int> idx_vector;
            idx_vector.reserve(4);

            while(N_elements_read < mesh.N_elements)
            {
                getline(stream,buffer);
                line = split(buffer," ");

                int N_elements_to_read = std::stoi(line[3]);
                int element_type = std::stoi(line[2]);

                int entity_tag = std::stoi(line[1])-1;
                int entity_dim = std::stoi(line[0]);

                int entity_idx = 0;
                for(int i = 0; i < entity_dim; i++)
                {
                    entity_idx += mesh.msh_entities.dim_counts[i];
                }
                entity_idx += entity_tag;

                for(int i = 0; i < N_elements_to_read; i++)
                {
                    getline(stream,buffer);
                    line = split(buffer," ");

                    idx_vector.clear();

                    int idx = std::stoi(line[0])-1;
                    int n_vertices = line.size()-2;

                    for(int k = 0; k < n_vertices; k++)
                    {
                        idx_vector.push_back(std::stoi(line[k+1])-1);
                    }
                    
                    mesh.msh_elements[idx].idx = idx;
                    mesh.msh_elements[idx].element_type = element_type;
                    mesh.msh_elements[idx].physical_idx = mesh.msh_entities.entity_vector[entity_idx].phys_tag;
                    mesh.msh_elements[idx].node_idxs = idx_vector;
                }

                N_elements_read += N_elements_to_read;
            }
        }
    }

    return mesh;
}