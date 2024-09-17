#include "widgets/feed.h"
#include <citro2d.h>
#include "defines.h"

Feed::Feed(float text_scale) {
    this->posts = std::vector<Post>();
    this->posts.reserve(50);
    this->text_scale = text_scale;
}

Feed::~Feed() { this->posts.clear(); }

void Feed::draw(float h_displacement, float scrollY) {
    float add = 0.0f;
    for (size_t i = 0; i < this->posts.size(); i++) {
        float screen_origin = -scrollY;
        float screen_end = -(scrollY - (float)SCREEN_HEIGHT);

        float Yorigin = add;
        float Yend = Yorigin + this->posts[i].height;

        float realYorigin = Yorigin + scrollY;
        float realYend = realYorigin + this->posts[i].height;

        bool screen_origin_inside_post = (screen_origin >= Yorigin) && (screen_origin <= Yend);
        bool screen_end_inside_post = (screen_end >= Yorigin) && (screen_end <= Yend);

        bool post_origin_inside_screen = realYorigin >= 0.0f && realYorigin < (float)SCREEN_HEIGHT;
        bool post_end_inside_screen = realYend >= 0.0f && realYend < (float)SCREEN_HEIGHT;

        if (post_origin_inside_screen || post_end_inside_screen || screen_origin_inside_post || screen_end_inside_post) {
            this->posts[i].draw(h_displacement, realYorigin);
        }

        add += this->posts[i].height;
    }
}

void Feed::remove_post(size_t index) {
    this->posts.erase(this->posts.begin() + index);
}

float Feed::get_total_height() {
    float add = 0.0f;
    for (size_t i = 0; i < this->posts.size(); i++) {
        add += this->posts[i].height;
    }
    return add;
}
