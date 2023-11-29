#pragma once
#include <iostream>

template<typename T>
void check_if_allocated(T* p)
{
    if(p != nullptr)
    {
        std::cout << "Memory is already allocated, exiting...\n";
        exit(1);
    }
}