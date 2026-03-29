#include "raylib.h"
#include <stdlib.h> // 用于 rand() 和 srand()
#include <time.h>   // 用于 time()

// --- 1. 定义常量和数据结构 ---
#define LINES_OF_BRICKS 5
#define BRICKS_PER_LINE 10
#define MAX_POWERUPS 5 // 屏幕上同时存在的最大技能球数量

typedef enum { SPEED_UP, WIDER_PADDLE, EXTRA_LIFE, SLOW_BALL } PowerUpType;

struct PowerUp {
    Vector2 position;
    Vector2 speed;
    PowerUpType type;
    bool active;
    float radius;
};

struct Paddle {
    Vector2 position;
    Vector2 size;
    float speed;
    Color color;
};

struct Ball {
    Vector2 position;
    Vector2 speed;
    float radius;
    Color color;
};

struct Brick {
    Vector2 position;
    Vector2 size;
    bool active;
    Color color;
};

int main() {
    // --- 2. 初始化 ---
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Breakout - V5.0 (Skills & Speed)");
    
    srand(time(NULL)); // 初始化随机数种子

    int lives = 3;
    int currentLevel = 1;
    bool gameOver = false;
    bool gameWon = false;
    bool initLevel = true;

    Paddle player = { 0 };
    player.size = { 100, 20 };
    player.speed = 8.0f;
    player.color = DARKGRAY;

    Ball ball = { 0 };
    ball.radius = 10.0f;
    ball.color = MAROON;

    Brick bricks[LINES_OF_BRICKS][BRICKS_PER_LINE] = { 0 };
    PowerUp powerUps[MAX_POWERUPS] = { 0 };
    for (int i = 0; i < MAX_POWERUPS; i++) powerUps[i].active = false;

    Color rowColors[5] = { RED, ORANGE, YELLOW, GREEN, BLUE };

    SetTargetFPS(60);

    // --- 3. 游戏主循环 ---
    while (!WindowShouldClose()) {
        
        // 关卡初始化
        if (initLevel) {
            player.position = { screenWidth / 2.0f - player.size.x / 2.0f, screenHeight - 40.0f };
            player.size.x = 100; // 每一关开始重置挡板长度
            
            ball.position = { screenWidth / 2.0f, screenHeight / 2.0f };
            // --- 核心修改：球速随关卡增加 ---
            float baseSpeed = 3.0f + (currentLevel * 1.5f); 
            ball.speed = { baseSpeed, -baseSpeed };

            int activeRows = currentLevel + 2; 
            for (int i = 0; i < LINES_OF_BRICKS; i++) {
                for (int j = 0; j < BRICKS_PER_LINE; j++) {
                    bricks[i][j].position = { (float)j * (screenWidth/BRICKS_PER_LINE) + 2, (float)i * 24 + 50 };
                    bricks[i][j].size = { (float)screenWidth/BRICKS_PER_LINE - 4, 20 };
                    bricks[i][j].active = (i < activeRows);
                    bricks[i][j].color = rowColors[i % 5];
                }
            }
            // 清除上一关留下的技能球
            for (int i = 0; i < MAX_POWERUPS; i++) powerUps[i].active = false;
            initLevel = false;
        }

        if (!gameOver && !gameWon) {
            // 1. 玩家控制
            if (IsKeyDown(KEY_LEFT)) player.position.x -= player.speed;
            if (IsKeyDown(KEY_RIGHT)) player.position.x += player.speed;
            if (player.position.x <= 0) player.position.x = 0;
            if (player.position.x + player.size.x >= screenWidth) player.position.x = screenWidth - player.size.x;

            // 2. 球体运动
            ball.position.x += ball.speed.x;
            ball.position.y += ball.speed.y;

            if ((ball.position.x >= (screenWidth - ball.radius)) || (ball.position.x <= ball.radius)) ball.speed.x *= -1.0f;
            if (ball.position.y <= ball.radius) ball.speed.y *= -1.0f;

            // 死亡判定
            if (ball.position.y >= screenHeight + ball.radius) {
                lives--;
                if (lives > 0) {
                    ball.position = { screenWidth / 2.0f, screenHeight / 2.0f };
                    float s = 3.0f + (currentLevel * 1.5f);
                    ball.speed = { s, -s };
                } else gameOver = true;
            }

            // 3. 碰撞检测：球与挡板
            if (CheckCollisionCircleRec(ball.position, ball.radius, {player.position.x, player.position.y, player.size.x, player.size.y})) {
                if (ball.speed.y > 0) {
                    ball.speed.y *= -1.0f;
                    ball.speed.x = (ball.position.x - (player.position.x + player.size.x/2)) / player.size.x * 10.0f;
                }
            }

            // 4. 碰撞检测：球与砖块
            int activeBricks = 0;
            for (int i = 0; i < LINES_OF_BRICKS; i++) {
                for (int j = 0; j < BRICKS_PER_LINE; j++) {
                    if (bricks[i][j].active) {
                        activeBricks++;
                        if (CheckCollisionCircleRec(ball.position, ball.radius, {bricks[i][j].position.x, bricks[i][j].position.y, bricks[i][j].size.x, bricks[i][j].size.y})) {
                            bricks[i][j].active = false;
                            ball.speed.y *= -1.0f;

                            // --- 核心修改：技能球掉落概率 (20% 概率) ---
                            if (rand() % 100 < 20) {
                                for (int p = 0; p < MAX_POWERUPS; p++) {
                                    if (!powerUps[p].active) {
                                        powerUps[p].active = true;
                                        powerUps[p].position = bricks[i][j].position;
                                        powerUps[p].speed = { 0, 3.0f };
                                        powerUps[p].radius = 12.0f;
                                        powerUps[p].type = (PowerUpType)(rand() % 4); // 随机选一个技能
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // 5. 更新技能球逻辑
            for (int i = 0; i < MAX_POWERUPS; i++) {
                if (powerUps[i].active) {
                    powerUps[i].position.y += powerUps[i].speed.y;

                    // 检测技能球与挡板碰撞
                    if (CheckCollisionCircleRec(powerUps[i].position, powerUps[i].radius, {player.position.x, player.position.y, player.size.x, player.size.y})) {
                        powerUps[i].active = false;
                        // 应用技能效果
                        switch (powerUps[i].type) {
                            case SPEED_UP: ball.speed.x *= 1.2f; ball.speed.y *= 1.2f; break;
                            case WIDER_PADDLE: player.size.x += 30; break;
                            case EXTRA_LIFE: lives++; break;
                            case SLOW_BALL: ball.speed.x *= 0.8f; ball.speed.y *= 0.8f; break;
                        }
                    }
                    // 掉出屏幕清理
                    if (powerUps[i].position.y > screenHeight) powerUps[i].active = false;
                }
            }

            if (activeBricks == 0) {
                currentLevel++;
                if (currentLevel > 3) gameWon = true;
                else initLevel = true;
            }
        } else {
            if (IsKeyPressed(KEY_ENTER)) {
                lives = 3; currentLevel = 1; gameOver = false; gameWon = false; initLevel = true;
            }
        }

        // --- 4. 绘制 ---
        BeginDrawing();
            ClearBackground(RAYWHITE);
            if (!gameOver && !gameWon) {
                DrawRectangleV(player.position, player.size, player.color);
                DrawCircleV(ball.position, ball.radius, ball.color);
                
                for (int i = 0; i < LINES_OF_BRICKS; i++) {
                    for (int j = 0; j < BRICKS_PER_LINE; j++)
                        if (bricks[i][j].active) DrawRectangleRec({bricks[i][j].position.x, bricks[i][j].position.y, bricks[i][j].size.x, bricks[i][j].size.y}, bricks[i][j].color);
                }

                // 绘制技能球
                for (int i = 0; i < MAX_POWERUPS; i++) {
                    if (powerUps[i].active) {
                        Color pColor = GOLD;
                        const char* pText = "?";
                        if (powerUps[i].type == WIDER_PADDLE) { pColor = BLUE; pText = "W"; }
                        else if (powerUps[i].type == EXTRA_LIFE) { pColor = RED; pText = "+"; }
                        else if (powerUps[i].type == SLOW_BALL) { pColor = GREEN; pText = "S"; }
                        
                        DrawCircleV(powerUps[i].position, powerUps[i].radius, pColor);
                        DrawText(pText, powerUps[i].position.x - 5, powerUps[i].position.y - 8, 15, WHITE);
                    }
                }

                DrawText(TextFormat("LIVES: %i  LEVEL: %i", lives, currentLevel), 10, screenHeight - 30, 20, DARKGRAY);
            } else {
                DrawText(gameOver ? "GAME OVER" : "YOU WIN!", 300, 250, 40, gameOver ? MAROON : GREEN);
                DrawText("PRESS [ENTER] TO RESTART", 260, 310, 20, GRAY);
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}