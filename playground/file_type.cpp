#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;
using std::clog;
using std::endl;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        clog << "Usage: " << argv[0] << " path" << endl;
        exit(EXIT_FAILURE);
    }

    if (fs::is_regular_file(argv[1]))
        clog << "is regular file" << endl;
    else
        clog << "not regular file" << endl;

    if (fs::is_block_file(argv[1]))
        clog << "is block file" << endl;
    else
        clog << "not block file" << endl;
    
    if (fs::is_character_file(argv[1]))
        clog << "is character file" << endl;
    else
        clog << "not character file" << endl;
    
    if (fs::is_other(argv[1]))
        clog << "is other" << endl;
    else
        clog << "not other" << endl;
}