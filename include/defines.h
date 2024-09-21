#define TOP_SCREEN_WIDTH  400
#define BOTTOM_SCREEN_WIDTH 320

#define SCREEN_HEIGHT 240

#define SCREEN_TOP_BOTTOM_DIFF 40

#define STACKSIZE (4 * 1024)

typedef enum ThreadTaskType_t {
    LOAD_POSTS = 0,
    LOAD_IMAGE = 1
} ThreadTaskType;