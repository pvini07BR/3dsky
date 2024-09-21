#include "asset_pool.h"
#include "curl_funcs.h"
#include "defines.h"
#include <3ds.h>

Thread thread;
LightEvent eventHandle;

/*
volatile bool runningThread = true;

void threadedLoading(void *a) {
    while (runningThread) {
        LightEvent_Wait(&eventHandle);

        printf("Event called!\n");

        ImageLoadingArgs* args = (ImageLoadingArgs*)(a);
        std::optional<C2D_Image> img = get_image_from_url(args->url, args->width, args->height);
        if (img.has_value()) {
            printf("%s\n", args->url.c_str());
        }
    }
}
*/

AssetPool::AssetPool() {
    this->images = std::unordered_map<std::string, C2D_Image>();

    //LightEvent_Init(&eventHandle, ResetType::RESET_ONESHOT);
    //s32 prio = 0;
	//svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
    //thread = threadCreate(threadedLoading, (void*)&this->args, (1024 * 1024), prio-2, -2, true);
    //if (thread == nullptr) { printf("The thread handle is null.\n"); }
}

AssetPool::~AssetPool() {
    for (auto& it: this->images) {
        delete ((C2D_Image)it.second).tex;
        delete ((C2D_Image)it.second).subtex;
    }
    this->images.clear();

    //runningThread = false;
    //threadJoin(thread, U64_MAX);
    //LightEvent_Clear(&eventHandle);
}

void AssetPool::get_image(std::string url, C2D_Image **output, unsigned int width, unsigned int height) {
    if (this->images.find(url) != this->images.end()) {
        (*output) = &this->images[url];
    } else {
        /*
        this->args.url = url;
        this->args.width = width;
        this->args.height = height;
        this->args.output = output;
        this->args.map = &this->images;

        LightEvent_Signal(&eventHandle);
        */
    }
}