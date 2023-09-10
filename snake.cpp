#include <SDL2/SDL.h>
#include <iostream>

using std::cout;
using std::endl;
using std::string;

#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600

#define SQUARE_WIDTH 30
#define SQUARE_HEIGHT 30

#define MAP_WIDTH (SCREEN_WIDTH / SQUARE_WIDTH)
#define MAP_HEIGHT (SCREEN_HEIGHT / SQUARE_HEIGHT)

/* When we turn left, we can just do (dir - 1) % 4 to get the new direction.
   and (dir + 1) % 4 when turning right. More convenient i guess? */
enum Direction {
	left = 0, up = 1, right = 2, down = 3
}; 

enum SquareType {
	Empty,
	Apple,
	Snake
};

class Square {
public:
	SquareType type;
	int life = 0;

	Square() {
		type = Empty;
		life = 0;
	}

	/* Okay. We're doing this whole mental gymnastic exercise just to be able
	   to consistently  determine the tail, then remove it when the snake moves.*/
	void Age() {
		if (type == Snake)
			life += 1;
	}

	Square& operator=(SquareType new_type) {
		type = new_type;
		life = 0;
		return *this;
	}
};

/* I don't usually like to unnecessarily wrap things in a class,
   but it kinda makes sense here (there are some shared global state that I don't wanna
   make global vars. also more convenient) */
class App {
private:
	SDL_Window *window;
	SDL_Renderer *renderer;
	Square map[MAP_HEIGHT][MAP_WIDTH];
	int headx;
	int heady;
	Direction head_dir = up;
	int snake_length = 2;

	bool dead = false;

public:
	App() {
		SDL_Window *window = SDL_CreateWindow("Snake lmao", 0, 0, 600, 600, 0);
		if (window == nullptr) {
			throw "Can't create a window";
		}
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
		if (renderer == nullptr) {
			throw "Can't create a renderer";
		}
	}
	~App() {
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
	}

	void DrawBg() {
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
		SDL_RenderClear(renderer);
	}
	void DrawMap() {
		SDL_Rect rect;
		for (int y = 0; y < MAP_HEIGHT; y++) {
			for (int x = 0; x < MAP_WIDTH; x++) {
				rect.x = x * (rect.w = SQUARE_WIDTH);
				rect.y = y * (rect.h = SQUARE_HEIGHT);
				switch (map[y][x].type) {
				default: /* Hah! fallthrough */
				case Empty:
					SDL_SetRenderDrawColor(renderer, 0x70, 0x70, 0x70, SDL_ALPHA_OPAQUE);
					SDL_RenderDrawRect(renderer, &rect);
					break;
				case Apple:
					SDL_SetRenderDrawColor(renderer, 0xF0, 0x10, 0x00, SDL_ALPHA_OPAQUE);
					SDL_RenderFillRect(renderer, &rect);
					break;
				case Snake:
					SDL_SetRenderDrawColor(renderer, 0x10, 0xF0, 0x00, SDL_ALPHA_OPAQUE);
					SDL_RenderFillRect(renderer, &rect);
					break;
				}
			}
		}
	}
	void Reset() {
		for (int i = 0; i < MAP_HEIGHT; i++) {
			for (int j = 0; j < MAP_WIDTH; j++) {
				map[i][j] = Empty;
			}
		}

		heady = MAP_HEIGHT / 2;
		headx = MAP_WIDTH / 2;
		map[heady][headx] = Snake;
		dead = false;
		head_dir = up;
		snake_length = 2;
		SpawnApple();
	}
	void SpawnApple() {
		int x,y;
		do {
			x = rand() % MAP_WIDTH;
			y = rand() % MAP_HEIGHT;
		} while (map[y][x].type != Empty);
		map[y][x] = Apple;
	}
	void GameCycle() {
		if (dead) return;
		/* age the snake's body, and disintegrate the part that's too far behind. */
		for (int i = 0; i < MAP_HEIGHT; i++) {
			for (int j = 0; j < MAP_WIDTH; j++) {
				map[i][j].Age();
				if (map[i][j].life >= snake_length)
					map[i][j] = Empty;
			}
		}

		/* The above made sure that the rest of the snake's body comes along with the head,
		   now we move the head */
		int newx = headx, newy = heady;
		switch (head_dir) {
		case left: newx -= 1; break;
		case right: newx += 1; break;
		case up: newy -= 1; break;
		case down: newy += 1; break;
		default: break;
		}
		/* Bounds check. */
		if ((newx >= MAP_WIDTH) || (newx < 0) || (newy >= MAP_HEIGHT) || (newy < 0)) {
			dead = true;
			return;
		}
		if (map[newy][newx].type == Snake) {
			dead = true;
			return;
		}
		if (map[newy][newx].type == Apple) {
			snake_length += 1;
			SpawnApple();
		}
		
		/* Move the lazy boi's head */
		map[newy][newx] = Snake;
		headx = newx;
		heady = newy;
	}
	void MainLoop() {
		Reset();
		while (true) {
			SDL_Event e;
			while (SDL_PollEvent(&e)) {
				switch (e.type) {
				case SDL_QUIT:
					return;
				case SDL_KEYUP:
					if (dead)
						Reset();
					switch (e.key.keysym.sym) {
						case SDLK_LEFT:
							head_dir = static_cast<Direction>((head_dir + 3) % 4);
							break;
						case SDLK_RIGHT:
							head_dir = static_cast<Direction>((head_dir + 1) % 4);
							break;
						case SDLK_ESCAPE:
							return;
					}
				}
			}

			/* If the snake's dead, we just keep waiting until an event
			   above causes reset or quit */
			if (!dead)
				GameCycle();

			DrawBg();
			DrawMap();
			SDL_RenderPresent(renderer);
			SDL_Delay(200);
		}
	}
};


int main(void) {
	/* no log system, just puke everything to stdout lol */
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		cout << "Failed to init" << endl;
		return -1;
	}

	try {
		App app;
		app.MainLoop();
	} catch (const char *description) {
		cout << description << endl;
		return -1;
	}
	return 0;
}
