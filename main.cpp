#include "raylib.h"
#include <vector>
#include <string>
#include <cmath>

// 常量定义
const int screenWidth = 800;
const int screenHeight = 600;

// 游戏状态
typedef enum GameScreen { TITLE, GAMEPLAY, LEVEL_UP, ENDING } GameScreen;

// 技能类型 (包含多球和火球)
typedef enum PowerType { NONE, WIDER_PADDLE, EXTRA_LIFE, MULTI_BALL, FIRE_BALL } PowerType;

struct Brick {
    Rectangle rect;
    bool active;
    Color color;
};

// 游戏球结构体（支持多球机制）
struct GameBall {
    Vector2 pos;
    Vector2 speed;
    float radius;
    bool active;
    float fireTimer; // 大于0表示是火球状态
};

struct PowerUp {
    Vector2 pos;
    PowerType type;
    bool active;
    float speed = 3.0f;
};

// 游戏全局变量
GameScreen currentScreen = TITLE;
int currentLevel = 1;
int lives = 3;
float paddleTimer = 0.0f; 

Rectangle paddle = { screenWidth / 2 - 50.0f, screenHeight - 40.0f, 100.0f, 15.0f };
std::vector<GameBall> balls;
std::vector<Brick> bricks;
std::vector<PowerUp> powerUps;

// 初始化关卡函数
void InitLevel(int level) {
    bricks.clear();
    powerUps.clear();
    balls.clear();
    paddle.width = 100.0f;
    paddleTimer = 0.0f;
    
    // 初始化第一个球
    float speedBase = 4.0f + (level * 1.5f);
    GameBall initialBall = { 
        { (float)screenWidth / 2, (float)screenHeight / 2 + 50.0f }, 
        { speedBase, -speedBase }, 
        8.0f, true, 0.0f 
    };
    balls.push_back(initialBall);

    // 砖块行数随关卡增加
    int rows = 1 + (level * 2);
    int cols = 10;
    float bWidth = 70.0f;
    float bHeight = 20.0f;
    float padding = 8.0f;
    float startX = (screenWidth - (cols * (bWidth + padding))) / 2.0f;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            Brick b;
            b.rect = { startX + j * (bWidth + padding), 60.0f + i * (bHeight + padding), bWidth, bHeight };
            b.active = true;
            if (i < 2) b.color = RED;
            else if (i < 4) b.color = ORANGE;
            else b.color = GOLD;
            bricks.push_back(b);
        }
    }
}

int main() {
    InitWindow(screenWidth, screenHeight, "Raylib Breakout: Advanced Mechanics");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // --- 逻辑更新 ---
        switch (currentScreen) {
            case TITLE: {
                if (IsKeyPressed(KEY_ENTER)) {
                    currentLevel = 1;
                    lives = 3;
                    InitLevel(currentLevel);
                    currentScreen = GAMEPLAY;
                }
                break;
            }

            case GAMEPLAY: { 
                // 1. 挡板移动
                if (IsKeyDown(KEY_LEFT) && paddle.x > 0) paddle.x -= 8.0f;
                if (IsKeyDown(KEY_RIGHT) && paddle.x < screenWidth - paddle.width) paddle.x += 8.0f;

                // 2. 技能倒计时
                if (paddleTimer > 0) {
                    paddleTimer -= GetFrameTime();
                    if (paddleTimer <= 0) paddle.width = 100.0f;
                }

                // 3. 多球独立运动与碰撞
                int activeBallsCount = 0;
                for (auto &ball : balls) {
                    if (!ball.active) continue;
                    activeBallsCount++;

                    // 火球倒计时
                    if (ball.fireTimer > 0) ball.fireTimer -= GetFrameTime();

                    ball.pos.x += ball.speed.x;
                    ball.pos.y += ball.speed.y;

                    // 墙壁碰撞
                    if (ball.pos.x + ball.radius >= screenWidth || ball.pos.x - ball.radius <= 0) ball.speed.x *= -1;
                    if (ball.pos.y - ball.radius <= 0) ball.speed.y *= -1;

                    // 掉落底部
                    if (ball.pos.y + ball.radius >= screenHeight) {
                        ball.active = false;
                    }

                    // 挡板碰撞
                    if (CheckCollisionCircleRec(ball.pos, ball.radius, paddle)) {
                        ball.speed.y = -std::abs(ball.speed.y); // 强制向上弹
                        ball.pos.y = paddle.y - ball.radius;
                    }

                    // 砖块碰撞
                    for (auto &b : bricks) {
                        if (b.active && CheckCollisionCircleRec(ball.pos, ball.radius, b.rect)) {
                            b.active = false;
                            
                            // 只有不是火球时，才反弹！火球直接穿透！
                            if (ball.fireTimer <= 0) {
                                ball.speed.y *= -1; 
                            }
                            
                            // 掉落技能 (概率设为 25%)
                            if (GetRandomValue(1, 100) <= 25) {
                                PowerUp p;
                                p.pos = { b.rect.x + b.rect.width/2, b.rect.y };
                                p.type = (PowerType)GetRandomValue(1, 4); // 随机 1~4 的技能
                                p.active = true;
                                powerUps.push_back(p);
                            }
                            
                            if (ball.fireTimer <= 0) break; // 普通球一次只打一个，火球可同时打穿多个
                        }
                    }
                }

                // 生命值判定 (所有球都掉落了)
                if (activeBallsCount == 0) {
                    lives--;
                    if (lives <= 0) {
                        currentScreen = ENDING;
                    } else {
                        // 重新生成一个球
                        balls.clear();
                        float speedBase = 4.0f + (currentLevel * 1.5f);
                        GameBall newBall = { 
                            { (float)screenWidth / 2, (float)screenHeight / 2 + 50.0f }, 
                            { speedBase, -speedBase }, 
                            8.0f, true, 0.0f 
                        };
                        balls.push_back(newBall);
                    }
                }

                // 4. 更新掉落物与技能生效
                for (auto &p : powerUps) {
                    if (p.active) {
                        p.pos.y += p.speed;
                        if (CheckCollisionCircleRec(p.pos, 10.0f, paddle)) {
                            p.active = false;
                            
                            // 判断吃到什么技能
                            switch (p.type) {
                                case WIDER_PADDLE:
                                    paddle.width = 180.0f;
                                    paddleTimer = 10.0f; 
                                    break;
                                case EXTRA_LIFE:
                                    lives++;
                                    break;
                                case MULTI_BALL: {
                                    // 找到当前活跃的一个球作为母球分裂
                                    GameBall cloneBall;
                                    bool found = false;
                                    for (const auto& b : balls) { if(b.active) { cloneBall = b; found = true; break; } }
                                    if(found) {
                                        GameBall b1 = cloneBall; b1.speed.x *= -1; // 水平反向
                                        GameBall b2 = cloneBall; b2.speed.y = -std::abs(b2.speed.y); // 确保向上
                                        balls.push_back(b1);
                                        balls.push_back(b2);
                                    }
                                    break;
                                }
                                case FIRE_BALL: {
                                    // 让所有活跃的球变成火球
                                    for (auto& b : balls) {
                                        if (b.active) b.fireTimer = 5.0f; // 持续5秒
                                    }
                                    break;
                                }
                                default: break;
                            }
                        }
                        if (p.pos.y > screenHeight) p.active = false;
                    }
                }

                // 5. 胜利判定
                bool allBroken = true;
                for (const auto &b : bricks) { if (b.active) { allBroken = false; break; } }
                if (allBroken) {
                    if (currentLevel >= 3) currentScreen = ENDING; 
                    else currentScreen = LEVEL_UP;
                }
                break;
            } 

            case LEVEL_UP: {
                if (IsKeyPressed(KEY_ENTER)) {
                    currentLevel++;
                    InitLevel(currentLevel);
                    currentScreen = GAMEPLAY;
                }
                break;
            }

            case ENDING: {
                if (IsKeyPressed(KEY_ENTER)) currentScreen = TITLE;
                break;
            }
        }

        // --- 渲染绘制 ---
        BeginDrawing();
            ClearBackground(BLACK);

            switch (currentScreen) {
                case TITLE: {
                    // 绘制酷炫的开始界面
                    DrawText("SUPER BREAKOUT", screenWidth/2 - MeasureText("SUPER BREAKOUT", 50)/2, 180, 50, GOLD);
                    DrawText("NEW MECHANICS EDITION", screenWidth/2 - MeasureText("NEW MECHANICS EDITION", 20)/2, 240, 20, LIGHTGRAY);
                    
                    // 闪烁效果：利用 GetTime()
                    if ((int)(GetTime() * 2.0) % 2 == 0) {
                        DrawText("PRESS [ENTER] TO START", screenWidth/2 - MeasureText("PRESS [ENTER] TO START", 20)/2, 350, 20, GREEN);
                    }
                    
                    // 技能图鉴说明
                    DrawText("POWER-UPS:", 280, 420, 20, RAYWHITE);
                    DrawCircle(300, 460, 8.0f, BLUE);  DrawText(" WIDE", 315, 452, 15, WHITE);
                    DrawCircle(300, 485, 8.0f, GREEN); DrawText(" LIFE", 315, 477, 15, WHITE);
                    DrawCircle(450, 460, 8.0f, PINK);  DrawText(" MULTI-BALL", 465, 452, 15, WHITE);
                    DrawCircle(450, 485, 8.0f, RED);   DrawText(" FIREBALL", 465, 477, 15, WHITE);
                    break;
                }

                case GAMEPLAY: { 
                    // 画 UI
                    DrawText(TextFormat("LEVEL: %i", currentLevel), 20, 20, 20, RAYWHITE);
                    DrawText(TextFormat("LIVES: %i", lives), 150, 20, 20, GREEN);
                    if (paddleTimer > 0) DrawText("WIDE ACTIVE!", 300, 20, 20, SKYBLUE);

                    // 画挡板
                    DrawRectangleRec(paddle, paddleTimer > 0 ? SKYBLUE : GRAY);

                    // 画所有的球
                    for (const auto &ball : balls) {
                        if (ball.active) {
                            Color ballColor = (ball.fireTimer > 0) ? RED : WHITE; // 火球变红
                            DrawCircleV(ball.pos, ball.radius, ballColor);
                            // 火球发光特效
                            if (ball.fireTimer > 0) DrawCircleLinesV(ball.pos, ball.radius + 2.0f, ORANGE);
                        }
                    }

                    // 画砖块
                    for (const auto &b : bricks) {
                        if (b.active) DrawRectangleRec(b.rect, b.color);
                    }

                    // 画技能球
                    for (const auto &p : powerUps) {
                        if (p.active) {
                            Color c;
                            const char* txt = "";
                            if (p.type == WIDER_PADDLE) { c = BLUE; txt = "W"; }
                            else if (p.type == EXTRA_LIFE) { c = GREEN; txt = "+"; }
                            else if (p.type == MULTI_BALL) { c = PINK; txt = "M"; }
                            else if (p.type == FIRE_BALL) { c = RED; txt = "F"; }
                            
                            DrawCircleV(p.pos, 10.0f, c);
                            DrawText(txt, p.pos.x - 4, p.pos.y - 5, 12, WHITE);
                        }
                    }
                    break;
                } 

                case LEVEL_UP: {
                    DrawText("LEVEL CLEAR!", 250, 250, 40, GOLD);
                    DrawText("PRESS [ENTER] FOR NEXT LEVEL", 210, 320, 20, RAYWHITE);
                    break;
                }

                case ENDING: {
                    if (lives <= 0) DrawText("GAME OVER", 280, 250, 40, RED);
                    else DrawText("YOU WIN! CONGRATS!", 200, 250, 40, GOLD);
                    DrawText("PRESS [ENTER] TO RETURN TITLE", 220, 350, 20, GRAY);
                    break;
                }
            }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}