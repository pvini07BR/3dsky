#include "widgets/post.h"
#include "c2d/text.h"
#include "defines.h"
#include "curl_funcs.h"
#include <cmath>
#include <cstdio>

#define PFP_RES 32

const float display_name_scale_factor = 1.5f;
const float handle_scale_factor = 0.8f;

std::string wrap_text(C2D_TextBuf textBuf, std::string text, float text_scale)
{
    std::string wrapped_text = "";
    C2D_Text tempText;
    for (size_t i = 0; i < text.size(); i++) {
        wrapped_text += text[i];

        C2D_TextParse(&tempText, textBuf, wrapped_text.c_str());
        C2D_TextOptimize(&tempText);

        float t_width;
        C2D_TextGetDimensions(&tempText, text_scale, text_scale, &t_width, NULL);
        if ((t_width + PFP_RES + 10) > BOTTOM_SCREEN_WIDTH) {
            int n = wrapped_text.rfind(' ', i);
            if (n != -1) {
                wrapped_text[n] = '\n';
            } else {
                wrapped_text.insert(wrapped_text.begin() + (i-1), '\n');
            }
        }

        C2D_TextBufClear(textBuf);
    }

    return wrapped_text;
}

Post::Post(C2D_TextBuf textBuf, const char *text, const char *handle, const char *display_name, const char *pfp_url, float text_scale) {
    // NOTE: The text buffer will be used here only for getting the proper post height.
    this->visible = false;
    this->text = text;
    this->author_handle = handle;
    this->author_display_name = display_name;
    this->pfp = std::nullopt;
    this->pfp_url = pfp_url;

    // Replace "avatar" in the URL to avatar_thumbnail so it will only get a lower resolution version of the pfp
    std::string replace_word = "avatar";
    size_t pos = this->pfp_url.find("avatar");
    if (pos != std::string::npos) {
        this->pfp_url.replace(pos, replace_word.size(), "avatar_thumbnail");
    }

    while (this->text.size() > MAX_POST_CHARACTERS) {
        this->text.erase(this->text.size()-1);
    }

    this->text_scale = text_scale;
    this->height = 0.0f;
    this->wrapped_text = wrap_text(textBuf, text, text_scale);

    C2D_TextParse(&this->c2d_text, textBuf, wrapped_text.c_str());
    C2D_TextOptimize(&this->c2d_text);

    C2D_TextParse(&this->c2d_handle_text, textBuf, this->author_handle.c_str());
    C2D_TextOptimize(&this->c2d_handle_text);

    C2D_TextParse(&this->c2d_author_display_text, textBuf, this->author_display_name.c_str());
    C2D_TextOptimize(&this->c2d_author_display_text);

    float text_height;
    C2D_TextGetDimensions(&this->c2d_text, this->text_scale, this->text_scale, nullptr, &text_height);
    C2D_TextGetDimensions(&this->c2d_author_display_text, this->text_scale*display_name_scale_factor, this->text_scale*display_name_scale_factor, &this->author_display_width, &this->author_display_height);

    C2D_TextGetDimensions(&this->c2d_handle_text, this->text_scale * handle_scale_factor, this->text_scale * handle_scale_factor, nullptr, &this->author_handle_height);

    this->height = text_height + author_display_height + 5.0f;

    C2D_TextBufClear(textBuf);
}

Post::~Post() {}

void Post::draw(float x, float y, C2D_TextBuf textBuf) {
    u32 lineColor = C2D_Color32(47, 64, 81, 255);

    //C2D_DrawText(&c2d_text, C2D_WithColor | C2D_WordWrap, x, y, 0.0, this->text_scale, this->text_scale, C2D_Color32(255, 255, 255, 255), (float)BOTTOM_SCREEN_WIDTH);
    if (this->visible) {
        C2D_TextBufClear(textBuf);

        C2D_TextParse(&this->c2d_text, textBuf, wrapped_text.c_str());
        C2D_TextOptimize(&this->c2d_text);

        C2D_TextParse(&this->c2d_handle_text, textBuf, this->author_handle.c_str());
        C2D_TextOptimize(&this->c2d_handle_text);

        C2D_TextParse(&this->c2d_author_display_text, textBuf, this->author_display_name.c_str());
        C2D_TextOptimize(&this->c2d_author_display_text);

        C2D_DrawText(&this->c2d_author_display_text, C2D_WithColor, x + (PFP_RES + 10.0f), y, 0.0, this->text_scale * display_name_scale_factor, this->text_scale * display_name_scale_factor, C2D_Color32(255, 255, 255, 255));
        C2D_DrawText(&this->c2d_handle_text, C2D_WithColor, x + (PFP_RES + 10.0f) + 5.0f + this->author_display_width, y + (author_handle_height/2.0), 0.0, this->text_scale * handle_scale_factor, this->text_scale * handle_scale_factor, C2D_Color32(128, 128, 128, 255));
        C2D_DrawText(&c2d_text, C2D_WithColor, x + (PFP_RES + 10.0f), y + this->author_display_height, 0.0, this->text_scale, this->text_scale, C2D_Color32(255, 255, 255, 255));
    }
    C2D_DrawLine(x, y, lineColor, x + BOTTOM_SCREEN_WIDTH, y, lineColor, 1.0f, 0.0f);
    C2D_DrawLine(x, y+this->height, lineColor, x + BOTTOM_SCREEN_WIDTH, y+this->height, lineColor, 1.0f, 0.0f);

    if (this->pfp.has_value()) {
        C2D_DrawImageAt(this->pfp.value(), x + 5.0f, y + 6.0f, 0.0f);
    }
}

void Post::on_enter() {
    this->pfp = get_image_from_url(this->pfp_url.c_str(), PFP_RES, PFP_RES);
}

void Post::on_exit() {
    if (this->pfp.has_value()) {
        C3D_TexDelete(this->pfp.value().tex);
        delete this->pfp.value().tex;
        delete this->pfp.value().subtex;
        this->pfp.value().tex = nullptr;
        this->pfp.value().subtex = nullptr;
        this->pfp = std::nullopt;
    }
}