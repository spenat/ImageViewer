#define _DEFAULT_SOURCE
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>


SDL_Surface* load_image(char* filename) {

    SDL_Surface *surface;
    surface = IMG_Load(filename);

    return surface;
}

/*
char* read_next(char* directory, DIR* dp) {
    struct dirent *ep;
    char* filename;

    if ((ep = readdir(dp))) {
        filename = malloc(strlen(ep->d_name) + 1);
        strcpy(filename, ep->d_name);
    } 
    else
    {
        (void) closedir(dp);
        dp = opendir(directory);
        return "";
    }
    return filename;
}
*/

int show_image(char* filename, SDL_Renderer* renderer, int is_zoomed, int screen_width, int screen_height) {
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect SrcR;
    double scale;

    surface = load_image(filename);
    if (!surface) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create surface from image: %s", SDL_GetError());
        printf("file: %s is not of supported image type\n", filename);
        return 0;
    }

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_SetTextureAlphaMod(texture, 100);
    SDL_SetTextureColorMod(texture, 255,255,255);

    SDL_FreeSurface(surface);
    if (is_zoomed == 1) { // && surface->h > screen_height) {
        scale = (double)surface->h / screen_height;
    } else {
        scale = 1;
    }
    SrcR.x = (int)((screen_width - (surface->w / scale)) / 2);
    SrcR.y = (int)((screen_height - (surface->h / scale)) / 2);
    SrcR.w = surface->w / scale;
    SrcR.h = surface->h / scale;
    SDL_RenderClear(renderer);
    SDL_RenderCopyEx(renderer, texture, NULL, &SrcR, 0, NULL, SDL_FLIP_NONE);
    SDL_DestroyTexture(texture);
    return 1;
}

/*
char* search_next_image(char* directory, DIR* dp, SDL_Renderer* renderer, int is_zoomed, int screen_height, int screen_width)
{
    char* filename;
    char* temp_filename;
    while ((filename = read_next(directory, dp))) {
        if (strcmp(filename, ".") && strcmp(filename, "..")) 
        {
        temp_filename = malloc(strlen(directory) + strlen(filename) + 1);
        strcpy(temp_filename, directory);
        strcat(temp_filename, filename);
        //printf("temp_filename: %s\n", temp_filename);
        if (show_image(temp_filename, renderer, is_zoomed, screen_width, screen_height))
            break;
        if (!strcmp(filename, ""))
            break; 
        }
    }
    return temp_filename;
}
*/

int search_next_image(int index, int n, char* directory, struct dirent** filelist, SDL_Renderer* renderer, int is_zoomed, int screen_height, int screen_width) {
    char* temp_filename;
    index++;
    while((index < n)) {
        temp_filename = malloc(strlen(directory) + strlen(filelist[index]->d_name) + 1);
        strcpy(temp_filename, directory);
        strcat(temp_filename, filelist[index]->d_name);
        printf("temp_filename: %s\n", temp_filename);
        if (show_image(temp_filename, renderer, is_zoomed, screen_width, screen_height))
            return index;
        else
            index++;
    }
    return 0;
}

int search_prev_image(int index, int n, char* directory, struct dirent** filelist, SDL_Renderer* renderer, int is_zoomed, int screen_height, int screen_width) {
    char* temp_filename;
    index--;
    while((index >= 0)) {
        temp_filename = malloc(strlen(directory) + strlen(filelist[index]->d_name) + 1);
        strcpy(temp_filename, directory);
        strcat(temp_filename, filelist[index]->d_name);
        printf("temp_filename: %s\n", temp_filename);
        if (show_image(temp_filename, renderer, is_zoomed, screen_width, screen_height))
            return index;
        else
            index--;
    }
    return n - 1;
}

void slide_show(int index, int n, char* directory, struct dirent** filelist, SDL_Renderer* renderer, int is_zoomed, int screen_height, int screen_width)
{
    char* temp_filename;
    int running = 1;
    int counter = 0;
    int wait_length = 40; // 40 * 50 = 2000 (2 sec)
    SDL_Event event;
    printf("Start slideshow\n");
    while (running) {
        SDL_PollEvent(&event);
        SDL_Delay(50);
        switch (event.type) {
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == 113) {
                    running = 0;
                    printf("Stop slideshow\n");
                }
                else if (event.key.keysym.sym == 98 || event.key.keysym.sym == 122) {
                    if (is_zoomed)
                        is_zoomed = 0;
                    else
                        is_zoomed = 1;
                    show_image(temp_filename, renderer, is_zoomed, screen_width, screen_height);

                }
                else if (event.key.keysym.sym == 43) {
                    wait_length += 10;
                    printf("increased wait time to %dms\n", wait_length * 50);
                }
                else if (event.key.keysym.sym == 45 && wait_length > 10) {
                    wait_length -= 10;
                    printf("decreased wait time to %dms\n", wait_length * 50);
                }

                break;
            default:
                if (counter > wait_length) {
                    index = search_next_image(index, n, directory, filelist, renderer, is_zoomed, screen_height, screen_width);
                    temp_filename = malloc(strlen(directory) + strlen(filelist[index]->d_name) + 1);
                    strcpy(temp_filename, directory);
                    strcpy(temp_filename, filelist[index]->d_name);
                    counter = 0;
                } else {
                    counter++;
                }
                break;
        }

        SDL_RenderPresent(renderer);
    }
}


int main(int argc, char *argv[])
{
    SDL_DisplayMode current;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Event event;
    int is_zoomed = 1;
    int screen_width, screen_height;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
        return 3;
    }

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    for (int i=0; i < SDL_GetNumVideoDisplays(); i++) {
        if (SDL_GetCurrentDisplayMode(i, &current) != 0)
            SDL_Log("Could not get display mode for video display #%d: %s", i, SDL_GetError());
        else {
            SDL_Log("Display #%d: current display mode is %dx%dpx @ %dhz.", i, current.w, current.h, current.refresh_rate);
            screen_width = current.w;
            screen_height = current.h;
        }
    }

    if (IMG_Init(IMG_INIT_JPG) < 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize IMG: %s", SDL_GetError());
        return 3;
    }

    if (SDL_CreateWindowAndRenderer(screen_width, screen_height, SDL_WINDOW_RESIZABLE | SDL_RENDERER_ACCELERATED, &window, &renderer)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
        return 3;
    }
    SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" );
    char* directory = "./";
    char* temp_filename;
    // DIR *dp;
    struct dirent **filelist;
    int n;
    int index = 0;
    // char lastchar;

    if (argc > 1) {
        directory = argv[1];
        //lastchar = directory[strlen(directory) - 1];
        //printf("lastchar: |%c|\n", lastchar);
        if (directory[strlen(directory) - 1] != '/')
            strcat(directory, "/");
    }

    printf("directory: %s\n", directory);

    n = scandir(directory, &filelist, 0, alphasort);
    if (n < 0)
        perror("scandir");
    /*
    else
    {
        for (int index = 0; index < n; ++index)
        {
            printf("%s\n", filelist[index]->d_name);
        }
    }
    */
    // dp = opendir(directory);
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0x00);
    SDL_RenderClear(renderer);
 
    int running = 1;

    index = search_next_image(index, n, directory, filelist, renderer, is_zoomed, screen_height, screen_width);
    temp_filename = malloc(strlen(directory) + strlen(filelist[index]->d_name) + 1);
    strcpy(temp_filename, directory);
    strcat(temp_filename, filelist[index]->d_name);

    while (running) {
        SDL_PollEvent(&event);
        SDL_Delay(50);
        switch (event.type) {
            case SDL_KEYDOWN:
                printf("key pressed: %d\n", event.key.keysym.sym);
                if (event.key.keysym.sym == 113) // press q
                    running = 0;
                else if (event.key.keysym.sym == 32 || event.key.keysym.sym == 46) { // press space
                    index = search_next_image(index, n, directory, filelist, renderer, is_zoomed, screen_height, screen_width);
                    printf("index: %d\n", index);
                    temp_filename = malloc(strlen(directory) + strlen(filelist[index]->d_name) + 1);
                    strcpy(temp_filename, directory);
                    strcat(temp_filename, filelist[index]->d_name);
                }
                else if (event.key.keysym.sym == 8 || event.key.keysym.sym == 44) { // press space
                    index = search_prev_image(index, n, directory, filelist, renderer, is_zoomed, screen_height, screen_width);
                    printf("index: %d\n", index);
                    temp_filename = malloc(strlen(directory) + strlen(filelist[index]->d_name) + 1);
                    strcpy(temp_filename, directory);
                    strcat(temp_filename, filelist[index]->d_name);
                }
                else if (event.key.keysym.sym == 98 || event.key.keysym.sym == 122) { // press b or z
                    if (is_zoomed)
                        is_zoomed = 0;
                    else
                        is_zoomed = 1;
                    show_image(temp_filename, renderer, is_zoomed, screen_width, screen_height);
                }
                else if (event.key.keysym.sym == 115 || event.key.keysym.sym == 102) // press s or f
                {
                    slide_show(index, n, directory, filelist, renderer, is_zoomed, screen_height, screen_width);
                }
                else {
                    show_image(temp_filename, renderer, is_zoomed, screen_width, screen_height);
                }
                break;
            case SDL_QUIT:
                running = 0;
                break;
            default:
                break;
        }
        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    SDL_Quit();

    return 0;
}
