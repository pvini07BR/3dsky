#pragma once

#include <string>
#include <citro2d.h>

#define MAX_POST_CHARACTERS 300

class Post {
public:
    Post(std::string text, float text_scale);
    ~Post();

    void draw(float x, float y);
    float get_height();
private:
    std::string text;
    float text_scale;
    float height;
    C2D_Text c2d_text;
    C2D_TextBuf textBuf;
};