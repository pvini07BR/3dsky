#include "widgets/post.h"
#include "c2d/text.h"
#include "defines.h"
#include <cmath>
#include <cstdio>

const float display_name_scale_factor = 1.5f;
const float handle_scale_factor = 0.8f;

std::string Post::wrap_text(std::string text)
{
    std::string wrapped_text = "";
    C2D_Text tempText;
    for (size_t i = 0; i < text.size(); i++) {
        wrapped_text += this->text[i];

        C2D_TextParse(&tempText, this->textBuf, wrapped_text.c_str());
        C2D_TextOptimize(&tempText);

        float t_width;
        C2D_TextGetDimensions(&tempText, text_scale, text_scale, &t_width, NULL);
        if (t_width > BOTTOM_SCREEN_WIDTH) {
            int n = wrapped_text.rfind(' ', i);
            if (n != -1) {
                wrapped_text[n] = '\n';
            } else {
                wrapped_text.insert(wrapped_text.begin() + (i-1), '\n');
            }
        }

        C2D_TextBufClear(this->textBuf);
    }

    return wrapped_text;
}

Post::Post(std::string text, std::string handle, std::string display_name, float text_scale) {
    this->text = text;
    this->author_handle = handle;
    this->author_display_name = display_name;

    while (this->text.size() > MAX_POST_CHARACTERS) {
        this->text.erase(this->text.size()-1);
    }

    this->text_scale = text_scale;
    this->height = 0.0f;
    this->textBuf = C2D_TextBufNew(MAX_POST_CHARACTERS+1);

    C2D_TextParse(&this->c2d_text, this->textBuf, wrap_text(text).c_str());
    C2D_TextOptimize(&this->c2d_text);

    C2D_TextParse(&this->c2d_handle_text, this->textBuf, this->author_handle.c_str());
    C2D_TextOptimize(&this->c2d_handle_text);

    C2D_TextParse(&this->c2d_author_display_text, this->textBuf, this->author_display_name.c_str());
    C2D_TextOptimize(&this->c2d_author_display_text);

    float text_height;
    C2D_TextGetDimensions(&this->c2d_text, this->text_scale, this->text_scale, nullptr, &text_height);
    C2D_TextGetDimensions(&this->c2d_author_display_text, this->text_scale*display_name_scale_factor, this->text_scale*display_name_scale_factor, &this->author_display_width, &this->author_display_height);

    C2D_TextGetDimensions(&this->c2d_handle_text, this->text_scale * handle_scale_factor, this->text_scale * handle_scale_factor, nullptr, &this->author_handle_height);

    this->height = text_height + author_display_height;
}

Post::~Post() {
    printf("Post destructed.\n");
    C2D_TextBufDelete(textBuf);
}

void Post::draw(float x, float y) {
    u32 lineColor = C2D_Color32(47, 64, 81, 255);

    //C2D_DrawText(&c2d_text, C2D_WithColor | C2D_WordWrap, x, y, 0.0, this->text_scale, this->text_scale, C2D_Color32(255, 255, 255, 255), (float)BOTTOM_SCREEN_WIDTH);
    C2D_DrawText(&this->c2d_author_display_text, C2D_WithColor, x, y, 0.0, this->text_scale * display_name_scale_factor, this->text_scale * display_name_scale_factor, C2D_Color32(255, 255, 255, 255));
    C2D_DrawText(&this->c2d_handle_text, C2D_WithColor, x + 5.0f + this->author_display_width, y + (author_handle_height/2.0), 0.0, this->text_scale * handle_scale_factor, this->text_scale * handle_scale_factor, C2D_Color32(128, 128, 128, 255));
    C2D_DrawText(&c2d_text, C2D_WithColor, x, y + this->author_display_height, 0.0, this->text_scale, this->text_scale, C2D_Color32(255, 255, 255, 255));
    
    C2D_DrawLine(x, y, lineColor, x + BOTTOM_SCREEN_WIDTH, y, lineColor, 1.0f, 0.0f);
    C2D_DrawLine(x, y+this->height, lineColor, x + BOTTOM_SCREEN_WIDTH, y+this->height, lineColor, 1.0f, 0.0f);
}

/*
Post::Post(Post&& other) noexcept
    : text(std::move(other.text)),
      text_scale(other.text_scale),
      height(other.height),
      c2d_text(other.c2d_text),
      author_handle(other.author_handle),
      author_display_name(other.author_display_name),
      textBuf(other.textBuf) {
    other.textBuf = nullptr;
}
*/