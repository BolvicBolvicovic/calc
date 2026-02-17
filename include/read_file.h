#ifndef READ_FILE_H
#define READ_FILE_H

static char*
read_file(arena_t* arena, const char* path)
{
	FILE*	file = fopen(path, "rb");
	
	if (!file)
	{
		fprintf(stderr, "Could not open file \"%s\".\n", path);
		exit(74);
	}

	fseek(file, 0L, SEEK_END);
	u64	file_size = ftell(file);
	rewind(file);
	
	char*	buffer		= ARENA_PUSH_ARRAY(arena, char, file_size + 1);
	u64	bytes_read	= fread(buffer, sizeof(char), file_size, file);
	buffer[bytes_read]	= '\0';
	
	if (bytes_read < file_size)
	{
		fprintf(stderr, "Could not read file \"%s\".\n", path);
		exit(74);
	}

	fclose(file);
	return buffer;
}

#endif
