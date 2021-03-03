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

/** Allocates memory, stores the file data in binary, and returns a ReadBinaryFileResult
	containing a pointer its place in memory and the size in bytes.
 */
INTERNAL ReadBinaryFileResult file_read_file_binary(const char* file_path)
{
	ReadBinaryFileResult binary_result;

	SDL_RWops* binary_file_rw = SDL_RWFromFile(file_path, "rb");
    if(binary_file_rw)
    {
       	binary_result.size = SDL_RWsize(binary_file_rw); // total size in bytes
        binary_result.memory = malloc(binary_result.size);
        SDL_RWread(binary_file_rw, binary_result.memory, binary_result.size, 1);
        SDL_RWclose(binary_file_rw);
    }
    else
    {
    	printf("Failed to read %s! File doesn't exist.\n", file_path);
    }

    return binary_result;
}