
#include "raylib.h"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define SNAKE_LENGTH 256
#define SQUARE_SIZE 32

Color green = {173, 204, 96, 255};
Color darkGreen = {41, 51, 22, 255};

typedef enum
{
	MENU,
	GIOCO
} Modalita;

typedef struct
{
	Rectangle rectangle;
	Color color;
	char *text;
} Bottone;

typedef struct Snake
{
	Vector2 position;
	Vector2 size;
	Vector2 speed;
	Color color;
	Texture2D im2;
} Snake;

typedef struct Food
{
	Vector2 position;
	Vector2 size;
	bool active;
	Texture2D im1;
	Color color;
} Food;

static const int screenWidth = 800;
static const int screenHeight = 750;

static int framesCounter = 0;
static bool gameOver = false;
static bool pause = false;
static int score = 0;

static Food fruit = {0};
static Snake snake[SNAKE_LENGTH] = {0};
static Vector2 snakePosition[SNAKE_LENGTH] = {0};
static bool allowMove = false;
static Vector2 offset = {0};
static int counterTail = 0;

char easterEggSequence[] = "UPUPDOWNDOWNLEFT";
char userInputSequence[sizeof(easterEggSequence)] = {0};
int userInputIndex = 0;

// suoni
static Music music;
Sound eatSound;
Sound gameoverSound;

static Modalita modalita = MENU;
static Bottone bottoneStart = {
	{(screenWidth - 500) / 2,
	 (screenHeight - 250) / 2,
	 500,
	 250},
	BLACK,
	"SNAKE"};

static void InitGame(void);		   // Initialize game
static void UpdateGame(void);	   // Update game (one frame)
static void DrawGame(void);		   // Draw game (one frame)
static void UnloadGame(void);	   // Unload game
static void UpdateDrawFrame(void); // Update and Draw (one frame)

int main(void)
{

	InitWindow(screenWidth, screenHeight, "Snake");

	InitGame();

	SetTargetFPS(60);

	// musica del gioco
	InitAudioDevice();
	music = LoadMusicStream("resources/musica.mp3");
	PlayMusicStream(music);

	// audio quando mangia la mela
	eatSound = LoadSound("resources/eat.mp3");

	gameoverSound = LoadSound("resources/gameover.mp3");

	// Main game loop
	while (!WindowShouldClose())
	{

		UpdateMusicStream(music);
		UpdateDrawFrame();
	}

	UnloadGame();

	CloseWindow();

	return 0;
}

void InitGame(void)
{
	framesCounter = 0;
	gameOver = false;
	pause = false;

	counterTail = 1;
	allowMove = false;

	offset.x = screenWidth % SQUARE_SIZE;
	offset.y = 700 % SQUARE_SIZE;

	Texture2D snakeTexture = LoadTexture("resources/snake.png");

	for (int i = 0; i < SNAKE_LENGTH; i++)
	{
		snake[i].position = (Vector2){offset.x / 2, offset.y / 2};
		snake[i].size = (Vector2){SQUARE_SIZE, SQUARE_SIZE};
		snake[i].speed = (Vector2){SQUARE_SIZE, 0};

		snake[i].im2 = snakeTexture;

		if (i == 0)
			snake[i].color = green;
		else
			snake[i].color = BLACK;
	}

	for (int i = 0; i < SNAKE_LENGTH; i++)
	{
		snakePosition[i] = (Vector2){0.0f, 0.0f};
	}

	fruit.size = (Vector2){SQUARE_SIZE, SQUARE_SIZE};
	fruit.color = RED;
	fruit.active = false;
	fruit.im1 = LoadTexture("resources/mela.png");
}

void UpdateGame(void)
{
	switch (modalita)
	{
	case MENU:
		if (CheckCollisionPointRec(GetMousePosition(), bottoneStart.rectangle))
		{
			bottoneStart.color = GREEN;

			if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
			{
				bottoneStart.color = BLACK;
			}

			if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON))
			{
				modalita = GIOCO;
			}
		}
		else
		{
			bottoneStart.color = darkGreen;
		}

		break;

	case GIOCO:
		if (!gameOver)
		{
			if (IsKeyPressed(KEY_SPACE))
				pause = !pause;

			if (!pause)
			{
				// controlli del giocatore
				if (IsKeyPressed(KEY_RIGHT) && (snake[0].speed.x == 0) && allowMove)
				{
					snake[0].speed = (Vector2){SQUARE_SIZE, 0};
					allowMove = false;
				}
				if (IsKeyPressed(KEY_LEFT) && (snake[0].speed.x == 0) && allowMove)
				{
					snake[0].speed = (Vector2){-SQUARE_SIZE, 0};
					allowMove = false;
				}
				if (IsKeyPressed(KEY_UP) && (snake[0].speed.y == 0) && allowMove)
				{
					snake[0].speed = (Vector2){0, -SQUARE_SIZE};
					allowMove = false;
				}
				if (IsKeyPressed(KEY_DOWN) && (snake[0].speed.y == 0) && allowMove)
				{
					snake[0].speed = (Vector2){0, SQUARE_SIZE};
					allowMove = false;
				}

				// mouvimenti dello snake
				for (int i = 0; i < counterTail; i++)
					snakePosition[i] = snake[i].position;

				if ((framesCounter % 7) == 0)
				{
					for (int i = 0; i < counterTail; i++)
					{
						if (i == 0)
						{

							snake[0].position.x += snake[0].speed.x;
							snake[0].position.y += snake[0].speed.y;
							allowMove = true;
						}
						else
							snake[i].position = snakePosition[i - 1];
					}
				}

				// collisioni 1
				if (((snake[0].position.x) > (screenWidth - offset.x)) ||
					((snake[0].position.y) > (700 - offset.y)) ||
					(snake[0].position.x < 0) || (snake[0].position.y < 0))
				{
					gameOver = true;
					PlaySound(gameoverSound);
				}

				if ((snake[0].position.x + snake[0].size.x) > screenWidth)
				{
					gameOver = true;
					PlaySound(gameoverSound);
				}

				// collisioni 2
				for (int i = 1; i < counterTail; i++)
				{
					if ((snake[0].position.x == snake[i].position.x) && (snake[0].position.y == snake[i].position.y))
						gameOver = true;
					PlaySound(gameoverSound);
				}

				// posizione della frutta
				if (!fruit.active)
				{
					fruit.active = true;
					fruit.position = (Vector2){GetRandomValue(0, (screenWidth / SQUARE_SIZE) - 1) * SQUARE_SIZE + offset.x / 2, GetRandomValue(0, (700 / SQUARE_SIZE) - 1) * SQUARE_SIZE + offset.y / 2};

					for (int i = 0; i < counterTail; i++)
					{
						while ((fruit.position.x == snake[i].position.x) && (fruit.position.y == snake[i].position.y))
						{
							fruit.position = (Vector2){GetRandomValue(0, (screenWidth / SQUARE_SIZE) - 1) * SQUARE_SIZE + offset.x / 2, GetRandomValue(0, (screenHeight / SQUARE_SIZE) - 1) * SQUARE_SIZE + offset.y / 2};
							i = 0;
						}
					}
				}

				// Collisione 3
				if ((snake[0].position.x < (fruit.position.x + fruit.size.x) && (snake[0].position.x + snake[0].size.x) > fruit.position.x) &&
					(snake[0].position.y < (fruit.position.y + fruit.size.y) && (snake[0].position.y + snake[0].size.y) > fruit.position.y))
				{
					snake[counterTail].position = snakePosition[counterTail - 1];
					PlaySound(eatSound);
					counterTail += 1;
					fruit.active = false;
					score = score + 8;
				}

				framesCounter++;
			}
		}
		else
		{
			if (IsKeyPressed(KEY_ENTER))
			{
				InitGame();
				gameOver = false;
			}
		}

		break;
	}
}

void DrawGame(void)
{
	BeginDrawing();

	ClearBackground(green);

	switch (modalita)
	{
	case MENU:
		DrawText(TextFormat("Click to start a game"), 190, 200, 40, darkGreen);

		DrawRectangleRec(bottoneStart.rectangle, bottoneStart.color);
		DrawText(bottoneStart.text, bottoneStart.rectangle.x + bottoneStart.rectangle.width / 2 - MeasureText(bottoneStart.text, 50) / 2, bottoneStart.rectangle.y + bottoneStart.rectangle.height / 2 - 25, 50, WHITE);
		break;

	case GIOCO:
		if (!gameOver)
		{
			DrawRectangleLinesEx((Rectangle){0, 0, 800, 690}, 2, darkGreen);

			// linee che formano la tabella
			for (int i = 0; i < screenWidth / SQUARE_SIZE + 1; i++)
			{
				DrawLineV((Vector2){SQUARE_SIZE * i + offset.x / 2, offset.y / 2}, (Vector2){SQUARE_SIZE * i + offset.x / 2, 700 - offset.y / 2}, LIGHTGRAY);
			}

			for (int i = 0; i < 700 / SQUARE_SIZE + 1; i++)
			{
				DrawLineV((Vector2){offset.x / 2, SQUARE_SIZE * i + offset.y / 2}, (Vector2){screenWidth - offset.x / 2, SQUARE_SIZE * i + offset.y / 2}, LIGHTGRAY);
			}

			for (int i = 0; i < counterTail; i++)
			{
				DrawTextureV(snake[i].im2, snake[i].position, snake[i].color);
			}

			if (fruit.active)
				DrawTexture(fruit.im1, fruit.position.x, fruit.position.y, fruit.color);

			DrawText(TextFormat("SCORE: %04i", score), 10, 700, 25, darkGreen);
			DrawText(TextFormat("Retro Game"), 400, 700, 25, darkGreen);

			if (pause)
				DrawText("GAME PAUSE", screenWidth / 2 - MeasureText("GAME PAUSE", 40) / 2, screenHeight / 2 - 40, 40, GRAY);
		}
		else
			DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth() / 2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20) / 2, GetScreenHeight() / 2 - 50, 20, GRAY);

		break;
	}

	EndDrawing();
}

void UnloadGame(void)
{
	UnloadTexture(fruit.im1);
	UnloadTexture(snake[0].im2);

	UnloadSound(gameoverSound);
}

void UpdateDrawFrame(void)
{
	UpdateGame();
	DrawGame();
}
