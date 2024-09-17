#include "curl_funcs.h"

#include <curl/curl.h>
#include <cstring>
#include <jansson.h>
#include <3ds.h>

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	char *ptr = (char*)realloc(mem->memory, mem->size + realsize + 1);
	if(!ptr) {
	/* out of memory! */
	fprintf(stderr, "not enough memory (realloc returned NULL)\n");
	return 0;
	}

	mem->memory = ptr;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->size += realsize;
	mem->memory[mem->size] = 0;

	return realsize;
}

void get_posts(void *postFetching)
{
    CURLcode statuscode;
	struct curl_slist *slist1 = NULL;
	slist1 = curl_slist_append(slist1, "Accept: application/json");
  	slist1 = curl_slist_append(slist1, "Authorization: Bearer <TOKEN>");

    struct MemoryStruct chunk;
	chunk.memory = (char*)malloc(1);
	chunk.size = 0;

    CURL *hnd = curl_easy_init();
	if (hnd) {
		curl_easy_setopt(hnd, CURLOPT_URL, "https://public.api.bsky.app/xrpc/app.bsky.feed.getFeed?feed=at://did:plc:z72i7hdynmk6r22z27h6tvur/app.bsky.feed.generator/whats-hot");
		curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
		curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&chunk);

		statuscode = curl_easy_perform(hnd);
	} else {
		fprintf(stderr, "Could not create cURL handle.");
	}

    if (statuscode == CURLE_OK) {
		json_error_t error;
		json_t *root = json_loads(chunk.memory, 0, &error);
		if (!root) {
			fprintf(stderr, "Error parsing string at line %d: %s\n", error.line, error.text);
		} else {
			PostFetching *pf = static_cast<PostFetching*>(postFetching);
			pf->cursor = json_string_value(json_object_get(root, "cursor"));

			json_t *posts_obj = json_object_get(root, "feed");
			for (size_t i = 0; i < json_array_size(posts_obj); i++) {
				json_t *obj = json_array_get(posts_obj, i);
				json_t *post_obj = json_object_get(obj, "post");

				json_t *post_author = json_object_get(post_obj, "author");
				json_t *post_record = json_object_get(post_obj, "record");

				svcSignalEvent(pf->eventHandle);

				static_cast<std::vector<Post>*>(pf->posts)->emplace_back(
					json_string_value(json_object_get(post_record, "text")),
					json_string_value(json_object_get(post_author, "handle")),
					json_string_value(json_object_get(post_author, "displayName")),
					0.5f
				);
			}
		}

		json_decref(root);
	} else {
        fprintf(stderr, "HTTP request returned %d: %s", statuscode, curl_easy_strerror(statuscode));
    }

	curl_easy_cleanup(hnd);
	free(chunk.memory);
	hnd = NULL;
	curl_slist_free_all(slist1);
	slist1 = NULL;
}

void threading_test(void *arg) {
	int i = 0;
	while (i < 5)
	{
		printf("thread%d says %d\n", (int)arg, i++);
		svcSleepThread(1000000ULL * (u32)arg);
	}
}