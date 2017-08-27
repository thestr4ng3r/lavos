
#include "triangle_application.h"

#include <iostream>
#include <stdexcept>
#include <functional>

int main()
{
    TriangleApplication app;

    try
    {
        app.Run();
    }
    catch(const std::runtime_error &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}