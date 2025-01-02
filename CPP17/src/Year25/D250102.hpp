#include <iostream>
#include <filesystem>
#include <string>

namespace Y25M1
{
    namespace fs = std::filesystem;

    int fileSystemEx1()
    {

        try{
            std::filesystem::path currentPath = fs::current_path();
            std::cout << "Current path : " << currentPath << "\n";

            std::filesystem::path parentPath = currentPath.parent_path();
            std::cout << "Parent path : " << parentPath << "\n";

            std::string subdir = "subdir";
            std::filesystem subPath = currentPath / subdir;

            if(fs::exist(subPath) && fs::is_directory(subPath))
            {
                std::cout << "Subdirectory path : " << subPath << "\n";
            }
            else
            {
                std::cout << "Subdirectory doesn't exist : " << subPath << "\n";
            }

            // change working directory
            std::filesystem::current_path(parentPath);
            std::cout << "Changed to parent path : " << fs::current_path() << "\n";

            std::filesystem::curret_path(currentPath);
            std::cout << "Returned to original path : " << fs::current_path() << "\n";
        }
        catch(const std::filesystem::filesystem_error& e)
        {
            std::cerr << "Filesystem error : " << e.what() << "\n";
        }
        catch(const std::exception& e)
        {
            std::cerr << "Standard exception : " << e.what() << "\n";
        }
        return 0;
    }
}