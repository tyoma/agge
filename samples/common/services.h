#pragma once

#include <cstddef>

struct stream
{
	virtual ~stream() { }
	virtual void read(void *buffer, std::size_t size) = 0;
};

struct services
{
	virtual stream *open_file(const char *path) = 0;
};
