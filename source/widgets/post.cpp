#include "widgets/post.h"
#include "defines.h"

const u32 lineColor = C2D_Color32(47, 64, 81, 255);

Post::Post(std::string text, float text_scale) {
    this->text = text;
    while (this->text.size() > MAX_POST_CHARACTERS) {
        this->text.erase(this->text.size()-1);
    }

    this->text_scale = text_scale;
    this->height = 128.0f;
    this->textBuf = C2D_TextBufNew(MAX_POST_CHARACTERS);

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
            }
        }

        C2D_TextBufClear(this->textBuf);
    }

    C2D_TextParse(&this->c2d_text, this->textBuf, wrapped_text.c_str());
    C2D_TextOptimize(&this->c2d_text);

    C2D_TextGetDimensions(&this->c2d_text, this->text_scale, this->text_scale, NULL, &this->height);
}

Post::~Post() {
    C2D_TextBufDelete(textBuf);
}

void Post::draw(float x, float y) {
    // C2D_DrawRectSolid(
    //     x, y, 0.0,
    //     BOTTOM_SCREEN_WIDTH,
    //     this->height / 2.0,
    //     C2D_Color32(128, 0, 0, 255)
    // );

    C2D_DrawText(&c2d_text, C2D_WithColor, x, y, 0.0, this->text_scale, this->text_scale, C2D_Color32(255, 255, 255, 255));

    C2D_DrawLine(x, y, lineColor, x + BOTTOM_SCREEN_WIDTH, y, lineColor, 1.0f, 0.0f);
    C2D_DrawLine(x, y+this->height, lineColor, x + BOTTOM_SCREEN_WIDTH, y+this->height, lineColor, 1.0f, 0.0f);
}

float Post::get_height() {
    return this->height;
}