// 


#include <iostream>
#include <cmath>
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <array>
#include <rlgl.h>
#include <thread> 
#define WIDTHGAME 100
#define HEIGHTGAME 100
#define toDegree(x) (x * 180.0 / 3.14159)
#define toRadian(x) (x * 3.14159 / 180.0)
#define gameFPS 120
#define getDecimal(x) (x - (int)x)
#define sgn(x) (x < 0 ? -1 : 1)

#define gravCoef 10
#define dragCoef 0.999
#define elasticCoef 0.8
#define collisionCoef 0.4
#define subSteps 2
#define ballCount 5000 

using namespace std;

const int screenWidth = 1000;
const int screenHeight = 1000;
const int maxWidth = screenWidth;
const int maxHeight = screenHeight;
const int minWidth = 0;
const int minHeight = 0;
int globalBallsCount = 0; 
const int maxBallsPerGrid = 10;


#include "vectorOperators.h"

class Ball
{
public:
    Vector2 position;
    Vector2 lastPosition;
    Vector2 velocity;
    Vector2 acceleration = {0, 0.1};
    float radius;
    Color col = RED;

    Ball(){

    }

    Ball(float x, float y, float radius)
    {
        this->position.x = x;
        this->position.y = y;
        this->radius = radius;
        this->velocity.x = 0;
        this->velocity.y = 0;
        this->lastPosition.x = x;
        this->lastPosition.y = y;

        // Make rainbow balls gradually shift with globalBalls and a sine function
        int r = (int)(sin(globalBallsCount * 0.01) * 127 + 128);
        int g = (int)(sin(globalBallsCount * 0.01 + 2) * 127 + 128);
        int b = (int)(sin(globalBallsCount * 0.01 + 4) * 127 + 128);
        col = {r, g, b, 255};
    };

    Ball(float x, float y, Vector2 vel, float radius)
    {
        this->position.x = x;
        this->position.y = y;
        this->radius = radius;
        this->velocity = vel;
        this->lastPosition.x = x - vel.x;
        this->lastPosition.y = y - vel.y;
        int r = (int)(sin(globalBallsCount * 0.01) * 127 + 128);
        int g = (int)(sin(globalBallsCount * 0.01 + 2) * 127 + 128);
        int b = (int)(sin(globalBallsCount * 0.01 + 4) * 127 + 128);
        col = {r, g, b, 255};
    };

    void update(float dt)
    {
        // Using dt
        Vector2 displacement = Vector2Subtract(position, lastPosition);
        lastPosition = position;
        position = position + displacement * dragCoef + acceleration * dt * dt;
    }

    void bounds()
    {
        getVelocity();
        if (position.x + radius > maxWidth)
        {
            position.x = maxWidth - radius;
            lastPosition.x = position.x + velocity.x * elasticCoef;
        }
        if (position.x - radius < minWidth)
        {
            position.x = minWidth + radius;
            lastPosition.x = position.x + velocity.x * elasticCoef;
        }
        if (position.y + radius > maxHeight)
        {
            position.y = maxHeight - radius;
            lastPosition.y = position.y + velocity.y * elasticCoef;
        }
        if (position.y - radius < minHeight)
        {
            position.y = minHeight + radius;
            lastPosition.y = position.y + velocity.y * elasticCoef;
        }
    }

    void draw()
    {
        DrawCircle(position.x, position.y, radius, col);
    }

    void collision(int debug);

    void setVelocity(Vector2 v)
    {
        lastPosition = position - v;
    }

    void getVelocity()
    {
        velocity = position - lastPosition;
    }
};

Ball globalBalls[ballCount];

void ballCoordToGrid(Ball *b, int *x, int *y);

// The 2d array of the grid
Ball* grid[WIDTHGAME][HEIGHTGAME][maxBallsPerGrid];
int gridCounts [WIDTHGAME][HEIGHTGAME]; // Will keep track of the number of balls in each grid

/**
 *  @brief Collision function for balls 
 *  @return void
 */
void Ball::collision(int debug = 0)
{
    int x;
    int y;
    ballCoordToGrid(this, &x, &y);
    vector<Ball*> closeBalls;
    
    for (int i = -1; i <= 1; i++)
    {
        if (x + i >= 0 && x + i < WIDTHGAME)
        {
            for (int j = -1; j <= 1; j++)
            {
                if (y + j >= 0 && y + j < HEIGHTGAME)
                {
                    for (int k = 0; k < gridCounts[x + i][y + j]; k++)
                    {
                        closeBalls.push_back(grid[x + i][y + j][k]);
                    }
                }
            }
        }
    }


    // Use space partitioning to find the balls that are close to this ball
    for (int i = 0; i < (int)closeBalls.size(); i++)
    {
        Ball *b = closeBalls.at(i);
        // Draw a line from the center of this ball to the center of the other ball
        if (debug == 1)
        {
            DrawLineEx(position, b->position, 1, RED);
        }

        if (b != this)
        {
            float length = sqrtf((b->position.x - position.x) * (b->position.x - position.x) + (b->position.y - position.y) * (b->position.y - position.y));
            if (b->radius + radius > length && length != 0)
            {
                float overlap = b->radius + radius - length;
                if (overlap > 0)
                {
                    float dx;
                    float dy;
                    float ang;
                    if (position.x - b->position.x == 0)
                    {
                        dx = 0;
                        dy = overlap / 2;
                    }
                    else
                    {
                        ang = atan((position.y - b->position.y) / (position.x - b->position.x));
                        dx = abs(cos(ang) * overlap / 2);
                        dy = abs(sin(ang) * overlap / 2);
                    }
                    position.x += -1 * sgn(b->position.x - position.x) * dx;
                    position.y += -1 * sgn(b->position.y - position.y) * dy;
                    b->position.x += sgn(b->position.x - position.x) * dx;
                    b->position.y += sgn(b->position.y - position.y) * dy;
                }
            }
        }
    }
}

void UpdateBallPositionsRange(int start, int end, float dt){
    for (int i = start; i < end; i++)
    {
        Ball *b = &globalBalls[i];
        b->update(dt);
    }
}

void CollideBallPositionsRange(int start, int end, int debug){
    for (int i = start; i < end; i++)
    {
        Ball *b = &globalBalls[i];
        b->collision(debug);
    }
}

void ballCoordToGrid(Ball *b, int *x, int *y)
{
    *x = (int)(b->position.x / (maxWidth / WIDTHGAME));
    *y = (int)(b->position.y / (maxHeight / HEIGHTGAME));
}

/**
 * @brief  
 * 
 */
void updateGrid(){
    // Clear the grid
    for (int i = 0; i < WIDTHGAME; i++)
    {
        for (int j = 0; j < HEIGHTGAME; j++)
        {
            grid[i][j][0] = NULL;
            gridCounts[i][j] = 0;
        }
    }
    // Push all the necessary balls into the grid
    for (int i = 0; i < globalBallsCount; i++)
    {
        Ball *b = &globalBalls[i];
        int x;
        int y;
        ballCoordToGrid(b, &x, &y);
        grid[x][y][gridCounts[x][y]] = b;
        gridCounts[x][y]++;
    }
}

void drawGrid (){
    for (int i = 0; i <= WIDTHGAME; i++)
    {
        for (int j = 0; j <= HEIGHTGAME; j++)
        {
            DrawRectangleLines(i * (maxWidth / WIDTHGAME), j * (maxHeight / HEIGHTGAME), maxWidth / WIDTHGAME, maxHeight / HEIGHTGAME, WHITE);
            DrawText(TextFormat("%d", i + j * WIDTHGAME), i * (maxWidth / WIDTHGAME), j * (maxHeight / HEIGHTGAME), 10, WHITE);
        }
    }
}

// Main function
int main()
{

    InitWindow(screenWidth, screenHeight, "Balluh");
    SetTargetFPS(gameFPS);

    Camera2D cam = {0};
    cam.zoom = 1; 

    float lastTime = 0;

    // test ball

    float computationTime = 0;
    float drawTime = 0;
    float lastComputationTime = 0;
    float lastDrawTime = 0; 

    while (WindowShouldClose() == false)
    {
        
        // Mouse zoom stuff 
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)){
            Vector2 delta = GetMouseDelta();
            delta = Vector2Scale(delta, -1.0f / cam.zoom);
            cam.target = Vector2Add(cam.target, delta);
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0)
        {
            Vector2 mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), cam);
            cam.offset = GetMousePosition(); 
            cam.target = mouseWorldPos;
            cam.zoom += wheel * 0.5f;
            if (cam.zoom < 0.1f)
                cam.zoom = 0.1f;
        }

        drawTime = GetTime();
        // Drawing begins here 
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(cam);
        for (int i = 0; i < globalBallsCount; i++)
        {
            Ball *b = &globalBalls[i];
            // draw last 10 obejcts for testing
            DrawCircleV(b->position, b->radius, b->col);

        }
        EndMode2D();
        drawTime = GetTime() - drawTime;
        lastDrawTime = drawTime;
        
        // Display the FPS currently
        DrawFPS(10, 10);
        // Draw ball count
        DrawText(TextFormat("Ball Count: %d", globalBallsCount), 10, 30, 10, WHITE); 
        DrawText(TextFormat("Computation Time: %f", lastComputationTime), 10, 50, 10, WHITE);
        DrawText(TextFormat("Draw Time: %f", lastDrawTime), 10, 70, 10, WHITE);
        EndDrawing(); 
        
        Vector2 mousePos = GetMousePosition();
        // Add balls on mouse pressed 
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && globalBallsCount < ballCount && mousePos.x > 0 && mousePos.x < maxWidth && mousePos.y > 0 && mousePos.y < maxHeight && GetTime() - lastTime > 0.001) 
        {
            // Array of glboal balls not a vector anymore
            globalBalls[globalBallsCount] = Ball(100, 100, {10, 1}, 5); 
            globalBallsCount++;
        }

        
        // Update at a constant update rate 
        if (GetTime() - lastTime > 0.001)
        {
        
            computationTime = GetTime();
            lastTime = GetTime();
            float dt = 1.0 / subSteps;
            for (int subStepCount = 0; subStepCount < subSteps; subStepCount++)
            {
                updateGrid(); 
                for (int i = 0; i < globalBallsCount; i++)
                {
                    Ball *b = &globalBalls[i];
                    b->collision();
                }
                for (int i = 0; i < globalBallsCount; i++)
                {
                    Ball *b = &globalBalls[i];
                    b->update(dt);
                }
                for (int i = 0; i < globalBallsCount; i++)
                {
                    Ball *b = &globalBalls[i];
                    b->bounds();
                }
            }
            computationTime = GetTime() - computationTime;
            lastComputationTime = computationTime;
        }

    }
    CloseWindow();
    return 0;
};
