#pragma once

#include "widgets/post.h"
#include <vector>
#include <string>

struct MemoryStruct {
	char *memory;
	size_t size;
};

struct PostFetching {
	// arguments
	std::vector<Post> *posts;
	Handle eventHandle;

	// returned values
	std::string cursor;
};

void get_posts(void *posts);
void threading_test(void *arg);
