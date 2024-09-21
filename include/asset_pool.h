#pragma once
#include <citro2d.h>
#include <unordered_map>
#include <string>

typedef struct ImageQueueEntry_t
{
    unsigned int width;
    unsigned int height;
    C2D_Image **output;

} ImageQueueEntry;

typedef struct ImageLoadingArgs_t {
    bool runThread;
    std::unordered_map<const char*, ImageQueueEntry_t> *queue;
    std::unordered_map<const char*, C2D_Image> *images_ptr;

} ImageLoadingArgs;

class AssetPool {
public:
    AssetPool();
    ~AssetPool();

    void get_image(const char* url, C2D_Image **output, unsigned int width = 0, unsigned int height = 0);
    void remove_from_queue(const char* url);
private:
    std::unordered_map<const char*, C2D_Image> images;
    std::unordered_map<const char*, ImageQueueEntry_t> loading_queue;
    ImageLoadingArgs threadArgs;
    Thread imageLoadThreadHandle;
};