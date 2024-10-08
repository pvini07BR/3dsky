#include "curl_funcs.h"

#include <curl/curl.h>
#include <cstring>
#include <iostream>
#include <chrono>
#include <jansson.h>
#include <3ds.h>
//#define STBI_FAILURE_USERMSG
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize2.h>

static u32 next_pow2(u32 i)
{
	--i;
	i |= i >> 1;
	i |= i >> 2;
	i |= i >> 4;
	i |= i >> 8;
	i |= i >> 16;
	++i;

	return i;
}

size_t write_data_placeholder(void *buffer, size_t size, size_t nmemb, void *userp)
{
   return size * nmemb;
}

size_t fixed_memory_write_callback(void *buffer, size_t size, size_t nmemb, void *userp) {
	size_t realsize = size * nmemb;
	struct FixedMemory *mem = (struct FixedMemory *)userp;

	memcpy(mem->memory + mem->byte_size, buffer, realsize);
	mem->byte_size += realsize;

	return realsize;
}

curl_off_t get_download_size(const char* url)
{
  CURL *curl = curl_easy_init();
  if (curl) {
	CURLcode res;
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data_placeholder);
	res = curl_easy_perform(curl);
	if (!res) {
		curl_off_t cl;
      	res = curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &cl);
		return cl;
	} else {
		return -1;
	}
  } else {
	return -1;
  }
}

void get_posts(std::string at_uri, std::string cursor, C2D_TextBuf textBuf, std::vector<Post> *posts, std::string *out_cursor)
{
	std::string url = "https://public.api.bsky.app/xrpc/app.bsky.feed.getFeed\?feed=" + at_uri + "&limit=50";
	if (!cursor.empty()) {
		url += "&cursor=" + cursor;
	}

	size_t download_size = get_download_size(url.c_str());

    CURLcode statuscode;
	struct curl_slist *slist1 = NULL;
	slist1 = curl_slist_append(slist1, "Accept: application/json");
  	slist1 = curl_slist_append(slist1, "Authorization: Bearer <TOKEN>");

    struct FixedMemory mem;
	mem.memory = malloc(download_size);
	mem.byte_size = 0;

    CURL *hnd = curl_easy_init();
	if (hnd) {
		curl_easy_setopt(hnd, CURLOPT_URL, url.c_str());
		curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, slist1);
		curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, fixed_memory_write_callback);
		curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void*)&mem);

		statuscode = curl_easy_perform(hnd);
	} else {
		fprintf(stderr, "Could not create cURL handle.");
	}

    if (statuscode == CURLE_OK) {
		json_error_t error;
		json_t *root = json_loads((const char*)mem.memory, 0, &error);
		if (!root) {
			fprintf(stderr, "Error parsing string at line %d: %s\n", error.line, error.text);
		} else {
			*out_cursor = json_string_value(json_object_get(root, "cursor"));

			json_t *posts_obj = json_object_get(root, "feed");
			for (size_t i = 0; i < json_array_size(posts_obj); i++) {
				json_t *obj = json_array_get(posts_obj, i);
				json_t *post_obj = json_object_get(obj, "post");

				json_t *post_author = json_object_get(post_obj, "author");
				json_t *post_record = json_object_get(post_obj, "record");

				posts->emplace_back(
					textBuf,
					json_string_value(json_object_get(post_record, "text")),
					json_string_value(json_object_get(post_author, "handle")),
					json_string_value(json_object_get(post_author, "displayName")),
					json_string_value(json_object_get(post_author, "avatar")),
					0.5f
				);
			}
		}

		json_decref(root);
	} else {
        fprintf(stderr, "HTTP request returned %d: %s", statuscode, curl_easy_strerror(statuscode));
    }
	
	curl_easy_cleanup(hnd);
	free(mem.memory);
	hnd = NULL;
	curl_slist_free_all(slist1);
	slist1 = NULL;
}

std::optional<C2D_Image> get_image_from_url(const char* url, unsigned int width, unsigned int height) {
	auto start = std::chrono::high_resolution_clock::now();
	size_t download_size = get_download_size(url);

	if (url == nullptr || strlen(url) == 0) {
		printf("URL is nullptr or empty. Aborting!\n");
		return std::nullopt;
	}

	CURL *hnd = curl_easy_init();
	if (hnd == nullptr) {
		printf("Failed to create cURL Handle. Aborting!\n");
		return std::nullopt;
	}

	struct FixedMemory imgmem;
	imgmem.memory = (stbi_uc*)malloc(download_size);
	imgmem.byte_size = 0;

	curl_easy_setopt(hnd, CURLOPT_URL, url);
	curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(hnd, CURLOPT_WRITEFUNCTION, fixed_memory_write_callback);
	curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)&imgmem);
	CURLcode ret = curl_easy_perform(hnd);

	if (ret == CURLE_OK) {
		int img_width, img_height, n_channels;
		unsigned char *img = stbi_load_from_memory((stbi_uc*)imgmem.memory, imgmem.byte_size, &img_width, &img_height, &n_channels, 4);
		if (img == nullptr) {
			fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
			free(imgmem.memory);
			curl_easy_cleanup(hnd);
			hnd = NULL;
			return std::nullopt;
		} else {
			if (width > 0 && height > 0) {
				img = stbir_resize_uint8_srgb(img, img_width, img_height, 0, nullptr, width, height, 0, STBIR_RGBA);
			}
			
			uint32_t wdst = (width > 0 && height > 0) ? width : img_width;
			uint32_t hdst = (width > 0 && height > 0) ? height : img_height;

			uint32_t wtex = next_pow2(wdst);
    		uint32_t htex = next_pow2(hdst);

			C2D_Image c2d_img;

			c2d_img.tex = new C3D_Tex;
			c2d_img.subtex = new Tex3DS_SubTexture({(u16)wdst, (u16)hdst, 0.0f, 1.0f, wdst / (float)wtex, 1.0f - (hdst / (float)htex)});

			if (!C3D_TexInit(c2d_img.tex, wtex, htex, GPU_RGBA8)) {
				delete c2d_img.tex;
				delete c2d_img.subtex;
				c2d_img.tex = nullptr;
				c2d_img.subtex = nullptr;
				return std::nullopt;
			}

			C3D_TexSetFilter(c2d_img.tex, GPU_NEAREST, GPU_NEAREST);
			C3D_TexSetWrap(c2d_img.tex, GPU_CLAMP_TO_BORDER, GPU_CLAMP_TO_BORDER);
			c2d_img.tex->border = 0x00FFFFFF;

			for(u32 y = 0; y < hdst; y++)
			{
				for(u32 x = 0; x < wdst; x++)
				{
					const u32 dst_pixel = ((((y >> 3) * (wtex >> 3) + (x >> 3)) << 6) +
										((x & 1) | ((y & 1) << 1) | ((x & 2) << 1) | ((y & 2) << 2) |
										((x & 4) << 2) | ((y & 4) << 3))) * 4;

					const u32 src_pixel = (y * (wdst * 4)) + (x * 4);

					((uint8_t*)c2d_img.tex->data)[dst_pixel + 0] = img[src_pixel + 3];
					((uint8_t*)c2d_img.tex->data)[dst_pixel + 1] = img[src_pixel + 2];
					((uint8_t*)c2d_img.tex->data)[dst_pixel + 2] = img[src_pixel + 1];
					((uint8_t*)c2d_img.tex->data)[dst_pixel + 3] = img[src_pixel + 0];
				}
			}
			
			stbi_image_free(img);
			free(imgmem.memory);
			curl_easy_cleanup(hnd);
			hnd = NULL;

			auto stop = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::seconds>(stop - start);
			std::cout << "Image downloading took " << duration.count() << " seconds.\n";
			return c2d_img;
		}
	} else {
		fprintf(stderr, "HTTP request returned %d: %s\n", ret, curl_easy_strerror(ret));
		free(imgmem.memory);
		curl_easy_cleanup(hnd);
		hnd = NULL;

		return std::nullopt;
	}
}