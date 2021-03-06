/** Returns the string content of a file.
 */
INTERNAL std::string file_read_file_string(const char* file_path)
{
    std::string string_content;

    std::ifstream file_stream(file_path, std::ios::in);
    if (file_stream.is_open() == false)
    {
        printf("Failed to read %s! File doesn't exist.\n", file_path);
    }

    std::string line = "";
    while (file_stream.eof() == false)
    {
        std::getline(file_stream, line);
        string_content.append(line + "\n");
    }

    file_stream.close();

    return string_content;
}

/** Deallocates the memory pointed to by the given BinaryFileHandle.
 */
INTERNAL void file_free_file_binary(BinaryFileHandle& binary_file_to_free)
{
    free(binary_file_to_free.memory);
    binary_file_to_free.memory = NULL;
    binary_file_to_free.size = 0;
}

/** Allocates memory, stores the file data in binary, and returns a BinaryFileHandle
    containing a pointer its place in memory and the size in bytes.
 */
INTERNAL void file_read_file_binary(BinaryFileHandle& mem_to_read_to, const char* file_path)
{
    if(mem_to_read_to.memory)
    {
        printf("WARNING: Binary File Handle already points to allocated memory. Freeing memory first...\n");
        file_free_file_binary(mem_to_read_to);
    }

    SDL_RWops* binary_file_rw = SDL_RWFromFile(file_path, "rb");
    if(binary_file_rw)
    {
        mem_to_read_to.size = SDL_RWsize(binary_file_rw); // total size in bytes
        mem_to_read_to.memory = malloc(mem_to_read_to.size);
        SDL_RWread(binary_file_rw, mem_to_read_to.memory, mem_to_read_to.size, 1);
        SDL_RWclose(binary_file_rw);
    }
    else
    {
        printf("Failed to read %s! File doesn't exist.\n", file_path);
    }
}