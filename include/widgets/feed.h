#pragma once

#include <string>
#include <vector>

#include "widgets/post.h"

class Feed {
public:
    Feed(float text_scale);
    ~Feed();
    void draw(float h_displacement, float scrollY);
    float get_total_height();
private:
    std::vector<Post> posts;
    float text_scale;
};