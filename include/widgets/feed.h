#pragma once

#include <string>
#include <vector>

#include "widgets/post.h"
#include "asset_pool.h"

class Feed {
public:
    Feed(float text_scale);
    ~Feed();

    Feed(const Feed&) = delete;
    Feed& operator=(const Feed&) = delete;
    Feed(Feed&&) noexcept = default;
    Feed& operator=(Feed&&) noexcept = default;
    
    void draw(float h_displacement, float scrollY);
    void reserve_more(size_t amount);
    void remove_post(size_t index);
    float get_total_height();

    std::vector<Post> posts;
    C2D_TextBuf textBuf;
private:
    float text_scale;
};
