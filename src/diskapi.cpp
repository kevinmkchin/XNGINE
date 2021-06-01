/**

    FILE operations to disk

*/

internal void FILE_free_file_binary(binary_file_handle_t& binary_file_to_free)
{
    free(binary_file_to_free.memory);
    binary_file_to_free.memory = NULL;
    binary_file_to_free.size = 0;
}

/** Allocates memory, stores the binary file data in memory, makes binary_file_handle_t.memory
    point to it. Pass along a binary_file_handle_t to receive the pointer to the file data in
    memory and the size in bytes. */
internal void FILE_read_file_binary(binary_file_handle_t& mem_to_read_to, const char* file_path)
{
    if(mem_to_read_to.memory)
    {
        con_printf("WARNING: Binary File Handle already points to allocated memory. Freeing memory first...\n");
        FILE_free_file_binary(mem_to_read_to);
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
        con_printf("Failed to read %s! File doesn't exist.\n", file_path);
        return;
    }
}

/** Returns the string content of a file as an std::string */
internal std::string FILE_read_file_string(const char* file_path)
{
    std::string string_content;

    std::ifstream file_stream(file_path, std::ios::in);
    if (file_stream.is_open() == false)
    {
        con_printf("Failed to read %s! File doesn't exist.\n", file_path);
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

internal void FILE_free_image(bitmap_handle_t& image_handle)
{
    FILE_free_file_binary(image_handle);
    image_handle.width = 0;
    image_handle.height = 0;
    image_handle.bit_depth = 0;
}

/** Allocates memory, loads an image file as an UNSIGNED BYTE bitmap, makes bitmap_handle_t.memory
    point to it. Pass along a bitmap_handle_t to receive the pointer to the bitmap in memory and
    bitmap information. */
internal void FILE_read_image(bitmap_handle_t& image_handle, const char* image_file_path)
{
    if(image_handle.memory)
    {
        con_printf("WARNING: Binary File Handle already points to allocated memory. Freeing memory first...\n");
        FILE_free_image(image_handle);
    }

    image_handle.memory = stbi_load(image_file_path, (int*)&image_handle.width, (int*)&image_handle.height, (int*)&image_handle.bit_depth, 0);
    if(image_handle.memory)
    {
        image_handle.size = image_handle.width * image_handle.height * image_handle.bit_depth;
    }
    else
    {
        con_printf("Failed to find image file at: %s\n", image_file_path);
        image_handle.width = 0;
        image_handle.height = 0;
        image_handle.bit_depth = 0;
        return;
    }
}