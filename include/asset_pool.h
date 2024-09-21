#pragma once
#include <citro2d.h>
#include <unordered_map>
#include <string>

struct ImageLoadingArgs {
    std::string url;
    std::unordered_map<std::string, C2D_Image> *map;
    C2D_Image** output;
    unsigned int width;
    unsigned int height;
};

class AssetPool {
public:
    AssetPool();
    ~AssetPool();

    void get_image(std::string url, C2D_Image **output, unsigned int width = 0, unsigned int height = 0);
private:
    std::unordered_map<std::string, C2D_Image> images;
    ImageLoadingArgs args;
};