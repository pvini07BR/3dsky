#pragma once

#include <string>
#include <citro2d.h>

#define MAX_POST_CHARACTERS 300

class Post {
public:
    Post(std::string text, std::string handle, std::string display_name, float text_scale);
    ~Post();

    Post(Post&& other);
    Post& operator=(Post&& other) noexcept {
    if (this != &other) {
        C2D_TextBufDelete(this->textBuf);
        text = std::move(other.text);
        text_scale = other.text_scale;
        height = other.height;
        c2d_text = other.c2d_text;
        textBuf = other.textBuf;
        author_handle = other.author_handle;
        other.textBuf = nullptr;
    }
    return *this;
}

    Post(const Post&) = delete;
    Post& operator=(const Post&) = delete;

    void draw(float x, float y);

    std::string text;
    std::string author_display_name;
    std::string author_handle;

    float author_display_width;
    float author_display_height;

    float author_handle_height;

    float height;
    float text_scale;
private:
    C2D_Text c2d_text;
    C2D_Text c2d_handle_text;
    C2D_Text c2d_author_display_text;
    C2D_TextBuf textBuf;

    std::string wrap_text(std::string text);
};
