// sdl_symbols.c
#include "all_headers.h"

int load_textures(void) {
    int num;
    int num_failures=0;
    printf("load_textures() loading %d objects.\n",num_objects);
    for (num=0;num<num_objects;num++) {
        if (objects[num].graphic_type==0) {
            printf("Loading texture for %s from file \"%s\", ojbect_mode = %d\n",objects[num].description, objects[num].graphic_name, objects[num].object_mode);
            objects[num].texture = load_symbol_texture(objects[num].graphic_name, objects[num].graphic_color, objects[num].object_mode);

            if (objects[num].texture==NULL) {
                printf("FAILED, returned NULL: %s\n",SDL_GetError());
                num_failures++;
                objects[num].graphic_type = 0; // 0 = none
            } else {
                printf("Success texture=%p\n",&objects[num].texture);
                objects[num].graphic_type = 1; // 1 = image texture
            }
        }
    }
    return num_failures;
}

int load_monster_textures(void) {
    int mnum;
    int num_failures=0;
    printf("load_monster_textures() loading %d monsters.\n",num_monsters);
    for (mnum=0;mnum<num_monsters;mnum++) {
        if (monsters[mnum].graphic_type==0) {
            printf("Loading texture for %s from file \"%s\", monster_mode = %d\n",monsters[mnum].description, monsters[mnum].graphic_name, monsters[mnum].monster_mode);
            monsters[mnum].texture = load_symbol_texture(monsters[mnum].graphic_name, monsters[mnum].graphic_color, monsters[mnum].monster_mode);
            
            if (monsters[mnum].texture==NULL) {
                printf("FAILED, returned NULL: %s\n",SDL_GetError());
                num_failures++;
                monsters[mnum].graphic_type = 0; // 0 = none
            } else {
                printf("Success texture=%p\n",&monsters[mnum].texture);
                monsters[mnum].graphic_type = 1; // 1 = image texture
            }
        }
    }
    return num_failures;
}

#define MODE_TEXTURE_NONE     0
#define MODE_TEXTURE_NO_COLOR 1
#define MODE_TEXTURE_COLOR    2
//SDL_Texture* load_symbol_texture( char *path, int r, int g, int b, int a ) {
SDL_Texture* load_symbol_texture( char *path, SDL_Color color, int mode ) {
    //The final optimized image as texture
    SDL_Texture* texture = NULL;
    SDL_Surface* loadedSurface = NULL;

    if (window == NULL) {printf("window==NULL\n");}

    //Load image at specified path
    if (mode!=MODE_TEXTURE_NONE) {
        loadedSurface = IMG_Load( path );
        if( loadedSurface == NULL ) {
            printf( "Unable to load image %s! SDL_image Error: %s\n", path, IMG_GetError() );
            return (NULL);
        }
    }
    //Convert surface to texture (surface is on the CPU, texture is on the GPU)
    texture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
    if( texture == NULL ) {
        printf( "Unable to CreateTextureFromSurface %s! SDL Error: %s\n", path, SDL_GetError() );
    }

    if (mode == MODE_TEXTURE_NONE) { // no texture
        printf("MODE_TEXTURE_NONE:");
        texture = NULL;
    } else {                 // loaded texture
        if (mode == MODE_TEXTURE_NO_COLOR) { // uncolored texxtue
            printf("MODE_TEXTURE_NO_COLOR: %s  color = (%X, %X, %X), alpha = %X\n", path, color.r, color.g, color.b, color.a);
            SDL_SetTextureBlendMode(texture, 1);
        } else if (mode == MODE_TEXTURE_COLOR) { // colored texture
            printf("MODE_TEXTURE_COLOR: %s  color = (%X, %X, %X), alpha = %X\n", path, color.r, color.g, color.b, color.a);
            SDL_SetTextureBlendMode(texture, 1);
            SDL_SetTextureColorMod(texture, color.r, color.g, color.b);
            SDL_SetTextureAlphaMod(texture, color.a);
        }
    }

    //Get rid of old loaded surface
    SDL_FreeSurface( loadedSurface );

    return texture;
}

int draw_image (SDL_Texture *texture, int x,int y,int w,int h, int dir, int panel) {
    SDL_Rect dest_rect;
    SDL_FRect dest_rect_f;
    SDL_Rect src_rect;
    SDL_Point center_pt;
    SDL_FPoint center_pt_f;
    float angle;
    SDL_RendererFlip flip;
    int t_rects;

    dest_rect.x = x - w/2;
    dest_rect.y = y - h/2;
    dest_rect.w = w;
    dest_rect.h = h;
    angle = dir *90.0;
    center_pt.x = w/2.0; center_pt.y = h/2.0;
    flip = SDL_FLIP_NONE;
    if (dir==-3) {
        flip = SDL_FLIP_HORIZONTAL;
        dir = 0;
    }
    angle = dir *90.0;
//    SDL_RenderCopy(renderer, texture, NULL, &dest_rect);
    SDL_Point t_size;
    SDL_QueryTexture(texture, NULL, NULL, &t_size.x, &t_size.y);
    if (t_size.y==0) return 0; // texture is empty, prevent divide by zero.
    t_rects = (t_size.x / t_size.y);
    if (t_rects<1) t_rects=1; // image is taller than wide, only a single panel
    src_rect.h = t_size.y;
    src_rect.w = t_size.y;
    src_rect.x = (panel%t_rects * t_size.y);
    src_rect.y = 0;
    center_pt_f.x = center_pt.x; center_pt_f.y = center_pt.y;
    dest_rect_f.x = dest_rect.x; dest_rect_f.y = dest_rect.y;
    dest_rect_f.h = dest_rect.h; dest_rect_f.w = dest_rect.w;
    //printf("SDL_RenderCopyExF (texture=%p)\n",&texture);
    SDL_RenderCopyExF(renderer, texture, &src_rect, &dest_rect_f,
        angle,&center_pt_f,flip);
    return 1;
}
