#pragma once

#include "widgets/post.h"
#include <vector>
#include <string>
#include <optional>
#include "stb_image.h"

struct FixedMemory {
	void *memory;
	size_t byte_size;
};

void get_posts(std::string at_uri, std::string cursor, C2D_TextBuf textBuf, std::vector<Post> *posts, std::string *out_cursor);
std::optional<C2D_Image> get_image_from_url(const char* url, unsigned int width = 0, unsigned int height = 0);