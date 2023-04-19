#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

//----------------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------------
#define SQUARE_SIZE             20

#define GRID_HORIZONTAL_SIZE    12
#define GRID_VERTICAL_SIZE      20

#define LATERAL_SPEED           10
#define TURNING_SPEED           12
#define FAST_FALL_AWAIT_COUNTER 30

#define FADING_TIME             33

//----------------------------------------------------------------------------------
// 
//----------------------------------------------------------------------------------
typedef enum GridSquare { EMPTY, MOVING, FULL, BLOCK, FADING } GridSquare;

//------------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 450;

static bool gameOver = false;
static bool pause = false;


static GridSquare grid [GRID_HORIZONTAL_SIZE][GRID_VERTICAL_SIZE];
static GridSquare piece [4][4];
static GridSquare incomingPiece [4][4];


static int piecePositionX = 0;
static int piecePositionY = 0;


static Color fadingColor;


static bool beginPlay = true;      
static bool pezzo_attivo = false;
static bool rilevamento = false;
static bool lineToDelete = false;


static int livello = 1;
static int linea = 0;


static int gravityMovementCounter = 0;
static int lateralMovementCounter = 0;
static int turnMovementCounter = 0;
static int fastFallMovementCounter = 0;

static int fadeLineCounter = 0;


static int velocita= 30;

//------------------------------------------------------------------------------------
// Module Functions Declaration (local)
//------------------------------------------------------------------------------------
static void InitGame(void);         // Initialize game
static void UpdateGame(void);       // Update game (one frame)
static void DrawGame(void);         // Draw game (one frame)
static void UnloadGame(void);       // Unload game
static void UpdateDrawFrame(void);  // Update and Draw (one frame)


static bool Crea_pezzo();
static void pezzo_casuale();
static void movimento_caduta(bool *rilevamento, bool *pezzo_attivo);
static bool movimento_laterale();
static bool Movimento_Turno();
static void Controlla_rilevamento(bool *rilevamento);
static void Verifica_completamento(bool *lineToDelete);
static int elimina_linea();

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    
    InitWindow(screenWidth, screenHeight, " tetris");

    InitGame();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        
        UpdateDrawFrame();
        
    }
#endif
    
    UnloadGame();        

    CloseWindow();        
    
    return 0;
}

//--------------------------------------------------------------------------------------
// Game Module Functions Definition
//--------------------------------------------------------------------------------------


void InitGame(void)
{
    
    livello = 1;
    linea = 0;

    fadingColor = GRAY;

    piecePositionX = 0;
    piecePositionY = 0;

    pause = false;

    beginPlay = true;
    pezzo_attivo = false;
    rilevamento = false;
    lineToDelete = false;

    
    gravityMovementCounter = 0;
    lateralMovementCounter = 0;
    turnMovementCounter = 0;
    fastFallMovementCounter = 0;

    fadeLineCounter = 0;
    velocita= 30;

  
    for (int i = 0; i < GRID_HORIZONTAL_SIZE; i++)
    {
        for (int j = 0; j < GRID_VERTICAL_SIZE; j++)
        {
            if ((j == GRID_VERTICAL_SIZE - 1) || (i == 0) || (i == GRID_HORIZONTAL_SIZE - 1)) grid[i][j] = BLOCK;
            else grid[i][j] = EMPTY;
        }
    }

    
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j< 4; j++)
        {
            incomingPiece[i][j] = EMPTY;
        }
    }
}


void UpdateGame(void)
{
    if (!gameOver)
    {
        if (IsKeyPressed('P')) pause = !pause;

        if (!pause)
        {
            if (!lineToDelete)
            {
                if (!pezzo_attivo)
                {
                   
                    pezzo_attivo = Crea_pezzo();

                    
                    fastFallMovementCounter = 0;
                }
                else    
                {
                    
                    fastFallMovementCounter++;
                    gravityMovementCounter++;
                    lateralMovementCounter++;
                    turnMovementCounter++;

                    
                    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT)) lateralMovementCounter = LATERAL_SPEED;
                    if (IsKeyPressed(KEY_UP)) turnMovementCounter = TURNING_SPEED;

                   
                    if (IsKeyDown(KEY_DOWN) && (fastFallMovementCounter >= FAST_FALL_AWAIT_COUNTER))
                    {
                        
                        gravityMovementCounter += velocita;
                    }

                    if (gravityMovementCounter >= velocita)
                    {
                        
                        Controlla_rilevamento(&rilevamento);

                        
                        movimento_caduta(&rilevamento, &pezzo_attivo);

                        
                        Verifica_completamento(&lineToDelete);

                        gravityMovementCounter = 0;
                    }

                    
                    if (lateralMovementCounter >= LATERAL_SPEED)
                    {
                        
                        if (!movimento_laterale()) lateralMovementCounter = 0;
                    }

                    
                    if (turnMovementCounter >= TURNING_SPEED)
                    {
                       
                        if (Movimento_Turno()) turnMovementCounter = 0;
                    }
                }

                
                for (int j = 0; j < 2; j++)
                {
                    for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
                    {
                        if (grid[i][j] == FULL)
                        {
                            gameOver = true;
                        }
                    }
                }
            }
            else
            {
                
                fadeLineCounter++;

                if (fadeLineCounter%8 < 4) fadingColor = MAROON;
                else fadingColor = GRAY;

                if (fadeLineCounter >= FADING_TIME)
                {
                    int deletedLinea = 0;
                    deletedLinea = elimina_linea();
                    fadeLineCounter = 0;
                    lineToDelete = false;

                    linea += deletedLinea;
                }
            }
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
}

// Draw game 
void DrawGame(void)
{
    BeginDrawing();

        ClearBackground(RAYWHITE);

        if (!gameOver)
        {
            // Draw gameplay area
            Vector2 offset;
            offset.x = screenWidth/2 - (GRID_HORIZONTAL_SIZE*SQUARE_SIZE/2) - 50;
            offset.y = screenHeight/2 - ((GRID_VERTICAL_SIZE - 1)*SQUARE_SIZE/2) + SQUARE_SIZE*2;

            offset.y -= 50;     // NOTE: Harcoded position!

            int controller = offset.x;

            for (int j = 0; j < GRID_VERTICAL_SIZE; j++)
            {
                for (int i = 0; i < GRID_HORIZONTAL_SIZE; i++)
                {
                    // Draw each square of the grid
                    if (grid[i][j] == EMPTY)
                    {
                        DrawLine(offset.x, offset.y, offset.x + SQUARE_SIZE, offset.y, LIGHTGRAY );
                        DrawLine(offset.x, offset.y, offset.x, offset.y + SQUARE_SIZE, RED );
                        DrawLine(offset.x + SQUARE_SIZE, offset.y, offset.x + SQUARE_SIZE, offset.y + SQUARE_SIZE, LIGHTGRAY );
                        DrawLine(offset.x, offset.y + SQUARE_SIZE, offset.x + SQUARE_SIZE, offset.y + SQUARE_SIZE, LIGHTGRAY );
                        offset.x += SQUARE_SIZE;
                    }
                    else if (grid[i][j] == FULL)
                    {
                        DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, GRAY);
                        offset.x += SQUARE_SIZE;
                    }
                    else if (grid[i][j] == MOVING)
                    {
                        DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, DARKGRAY);
                        offset.x += SQUARE_SIZE;
                    }
                    else if (grid[i][j] == BLOCK)
                    {
                        DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, LIGHTGRAY);
                        offset.x += SQUARE_SIZE;
                    }
                    else if (grid[i][j] == FADING)
                    {
                        DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, fadingColor);
                        offset.x += SQUARE_SIZE;
                    }
                }

                offset.x = controller;
                offset.y += SQUARE_SIZE;
            }

            
            offset.x = 500;
            offset.y = 45;

		//Prossimo pezzo
            int controler = offset.x;

            for (int j = 0; j < 4; j++)
            {
                for (int i = 0; i < 4; i++)
                {
                    if (incomingPiece[i][j] == EMPTY)
                    {
                        DrawLine(offset.x, offset.y, offset.x + SQUARE_SIZE, offset.y, LIGHTGRAY );
                        DrawLine(offset.x, offset.y, offset.x, offset.y + SQUARE_SIZE, LIGHTGRAY );
                        DrawLine(offset.x + SQUARE_SIZE, offset.y, offset.x + SQUARE_SIZE, offset.y + SQUARE_SIZE, LIGHTGRAY );
                        DrawLine(offset.x, offset.y + SQUARE_SIZE, offset.x + SQUARE_SIZE, offset.y + SQUARE_SIZE, LIGHTGRAY );
                        offset.x += SQUARE_SIZE;
                    }
                    else if (incomingPiece[i][j] == MOVING)
                    {
                        DrawRectangle(offset.x, offset.y, SQUARE_SIZE, SQUARE_SIZE, GRAY);
                        offset.x += SQUARE_SIZE;
                    }
                }

                offset.x = controler;
                offset.y += SQUARE_SIZE;
            }

            DrawText("INCOMING:", offset.x, offset.y - 100, 10, GRAY);
            DrawText(TextFormat("LINEA:      %04i", linea), offset.x, offset.y + 20, 10, GRAY);

            if (pause) DrawText("GAME PAUSED", screenWidth/2 - MeasureText("GAME PAUSED", 40)/2, screenHeight/2 - 40, 40, GRAY);
        }
        else DrawText("PRESS [ENTER] TO PLAY AGAIN", GetScreenWidth()/2 - MeasureText("PRESS [ENTER] TO PLAY AGAIN", 20)/2, GetScreenHeight()/2 - 50, 20, GRAY);

    EndDrawing();
}

// Unload game variables
void UnloadGame(void)
{
    // TODO: Unload all dynamic loaded data (textures, sounds, models...)
}

// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    UpdateGame();
    DrawGame();
}

//--------------------------------------------------------------------------------------
// Additional module functions
//--------------------------------------------------------------------------------------
static bool Crea_pezzo()
{
    piecePositionX = (int)((GRID_HORIZONTAL_SIZE - 4)/2);
    piecePositionY = 0;

    // If the game is starting and you are going to create the first piece, we create an extra one
    if (beginPlay)
    {
        pezzo_casuale();
        beginPlay = false;
    }

    // We assign the incoming piece to the actual piece
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j< 4; j++)
        {
            piece[i][j] = incomingPiece[i][j];
        }
    }

    // We assign a random piece to the incoming one
    pezzo_casuale();

    // Assign the piece to the grid
    for (int i = piecePositionX; i < piecePositionX + 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            if (piece[i - (int)piecePositionX][j] == MOVING) grid[i][j] = MOVING;
        }
    }

    return true;
}

static void pezzo_casuale()
{
    int random = GetRandomValue(0, 6);

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            incomingPiece[i][j] = EMPTY;
        }
    }

    switch (random)
    {
        case 0: { incomingPiece[1][1] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[1][2] = MOVING; incomingPiece[2][2] = MOVING; } break;    //Cube
        case 1: { incomingPiece[1][0] = MOVING; incomingPiece[1][1] = MOVING; incomingPiece[1][2] = MOVING; incomingPiece[2][2] = MOVING; } break;    //L
        case 2: { incomingPiece[1][2] = MOVING; incomingPiece[2][0] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[2][2] = MOVING; } break;    //L inversa
        case 3: { incomingPiece[0][1] = MOVING; incomingPiece[1][1] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[3][1] = MOVING; } break;    //Recta
        case 4: { incomingPiece[1][0] = MOVING; incomingPiece[1][1] = MOVING; incomingPiece[1][2] = MOVING; incomingPiece[2][1] = MOVING; } break;    //Creu tallada
        case 5: { incomingPiece[1][1] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[2][2] = MOVING; incomingPiece[3][2] = MOVING; } break;    //S
        case 6: { incomingPiece[1][2] = MOVING; incomingPiece[2][2] = MOVING; incomingPiece[2][1] = MOVING; incomingPiece[3][1] = MOVING; } break;    //S inversa
    }
}

static void movimento_caduta(bool *rilevamento, bool *pezzo_attivo)
{
    // If we finished moving this piece, we stop it
    if (*rilevamento)
    {
        for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
        {
            for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
            {
                if (grid[i][j] == MOVING)
                {
                    grid[i][j] = FULL;
                    *rilevamento = false;
                    *pezzo_attivo = false;
                }
            }
        }
    }
    else    // We move down the piece
    {
        for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
        {
            for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
            {
                if (grid[i][j] == MOVING)
                {
                    grid[i][j+1] = MOVING;
                    grid[i][j] = EMPTY;
                }
            }
        }

        piecePositionY++;
    }
}

static bool movimento_laterale()
{
    bool collision = false;

    // Piece movement
    if (IsKeyDown(KEY_LEFT))        // Move left
    {
        // Check if is possible to move to left
        for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
        {
            for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
            {
                if (grid[i][j] == MOVING)
                {
                    // Check if we are touching the left wall or we have a full square at the left
                    if ((i-1 == 0) || (grid[i-1][j] == FULL)) collision = true;
                }
            }
        }

        // If able, move left
        if (!collision)
        {
            for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
            {
                for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)             // We check the matrix from left to right
                {
                    // Move everything to the left
                    if (grid[i][j] == MOVING)
                    {
                        grid[i-1][j] = MOVING;
                        grid[i][j] = EMPTY;
                    }
                }
            }

            piecePositionX--;
        }
    }
    else if (IsKeyDown(KEY_RIGHT))  // Move right
    {
        // Check if is possible to move to right
        for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
        {
            for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
            {
                if (grid[i][j] == MOVING)
                {
                    // Check if we are touching the right wall or we have a full square at the right
                    if ((i+1 == GRID_HORIZONTAL_SIZE - 1) || (grid[i+1][j] == FULL))
                    {
                        collision = true;

                    }
                }
            }
        }

        // If able move right
        if (!collision)
        {
            for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
            {
                for (int i = GRID_HORIZONTAL_SIZE - 1; i >= 1; i--)             // We check the matrix from right to left
                {
                    // Move everything to the right
                    if (grid[i][j] == MOVING)
                    {
                        grid[i+1][j] = MOVING;
                        grid[i][j] = EMPTY;
                    }
                }
            }

            piecePositionX++;
        }
    }

    return collision;
}

static bool Movimento_Turno()
{
    // Input for turning the piece
    if (IsKeyDown(KEY_UP))
    {
        GridSquare aux;
        bool checker = false;

        // Check all turning possibilities
        if ((grid[piecePositionX + 3][piecePositionY] == MOVING) &&
            (grid[piecePositionX][piecePositionY] != EMPTY) &&
            (grid[piecePositionX][piecePositionY] != MOVING)) checker = true;

        if ((grid[piecePositionX + 3][piecePositionY + 3] == MOVING) &&
            (grid[piecePositionX + 3][piecePositionY] != EMPTY) &&
            (grid[piecePositionX + 3][piecePositionY] != MOVING)) checker = true;

        if ((grid[piecePositionX][piecePositionY + 3] == MOVING) &&
            (grid[piecePositionX + 3][piecePositionY + 3] != EMPTY) &&
            (grid[piecePositionX + 3][piecePositionY + 3] != MOVING)) checker = true;

        if ((grid[piecePositionX][piecePositionY] == MOVING) &&
            (grid[piecePositionX][piecePositionY + 3] != EMPTY) &&
            (grid[piecePositionX][piecePositionY + 3] != MOVING)) checker = true;

        if ((grid[piecePositionX + 1][piecePositionY] == MOVING) &&
            (grid[piecePositionX][piecePositionY + 2] != EMPTY) &&
            (grid[piecePositionX][piecePositionY + 2] != MOVING)) checker = true;

        if ((grid[piecePositionX + 3][piecePositionY + 1] == MOVING) &&
            (grid[piecePositionX + 1][piecePositionY] != EMPTY) &&
            (grid[piecePositionX + 1][piecePositionY] != MOVING)) checker = true;

        if ((grid[piecePositionX + 2][piecePositionY + 3] == MOVING) &&
            (grid[piecePositionX + 3][piecePositionY + 1] != EMPTY) &&
            (grid[piecePositionX + 3][piecePositionY + 1] != MOVING)) checker = true;

        if ((grid[piecePositionX][piecePositionY + 2] == MOVING) &&
            (grid[piecePositionX + 2][piecePositionY + 3] != EMPTY) &&
            (grid[piecePositionX + 2][piecePositionY + 3] != MOVING)) checker = true;

        if ((grid[piecePositionX + 2][piecePositionY] == MOVING) &&
            (grid[piecePositionX][piecePositionY + 1] != EMPTY) &&
            (grid[piecePositionX][piecePositionY + 1] != MOVING)) checker = true;

        if ((grid[piecePositionX + 3][piecePositionY + 2] == MOVING) &&
            (grid[piecePositionX + 2][piecePositionY] != EMPTY) &&
            (grid[piecePositionX + 2][piecePositionY] != MOVING)) checker = true;

        if ((grid[piecePositionX + 1][piecePositionY + 3] == MOVING) &&
            (grid[piecePositionX + 3][piecePositionY + 2] != EMPTY) &&
            (grid[piecePositionX + 3][piecePositionY + 2] != MOVING)) checker = true;

        if ((grid[piecePositionX][piecePositionY + 1] == MOVING) &&
            (grid[piecePositionX + 1][piecePositionY + 3] != EMPTY) &&
            (grid[piecePositionX + 1][piecePositionY + 3] != MOVING)) checker = true;

        if ((grid[piecePositionX + 1][piecePositionY + 1] == MOVING) &&
            (grid[piecePositionX + 1][piecePositionY + 2] != EMPTY) &&
            (grid[piecePositionX + 1][piecePositionY + 2] != MOVING)) checker = true;

        if ((grid[piecePositionX + 2][piecePositionY + 1] == MOVING) &&
            (grid[piecePositionX + 1][piecePositionY + 1] != EMPTY) &&
            (grid[piecePositionX + 1][piecePositionY + 1] != MOVING)) checker = true;

        if ((grid[piecePositionX + 2][piecePositionY + 2] == MOVING) &&
            (grid[piecePositionX + 2][piecePositionY + 1] != EMPTY) &&
            (grid[piecePositionX + 2][piecePositionY + 1] != MOVING)) checker = true;

        if ((grid[piecePositionX + 1][piecePositionY + 2] == MOVING) &&
            (grid[piecePositionX + 2][piecePositionY + 2] != EMPTY) &&
            (grid[piecePositionX + 2][piecePositionY + 2] != MOVING)) checker = true;

        if (!checker)
        {
            aux = piece[0][0];
            piece[0][0] = piece[3][0];
            piece[3][0] = piece[3][3];
            piece[3][3] = piece[0][3];
            piece[0][3] = aux;

            aux = piece[1][0];
            piece[1][0] = piece[3][1];
            piece[3][1] = piece[2][3];
            piece[2][3] = piece[0][2];
            piece[0][2] = aux;

            aux = piece[2][0];
            piece[2][0] = piece[3][2];
            piece[3][2] = piece[1][3];
            piece[1][3] = piece[0][1];
            piece[0][1] = aux;

            aux = piece[1][1];
            piece[1][1] = piece[2][1];
            piece[2][1] = piece[2][2];
            piece[2][2] = piece[1][2];
            piece[1][2] = aux;
        }

        for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
        {
            for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
            {
                if (grid[i][j] == MOVING)
                {
                    grid[i][j] = EMPTY;
                }
            }
        }

        for (int i = piecePositionX; i < piecePositionX + 4; i++)
        {
            for (int j = piecePositionY; j < piecePositionY + 4; j++)
            {
                if (piece[i - piecePositionX][j - piecePositionY] == MOVING)
                {
                    grid[i][j] = MOVING;
                }
            }
        }

        return true;
    }

    return false;
}

static void Controlla_rilevamento(bool *rilevamento)
{
    for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
    {
        for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
        {
            if ((grid[i][j] == MOVING) && ((grid[i][j+1] == FULL) || (grid[i][j+1] == BLOCK))) *rilevamento = true;
        }
    }
}

static void Verifica_completamento(bool *lineToDelete)
{
    int calculator = 0;

    for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
    {
        calculator = 0;
        for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
        {
            // Count each square of the line
            if (grid[i][j] == FULL)
            {
                calculator++;
            }

            // Check if we completed the whole line
            if (calculator == GRID_HORIZONTAL_SIZE - 2)
            {
                *lineToDelete = true;
                calculator = 0;
                // points++;

                // Mark the completed line
                for (int z = 1; z < GRID_HORIZONTAL_SIZE - 1; z++)
                {
                    grid[z][j] = FADING;
                }
            }
        }
    }
}

static int elimina_linea()
{
    int deletedLinea = 0;

    // Erase the completed line
    for (int j = GRID_VERTICAL_SIZE - 2; j >= 0; j--)
    {
        while (grid[1][j] == FADING)
        {
            for (int i = 1; i < GRID_HORIZONTAL_SIZE - 1; i++)
            {
                grid[i][j] = EMPTY;
            }

            for (int j2 = j-1; j2 >= 0; j2--)
            {
                for (int i2 = 1; i2 < GRID_HORIZONTAL_SIZE - 1; i2++)
                {
                    if (grid[i2][j2] == FULL)
                    {
                        grid[i2][j2+1] = FULL;
                        grid[i2][j2] = EMPTY;
                    }
                    else if (grid[i2][j2] == FADING)
                    {
                        grid[i2][j2+1] = FADING;
                        grid[i2][j2] = EMPTY;
                    }
                }
            }

             deletedLinea++;
        }
    }

    return deletedLinea;
}
