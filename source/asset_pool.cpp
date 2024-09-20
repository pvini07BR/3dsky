#include "asset_pool.h"
#include "curl_funcs.h"

AssetPool::AssetPool() {
    this->images = std::unordered_map<std::string, C2D_Image>();
}

AssetPool::~AssetPool() {
    for (auto& it: this->images) {
        delete ((C2D_Image)it.second).tex;
        delete ((C2D_Image)it.second).subtex;
    }
    this->images.clear();
}

C2D_Image* AssetPool::get_image(std::string url, unsigned int width, unsigned int height) {
    if (this->images.find(url) != this->images.end()) {
        return &this->images[url];
    } else {
        std::optional<C2D_Image> img = get_image_from_url(url, width, height);
        if (img.has_value()) {
            this->images[url] = img.value();
            return &this->images[url];
        } else {
            return nullptr;
        }
    }
}