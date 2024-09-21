#pragma once

#include <memory>
#include <string>
#include <citro2d.h>
#include "asset_pool.h"

#define MAX_POST_CHARACTERS 300

class Post {
public:
    Post(C2D_TextBuf textBuf, const char *text, const char *handle, const char *display_name, const char *pfp_url, float text_scale);
    ~Post();

    void draw(float x, float y, C2D_TextBuf textBuf);
    void on_enter(AssetPool *asset_pool);
    void on_exit(AssetPool *asset_pool);

    std::string text;
    std::string wrapped_text;
    std::string author_display_name;
    std::string author_handle;
    std::string pfp_url;

    C2D_Image* pfp;

    float author_display_width;
    float author_display_height;

    float author_handle_height;

    float height;
    float text_scale;
    bool visible;
private:
    C2D_Text c2d_text;
    C2D_Text c2d_handle_text;
    C2D_Text c2d_author_display_text;
};
