#include "gtest/gtest.h"

#include <string>
#include <SDL/SDL.h>

#include "../src/infoPanel/imagesCache.h"

#define STR_MAX 256

typedef struct
{
    int initial_index;
    int new_index;
    bool cache_used;
    std::string drawn_image_path;
} TestItem;

TEST(test_infoPanel, cacheTest)
{
    char **images_paths = NULL;
    int images_paths_count = 5;
    SDL_Surface *screen = NULL;

    const int test_data_count = 10;
    TestItem test_data[test_data_count];
    test_data[0] = { 0, -1, false, "" };
    test_data[1] = { 0, 0, false, "./page0.png" };
    test_data[2] = { 0, 1, true, "./page1.png" };
    test_data[3] = { 1, 2, true, "./page2.png" };
    test_data[4] = { 2, 3, true, "./page3.png" };
    test_data[5] = { 3, 2, true, "./page2.png" };
    test_data[6] = { 2, 4, false, "" }; // random jump is not yet implemented
    test_data[7] = { 2, 3, true, "./page3.png" };
    test_data[8] = { 3, 4, true, "./page4.png" };
    test_data[9] = { 4, 5, false, "" };

    images_paths = (char**)malloc(images_paths_count * sizeof(char*));

	for (int i = 0; i < images_paths_count; i++)
	{
		images_paths[i] = (char*)malloc(STR_MAX * sizeof(char));

		strcat(images_paths[i], "./page");
        char index_str[10];
        sprintf(index_str, "%d", i);
        strcat(images_paths[i], index_str);
        strcat(images_paths[i], ".png");
	}

    char* drawn_image_path = NULL;
    for (int i = 0; i < test_data_count; i++)
    {
        printf("Entering test item #%d\n", i);
        const TestItem& test_item = test_data[i];
        bool cache_used = false;
        drawn_image_path = drawImageByIndex(test_item.new_index, test_item.initial_index,
            images_paths, images_paths_count, screen, &cache_used);

        if (drawn_image_path != NULL)
        {
            ASSERT_STREQ(drawn_image_path, test_item.drawn_image_path.c_str());
        }
        else
        {
            ASSERT_EQ(test_item.drawn_image_path, "");
        }
        
        ASSERT_EQ(cache_used, test_item.cache_used);
    }
}