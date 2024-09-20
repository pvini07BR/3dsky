#pragma once
#include <citro2d.h>
#include <unordered_map>
#include <string>

class AssetPool {
public:
    AssetPool();
    ~AssetPool();

    C2D_Image* get_image(std::string url, unsigned int width = 0, unsigned int height = 0);
private:
    std::unordered_map<std::string, C2D_Image> images;
};