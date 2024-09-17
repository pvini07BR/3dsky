#pragma once

#include "widgets/post.h"
#include <vector>

struct MemoryStruct {
	char *memory;
	size_t size;
};

struct PostFetching {
	std::vector<Post> *posts;
	Handle eventHandle;
};

void get_posts(void *posts);
void threading_test(void *arg);
