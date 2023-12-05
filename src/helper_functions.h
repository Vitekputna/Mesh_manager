#pragma once
#include <iostream>
#include <algorithm>

template<typename T>
void check_if_allocated(T* p)
{
    if(p != nullptr)
    {
        std::cout << "Memory is already allocated, exiting...\n";
        exit(1);
    }
}

template<typename T>
bool contains(T* array, T value)
{
    auto it = std::find(std::begin(array),std::end(array),value);
    if(it != std::end(array)){return true;}
    else{return false;}
}

template<typename T>
bool contains(std::vector<T> array, T value)
{
    auto it = std::find(array.begin(),array.end(),value);
    if(it != array.end()){return true;}
    else{return false;}
}