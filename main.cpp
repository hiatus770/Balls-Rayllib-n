#include <iostream>
#include <cmath>
#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <array>
#define WIDTHGAME 30
#define HEIGHTGAME 30
#define toDegree(x) (x * 180.0 / 3.14159)
#define toRadian(x) (x * 3.14159 / 180.0)
#define gameFPS 500
#define getDecimal(x) (x - (int)x)
#define sgn(x) (x < 0 ? -1 : 1)

#define gravCoef 10
#define dragCoef 0.999
#define elasticCoef 0.8
#define collisionCoef 0.4
#define subSteps 4

using namespace std;

const int screenWidth = 800;
const int screenHeight = 800;
const int maxWidth = screenWidth;
const int maxHeight = screenHeight;
const int minWidth = 0;
const int minHeight = 0;

int colorCounter = 0;

// Define some operators for vector2 that are missing such as + - * /
Vector2 operator+(const Vector2 &v1, const Vector2 &v2)
{
    return {v1.x + v2.x, v1.y + v2.y};
}
Vector2 operator-(const Vector2 &v1, const Vector2 &v2)
{
    return {v1.x - v2.x, v1.y - v2.y};
}
Vector2 operator*(const Vector2 &v1, const Vector2 &v2)
{
    return {v1.x * v2.x, v1.y * v2.y};
}
Vector2 operator/(const Vector2 &v1, const Vector2 &v2)
{
    return {v1.x / v2.x, v1.y / v2.y};
}
Vector2 operator*(const Vector2 &v1, const double &v2)
{
    return {v1.x * v2, v1.y * v2};
}
Vector2 operator/(const Vector2 &v1, const double &v2)
{
    return {v1.x / v2, v1.y / v2};
}

class QuadTree;

class Ball
{
public:
    Vector2 position;
    Vector2 lastPosition;
    Vector2 velocity;
    Vector2 acceleration = {0, 0.1};
    float radius;
    Color col = RED;

    Ball(float x, float y, float radius)
    {
        this->position.x = x;
        this->position.y = y;
        this->radius = radius;
        this->velocity.x = 0;
        this->velocity.y = 0;
        this->lastPosition.x = x;
        this->lastPosition.y = y;
    };

    Ball(float x, float y, Vector2 vel, float radius)
    {
        this->position.x = x;
        this->position.y = y;
        this->radius = radius;
        this->velocity = vel;
        this->lastPosition.x = x - vel.x;
        this->lastPosition.y = y - vel.y;
    };

    void update(float dt)
    {
        // Using dt
        Vector2 displacement = Vector2Subtract(position, lastPosition);
        lastPosition = position;
        position = position + displacement * dragCoef + acceleration * dt * dt;

        // Vector2 displacement = Vector2Subtract(position, lastPosition);
        // lastPosition = position;
        // position = position + displacement*dragCoef + acceleration;
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

    void collision();

    void setVelocity(Vector2 v)
    {
        lastPosition = position - v;
    }

    void getVelocity()
    {
        velocity = position - lastPosition;
    }
};

vector<Ball> globalBalls;


void Ball::collision()
{
    // Use space partitioning to find the balls that are close to this ball
    for (int i = 0; i < (int)globalBalls.size(); i++)
    {
        Ball *b = &globalBalls.at(i);
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

// Main function
int main()
{

    InitWindow(screenWidth, screenHeight, "Balluh");
    SetTargetFPS(gameFPS);

    float lastTime = 0;

    // test ball

    while (WindowShouldClose() == false)
    {
        BeginDrawing();
        ClearBackground(BLACK);

        for (int i = 0; i < (int)globalBalls.size(); i++)
        {
            Ball *b = &globalBalls.at(i);
            b->draw();
        }

        // add balls on mouse press
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Vector2 mousePos = GetMousePosition();
            // with velocity of the mouse delta
            globalBalls.push_back(Ball(mousePos.x, mousePos.y, GetMouseDelta()/5, 10));
        }

        if (GetTime() - lastTime > 0.01)
        {
            lastTime = GetTime();
            float dt = 1.0 / subSteps;
            for (int subStepCount = 0; subStepCount < subSteps; subStepCount++)
            {
                for (int i = 0; i < (int)globalBalls.size(); i++)
                {
                    Ball *b = &globalBalls.at(i);
                    b->collision();
                }
                for (int i = 0; i < (int)globalBalls.size(); i++)
                {
                    Ball *b = &globalBalls.at(i);
                    b->update(dt);
                }

                for (int i = 0; i < (int)globalBalls.size(); i++)
                {
                    Ball *b = &globalBalls.at(i);
                    b->bounds();
                }
            }
        }

        // Display the FPS currently
        DrawFPS(10, 10);
        EndDrawing();
    }
    CloseWindow();
    return 0;
};
