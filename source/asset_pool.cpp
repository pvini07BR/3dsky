#include "asset_pool.h"
#include "curl_funcs.h"
#include "defines.h"
#include <3ds.h>
#include <algorithm>

void imageLoadingThread(void *arg) {
    ImageLoadingArgs *data = (ImageLoadingArgs_t*)arg;
    if (data == nullptr) {
        printf("Data is nullptr. Aborting.");
        return;
    }

	while(data->runThread) {
        if (data->queue == nullptr) {
            printf("Queue pointer is null. Aborting!\n");
            continue;
        }

        for (auto it = data->queue->begin(); it != data->queue->end(); ) {
            std::optional<C2D_Image> img = get_image_from_url(it->first, it->second.width, it->second.height);
            if (img.has_value()){
                (*data->images_ptr)[it->first] = std::move(img.value());
                (*it->second.output) = &(*data->images_ptr)[it->first];
                it = data->queue->erase(it);
            } else {
                ++it;
            }
        }
	}
}

AssetPool::AssetPool() {
    this->images = std::unordered_map<const char*, C2D_Image>();
    this->loading_queue = std::unordered_map<const char*, ImageQueueEntry_t>();

    this->threadArgs.runThread = true;
    this->threadArgs.queue = &this->loading_queue;
    this->threadArgs.images_ptr = &this->images;

    s32 prio = 0;
	svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
	this->imageLoadThreadHandle = threadCreate(imageLoadingThread, (void*)&this->threadArgs, (8 * 1024), prio+1, -2, true);
    if (this->imageLoadThreadHandle == nullptr) { printf("image loading thread handle is null.\n"); }
}

AssetPool::~AssetPool() {
    for (auto& it: this->images) {
        delete ((C2D_Image)it.second).tex;
        delete ((C2D_Image)it.second).subtex;
    }
    this->images.clear();

    this->threadArgs.runThread = false;
    threadJoin(this->imageLoadThreadHandle, U64_MAX);
}

void AssetPool::get_image(const char* url, C2D_Image **output, unsigned int width, unsigned int height) {
    if (this->images.find(url) != this->images.end()) {
        (*output) = &this->images[url];
    } else {
        if (this->loading_queue.find(url) == this->loading_queue.end()) {
            ImageQueueEntry_t entry;
            entry.width = width;
            entry.height = height;
            entry.output = output;
            loading_queue[url] = entry;
        }
    }
}

void AssetPool::remove_from_queue(const char* url) {
    if (this->loading_queue.find(url) != this->loading_queue.end()) {
        this->loading_queue.erase(url);
    }
}