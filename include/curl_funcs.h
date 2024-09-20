#pragma once

#include "widgets/post.h"
#include <vector>
#include <string>
#include <optional>

struct MemoryStruct {
	char *memory;
	size_t size;
};

void get_posts(std::string at_uri, std::string cursor, C2D_TextBuf textBuf, std::vector<Post> *posts, std::string *out_cursor);
std::optional<C2D_Image> get_image_from_url(std::string url, unsigned int width = 0, unsigned int height = 0);