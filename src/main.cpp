﻿#include <exception>
#include <iostream>
#include <ostream>

#include "ShamanEngine.h"

int main()
{
    
    SE::ShamanEngine shamanEngine{};

    try
    {
        shamanEngine.run();
    } catch (const std::exception& e)
    {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}