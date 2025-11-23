#include "cpptoml.h"

#include <iostream>
#include <cassert>
#include <chrono>
#include <cstdlib>

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " filename [timeout]" << std::endl;
        return 1;
    }

    // Default timeout is 30 seconds
    std::chrono::milliseconds timeout{30000};

    // Check if timeout is provided as a command line argument
    if (argc > 2)
    {
        try
        {
            timeout = std::chrono::milliseconds{std::stoi(argv[2])};
        }
        catch (const std::invalid_argument& e)
        {
            std::cerr << "Invalid timeout value: " << argv[2] << std::endl;
            return 1;
        }
        catch (const std::out_of_range& e)
        {
            std::cerr << "Timeout value out of range: " << argv[2] << std::endl;
            return 1;
        }
    }

    try
    {
        std::shared_ptr<cpptoml::table> g = cpptoml::parse_file(argv[1], timeout);
        std::cout << (*g) << std::endl;
    }
    catch (const cpptoml::parse_exception& e)
    {
        std::cerr << "Failed to parse " << argv[1] << ": " << e.what() << std::endl;
        return 1;
    }
    catch (const cpptoml::stream_read_timeout& e)
    {
        std::cerr << "Stream read timeout: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
