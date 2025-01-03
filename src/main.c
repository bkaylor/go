#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"

#include "vec2.h"
#include "draw.h"

typedef struct {
    int x;
    int y;
} Window;

typedef enum {
    BLACK,
    WHITE,
    NONE
} Stone_Type;

typedef struct {
    Stone_Type type;
    vec2 position;
    int group_id;
} Stone;

#define BOARD_SIZE 19
#define HISTORY_SIZE 200

typedef struct {
    Stone board[BOARD_SIZE][BOARD_SIZE];
    SDL_Rect board_rect;
    SDL_Rect playable_rect;
    float stone_width;

    int turn;
} Game;

typedef struct {
    int x;
    int y;
    int pressed;

    int hx;
    int hy;
    bool hovered;
} Mouse_State;

typedef struct {
    Window window;
    Game game;
    Mouse_State mouse_state;
    bool quit;
    bool reset;
} State;

/*  
    // TODO(bkaylor): This is wrong.

      a b c d e f g h j k l m n o p q r s t
   19 _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ 19
   18 |                                   | 18
   17 |                                   | 17
   16 |                                   | 16
   15 |                                   | 15
   14 |                                   | 14
   13 |                                   | 13
   12 |                                   | 12
   11 |                                   | 11
   10 |                                   | 10
   9  |                                   | 9
   8  |                                   | 8
   7  |                                   | 7
   6  |                                   | 6
   5  |                                   | 5
   4  |                                   | 4
   3  |                                   | 3
   2  |                                   | 2
   1  _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ 1
      a b c d e f g h j k l m n o p q r s t
*/

void render(SDL_Renderer *renderer, State state, TTF_Font *font)
{
    SDL_RenderClear(renderer);

    // Set background color.
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, NULL);

    // Draw board
    float stone_width = state.game.stone_width;
    Game game = state.game;
    SDL_Rect board_rect = game.board_rect;

    SDL_SetRenderDrawColor(renderer, 127, 127, 127, 255);
    SDL_RenderFillRect(renderer, &state.game.board_rect);

    SDL_Rect playable_rect = game.playable_rect;
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
    SDL_RenderFillRect(renderer, &state.game.playable_rect);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    for (int i = 0; i < BOARD_SIZE; i += 1)
    {
        SDL_RenderDrawLine(renderer, 
                           playable_rect.x, 
                           playable_rect.y + stone_width * i, 
                           playable_rect.x + playable_rect.w, 
                           playable_rect.y + stone_width * i);
    }

    for (int j = 0; j < BOARD_SIZE; j += 1)
    {
        SDL_RenderDrawLine(renderer, 
                           playable_rect.x + stone_width * j, 
                           playable_rect.y, 
                           playable_rect.x + stone_width * j, 
                           playable_rect.y + playable_rect.h);
    }

    // Draw labels.
    SDL_Color black_font_color = (SDL_Color){0, 0, 0, 255};
    SDL_Color white_font_color = (SDL_Color){255, 255, 255, 255};
    for (int i = 0; i < BOARD_SIZE; i += 1)
    {
        char label[2];
        // The letter I/i is commonly skipped in the labels, I guess because it's hard to read.
        sprintf(label, "%c", 65+i + ((i >= 8) ? 1 : 0));
        draw_text(renderer, 
                  playable_rect.x + (i*stone_width) - 2, 
                  playable_rect.y + playable_rect.h + 30, 
                  label,
                  font,
                  black_font_color);
    }

    for (int i = 0; i < BOARD_SIZE; i += 1)
    {
        char label[2];
        sprintf(label, "%d", 19-(i));
        draw_text(renderer, 
                  playable_rect.x - 40, 
                  playable_rect.y + (i * stone_width) - 5, 
                  label,
                  font,
                  black_font_color);
    }

    // Draw markings.
    draw_filled_circle(renderer, 
                       playable_rect.x + (3*stone_width), 
                       playable_rect.y + (3*stone_width),
                       3);
    draw_filled_circle(renderer, 
                       playable_rect.x + (9*stone_width), 
                       playable_rect.y + (3*stone_width),
                       3);
    draw_filled_circle(renderer, 
                       playable_rect.x + (15*stone_width), 
                       playable_rect.y + (3*stone_width),
                       3);

    draw_filled_circle(renderer, 
                       playable_rect.x + (3*stone_width), 
                       playable_rect.y + (9*stone_width),
                       3);
    draw_filled_circle(renderer, 
                       playable_rect.x + (9*stone_width), 
                       playable_rect.y + (9*stone_width),
                       3);
    draw_filled_circle(renderer, 
                       playable_rect.x + (15*stone_width), 
                       playable_rect.y + (9*stone_width),
                       3);

    draw_filled_circle(renderer, 
                       playable_rect.x + (3*stone_width), 
                       playable_rect.y + (15*stone_width),
                       3);
    draw_filled_circle(renderer, 
                       playable_rect.x + (9*stone_width), 
                       playable_rect.y + (15*stone_width),
                       3);
    draw_filled_circle(renderer, 
                       playable_rect.x + (15*stone_width), 
                       playable_rect.y + (15*stone_width),
                       3);

    // Draw stones.
    for (int i = 0; i < BOARD_SIZE; i += 1)
    {
        for (int j = 0; j < BOARD_SIZE; j += 1)
        {
            Stone stone = state.game.board[i][j];

            if (stone.type == NONE) continue;

            if (stone.type == BLACK)
            {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }

            draw_filled_circle(renderer, 
                        state.game.playable_rect.x + i * stone_width, 
                        state.game.playable_rect.y + j * stone_width, 
                        (stone_width/2)-1);


            SDL_Color font_color = stone.type == BLACK ? white_font_color : black_font_color;
            char group[3];
            sprintf(group, "%d", stone.group_id);
            draw_text(renderer, 
                      state.game.playable_rect.x + i * stone_width, 
                      state.game.playable_rect.y + j * stone_width, 
                      group,
                      font,
                      font_color);
        }
    }

    // Draw hovered.
    Mouse_State mouse = state.mouse_state;
    if (mouse.hovered)
    {
        // TODO(bkaylor): This should be a slightly transparent stone of the right color based on turn. 
        if (game.turn % 2 == 0)
        {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        }

        draw_circle(renderer,
                    state.game.playable_rect.x + mouse.hx * stone_width,
                    state.game.playable_rect.y + mouse.hy * stone_width,
                    stone_width/2);
    }

    SDL_RenderPresent(renderer);
}

bool rect_intersect(SDL_Rect r1, SDL_Rect r2)
{
    return (r2.x + r2.w > r1.x &&
            r2.y + r2.h > r1.y && 
            r1.x + r1.w > r2.x &&
            r1.y + r1.h > r2.y);
}

bool xy_in_rect(int x, int y, SDL_Rect rect)
{
    return rect_intersect((SDL_Rect){x, y, 1, 1}, rect);
}

void update(State *state) 
{
    Game *game = &state->game;

    // New game!
    if (state->reset)
    {
        for (int i = 0; i < BOARD_SIZE; i += 1)
        {
            for (int j = 0; j < BOARD_SIZE; j += 1)
            {
                Stone *stone = &game->board[i][j];
                stone->type = NONE;
                stone->group_id = 0;
            }
        }

        game->turn = 0;
        state->reset = false;
    }

    // Window layouting.
    Window window = state->window;

    {
        float top_padding =     0.04f;
        float left_padding =    0.04f;
        float right_padding =   0.04f;
        float bottom_padding =  0.04f;

        // Force square.
        int size = window.x > window.y ? window.y : window.x;

        game->board_rect.x = size * left_padding;
        game->board_rect.y = size * top_padding;

        game->board_rect.w = size * (1.0f - left_padding) * (1.0f - right_padding);
        game->board_rect.h = size * (1.0f - top_padding) * (1.0f - bottom_padding);
    }

    {
        float top_padding =     0.06f;
        float left_padding =    0.06f;
        float right_padding =   0.06f;
        float bottom_padding =  0.06f;

        SDL_Rect board_rect = game->board_rect;

        game->playable_rect.x = board_rect.x + (board_rect.w * left_padding);
        game->playable_rect.y = board_rect.y + (board_rect.h * top_padding);

        game->playable_rect.w = board_rect.w * (1.0f - left_padding) * (1.0f - right_padding);
        game->playable_rect.h = board_rect.h * (1.0f - top_padding) * (1.0f - bottom_padding);
    }

    game->stone_width = (float)game->playable_rect.w / (BOARD_SIZE-1);

    // Check if there's a hovered tile.
    Mouse_State *mouse = &state->mouse_state;
    
    mouse->hovered = false;
    if (xy_in_rect(mouse->x, mouse->y, game->playable_rect))
    {
        mouse->hx = (int)((mouse->x - game->playable_rect.x + (game->stone_width/2))/game->stone_width) % (BOARD_SIZE);
        mouse->hy = (int)((mouse->y - game->playable_rect.y + (game->stone_width/2))/game->stone_width) % (BOARD_SIZE);

        if (game->board[mouse->hx][mouse->hy].type != NONE)
        {
            mouse->hovered = false;
        }
        else
        {
            mouse->hovered = true;
        }

    }

    // Handle clicks.
    if (mouse->hovered && mouse->pressed == SDL_BUTTON_LEFT)
    {
        // TODO(bkaylor): There should be some cases where you aren't allowed to place a stone.
        // For example, if it would immediately die without capturing anything.
        // Also, something about ko?
        game->board[mouse->hx][mouse->hy].type = (game->turn % 2 == 0) ? BLACK : WHITE;
        game->turn += 1;

        // TODO(bkaylor): If a stone was placed, we're about to get a new board state.
        // Write the old board state to the history buffer.

        // A stone was placed- reset groups
        for (int i = 0; i < BOARD_SIZE; i += 1)
        {
            for (int j = 0; j < BOARD_SIZE; j += 1)
            {
                Stone *stone = &game->board[i][j];
                stone->group_id = 0;
            }
        }

        // Create groups.
        int next_group_id = 1;
        for (int i = 0; i < BOARD_SIZE; i += 1)
        {
            for (int j = 0; j < BOARD_SIZE; j += 1)
            {
                Stone *stone= &game->board[i][j];
                if (stone->type != NONE)
                {
                    if (stone->group_id == 0)
                    {
                        Stone *neighbors[4] = {0};
                        // Check neighbors
                        if (j-1 >= 0)
                        {
                            neighbors[0] = &game->board[i][j-1];
                        }

                        if (j+1 < BOARD_SIZE)
                        {
                            neighbors[1] = &game->board[i][j+1];
                        }

                        if (i-1 >= 0)
                        {
                            neighbors[2] = &game->board[i-1][j];
                        }

                        if (i+1 < BOARD_SIZE)
                        {
                            neighbors[3] = &game->board[i+1][j];
                        }

                        int adjacent_groups[4] = {0};
                        int adjacent_group_count = 0;
                        for (int k = 0; k < 4; k += 1)
                        {
                            Stone *neighbor = neighbors[k];
                            if (neighbor && (neighbor->type == stone->type) && (neighbor->group_id != 0))
                            {
                                adjacent_groups[adjacent_group_count] = neighbor->group_id;
                                adjacent_group_count += 1;
                            }
                        }

                        if (adjacent_group_count == 1)
                        {
                            // Only one adjacent group to this stone- join the group
                            stone->group_id = adjacent_groups[0];
                        }
                        else if (adjacent_group_count > 1)
                        {
                            // Two or more adjacent groups to this stone- join them together

                            int destination_group = adjacent_groups[0];
                            stone->group_id = destination_group;
                            for (int k = 1; k < adjacent_group_count; k += 1)
                            {
                                int group_to_convert = adjacent_groups[k];
                                for (int i2 = 0; i2 < BOARD_SIZE; i2 += 1)
                                {
                                    for (int j2 = 0; j2 < BOARD_SIZE; j2 += 1)
                                    {
                                        Stone *cursor = &game->board[i2][j2];
                                        if (cursor->type != NONE)
                                        {
                                            if (cursor->group_id == group_to_convert)
                                            {
                                                cursor->group_id = destination_group;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            // No adjacent groups- create a new one
                            stone->group_id = next_group_id;
                            next_group_id += 1;
                        }
                    }
                }
            }
        }

        // Count liberties, destroy if needed.
        for (int group_id_to_check = 1; group_id_to_check < next_group_id; group_id_to_check += 1)
        {
            int liberties = 0;
            for (int i = 0; i < BOARD_SIZE; i += 1)
            {
                for (int j = 0; j < BOARD_SIZE; j += 1)
                {
                    Stone *stone= &game->board[i][j];
                    if (stone->type != NONE)
                    {
                        if (stone->group_id == group_id_to_check)
                        {
                            int this_stones_liberties = 4;

                            // Check neighbors
                            if (j-1 >= 0)
                            {
                                Stone *neighbor = &game->board[i][j-1];
                                if (neighbor->type != NONE)
                                {
                                    this_stones_liberties -= 1;
                                }
                            }
                            else
                            {
                                this_stones_liberties -= 1;
                            }

                            if (j+1 < BOARD_SIZE)
                            {
                                Stone *neighbor = &game->board[i][j+1];
                                if (neighbor->type != NONE)
                                {
                                    this_stones_liberties -= 1;
                                }
                            }
                            else
                            {
                                this_stones_liberties -= 1;
                            }

                            if (i-1 >= 0)
                            {
                                Stone *neighbor = &game->board[i-1][j];
                                if (neighbor->type != NONE)
                                {
                                    this_stones_liberties -= 1;
                                }
                            }
                            else
                            {
                                this_stones_liberties -= 1;
                            }

                            if (i+1 < BOARD_SIZE)
                            {
                                Stone *neighbor = &game->board[i+1][j];
                                if (neighbor->type != NONE)
                                {
                                    this_stones_liberties -= 1;
                                }
                            }
                            else
                            {
                                this_stones_liberties -= 1;
                            }

                            liberties += this_stones_liberties;
                        }
                    }
                }
            }

            if (liberties == 0)
            {
                for (int i = 0; i < BOARD_SIZE; i += 1)
                {
                    for (int j = 0; j < BOARD_SIZE; j += 1)
                    {
                        Stone *stone= &game->board[i][j];
                        if (stone->type != NONE)
                        {
                            if (stone->group_id == group_id_to_check)
                            {
                                stone->type = NONE;
                                stone->group_id = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    return;
}

void get_input(State *state)
{
    state->mouse_state.pressed = 0;
    SDL_GetMouseState(&state->mouse_state.x, &state->mouse_state.y);

    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        state->quit = true;
                        break;

                    case SDLK_SPACE:
                        break;

                    case SDLK_TAB:
                        break;
                    
                    case SDLK_r:
                        state->reset = true;
                        break;

                    case SDLK_UP:
                        break;

                    case SDLK_DOWN:
                        break;

                    default:
                        break;
                }
                break;

            case SDL_QUIT:
                state->quit = true;
                break;
            
            case SDL_MOUSEBUTTONDOWN:
                state->mouse_state.pressed = event.button.button;
                break;

            default:
                break;
        }
    }
}

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG);

    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        printf("SDL_Init video error: %s\n", SDL_GetError());
        return 1;
    }

    if (SDL_Init(SDL_INIT_AUDIO) != 0)
    {
        printf("SDL_Init audio error: %s\n", SDL_GetError());
        return 1;
    }

	SDL_Window *win = SDL_CreateWindow("Go",
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			1000, 1000,
			SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	TTF_Init();
	TTF_Font *font = TTF_OpenFont("liberation.ttf", 12);
	if (!font)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error: Font", TTF_GetError(), win);
		return -666;
	}

    srand(time(NULL));

    State state;
    state.quit = false;
    state.reset = true;

    //
    // Main loop
    //
    while (!state.quit)
    {
        SDL_PumpEvents();
        get_input(&state);

        if (!state.quit)
        {
            int x, y;
            SDL_GetWindowSize(win, &state.window.x, &state.window.y);

            update(&state);
            render(ren, state, font);
        }
    }

    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
