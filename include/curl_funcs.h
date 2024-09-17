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
	C2D_TextBuf textBuf;
	std::vector<Post> *posts;
	LightEvent eventHandle;

	// returned values
	std::string cursor;
};

void get_posts(void *posts);
void threading_test(void *arg);
