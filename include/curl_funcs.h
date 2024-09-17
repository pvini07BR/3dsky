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
	std::string at_uri;
	C2D_TextBuf textBuf;
	std::vector<Post> *posts;
	LightEvent eventHandle;

	// this is both a input and output
	std::string cursor;
};

void get_posts(void *posts);
void threading_test(void *arg);
