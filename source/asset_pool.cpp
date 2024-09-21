#include "asset_pool.h"
#include "curl_funcs.h"
#include "defines.h"
#include <3ds.h>

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

void AssetPool::get_image(std::string url, C2D_Image **output, unsigned int width, unsigned int height) {
    if (this->images.find(url) != this->images.end()) {
        (*output) = &this->images[url];
    } else {
        
    }
}