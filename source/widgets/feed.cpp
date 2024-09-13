#include "widgets/feed.h"
#include <citro2d.h>
#include "defines.h"

#include <sstream>

Feed::Feed(float text_scale) {
    this->posts = std::vector<Post>();
    this->text_scale = text_scale;
    
    posts.reserve(10);

    for (int i = 0; i < 10; i++) {
        std::ostringstream oss;
        oss << "This\nis\npost\nnumber\n" << i << "\n.";
        this->posts.emplace_back(oss.str(), text_scale);
    }
}

Feed::~Feed() { this->posts.clear(); }

void Feed::draw(float h_displacement, float scrollY) {
    for (size_t i = 0; i < this->posts.size(); i++) {
        float height = this->posts[i].get_height();
        float y = i * height;
        float final_y = scrollY + y;
        float end_final_y = final_y + height;

        if ((final_y >= 0.0 && final_y < SCREEN_HEIGHT) || (end_final_y >= 0.0 && end_final_y < SCREEN_HEIGHT)) {
            this->posts[i].draw(h_displacement, final_y);
        }
    }
}

float Feed::get_total_height() {
    float add = 0.0f;
    for (size_t i = 0; i < this->posts.size(); i++) {
        add += this->posts[i].get_height();
    }
    return add;
}