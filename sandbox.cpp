#include "raylib.h"
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
//g++ sandbox.cpp -o game.exe -I C:\msys64\mingw64\include -L C:\msys64\mingw64\lib -lraylib -lopengl32 -lgdi32 -lwinmm

const int W = 1600;
const int H = 1000;

const int cellSize = 10;

const int gridx = W/cellSize;
const int gridy = H/cellSize;

std::random_device rd;
std::mt19937 gen(rd());

std::uniform_int_distribution<int>    coin(0, 1);


enum Element {
    EMPTY,
    SAND,
    WATER,
    FIRE,
    WOOD,
    STONE,
    ACID
};

struct Particle
{
    Element type = EMPTY;
    int lifeTime;
    int acseleration = 1;
    bool moved = false;

    Particle(Element type = EMPTY)
    : type(type), lifeTime(0), acseleration{1} {}
    
    
};

std::vector<Particle> particles;

struct Sandbox
{
    std::vector<Particle> data, next;
    Particle&       at(int x, int y)       { return data[y * gridx + x]; }
    const Particle& at(int x, int y) const { return data[y * gridx + x]; }
    
    bool in_bounds(int x, int y) const {
    return x >= 0 && x < gridx && y >= 0 && y < gridy; }


    Sandbox()
    : data(gridx*gridy), next(gridx*gridy) {}
    
    void update()
    {
        next.assign(gridx*gridy, Particle());
        for ( int y = gridy - 1; y >= 0; y--){
            for ( int x = gridx - 1; x >= 0; x--){
                Particle& p = at(x, y);
                if (p.moved) continue;
                switch (p.type)
                {
                    case SAND:
                        updateSand(x, y);
                        break;
                    case WATER:
                        updateWater(x, y);
                        break;
                    case ACID:
                        updateAcid(x, y);
                        break;
                    
                    default:
                        break;
                }
            }
        }
        std::swap(next, data);
    }
    
    void moveTo(int x1, int y1, int x2, int y2){
        next[y2*gridx + x2] = at(x1, y1);
        next[y1*gridx + x1] = at(x2, y2);
        next[y2*gridx + x2].moved = true;
    }

    bool free(int x, int y, Element t) const {
        int i = y*gridx + x;
        return data[i].type == t && (next[i].type == t || next[i].type == EMPTY);
    }

    bool is_sand_moveable(int x, int y) {
        if (!in_bounds(x, y)) return false;
        return free(x, y, EMPTY) || free(x, y, WATER) || free(x, y, FIRE);
    }
    
    bool is_water_moveable(int x, int y) {
        if (!in_bounds(x, y)) return false;
        int a = coin(gen);
        return free(x, y, EMPTY) || (free(x, y, ACID) && a);
    }
    
    bool is_acid_moveable(int x, int y) {
        if (!in_bounds(x, y)) return false;
        int a = coin(gen);
        return free(x, y, EMPTY) || (free(x, y, WATER) && a);
    }
    
    void updateSand(int x, int y)
    {
        int a = coin(gen) * 2 - 1;
        if      (is_sand_moveable(x,     y + 1)) moveTo(x, y, x,     y + 1);
        else if (is_sand_moveable(x + a, y + 1)) moveTo(x, y, x + a, y + 1);
        else if (is_sand_moveable(x - a, y + 1)) moveTo(x, y, x - a, y + 1);
        else next[y*gridx + x] = at(x, y);
    }
    
    void updateWater(int x, int y)
    {
        int a = coin(gen) * 2 - 1;
        if      (is_water_moveable(x,     y + 1)) moveTo(x, y, x,     y + 1);
        else if (is_water_moveable(x + a, y + 1)) moveTo(x, y, x + a, y + 1);
        else if (is_water_moveable(x - a, y + 1)) moveTo(x, y, x - a, y + 1);
        else if (is_water_moveable(x + a, y))     moveTo(x, y, x + a, y);
        else if (is_water_moveable(x - a, y))     moveTo(x, y, x - a, y);
        else next[y*gridx + x] = at(x, y);
    }
    
    void updateAcid(int x, int y)
    {
        int a = coin(gen) * 2 - 1;
        if      (is_acid_moveable(x,     y + 1)) moveTo(x, y, x,     y + 1);
        else if (is_acid_moveable(x + a, y + 1)) moveTo(x, y, x + a, y + 1);
        else if (is_acid_moveable(x - a, y + 1)) moveTo(x, y, x - a, y + 1);
        else if (is_acid_moveable(x + a, y))     moveTo(x, y, x + a, y);
        else if (is_acid_moveable(x - a, y))     moveTo(x, y, x - a, y);
        else next[y*gridx + x] = at(x, y);
    }

    void draw()
    {
        for ( int x = 0; x < gridx; x++){
            for ( int y = 0; y < gridy; y++){
                Particle& p = at(x, y);
                DrawRectangle(x*cellSize, y*cellSize, cellSize, cellSize, color_of(p.type));
    
            }
        }
    }    
    static Color color_of(Element t) {
        switch (t) {
            case EMPTY: return {0,   0,   0,   0};
            case SAND:  return {203, 189, 147, 255};
            case WATER: return { 15,  94, 156, 255};
            case FIRE:  return {255,   0,   0, 255};
            case WOOD:  return {139,  90,  43, 255};
            case STONE: return {128, 128, 128, 255};
            case ACID:  return {120, 255,   0, 255};
            default:    return {255, 255, 255, 255};
        }
    }
};

int main()
{

    InitWindow(W, H, "Sandbox");
    SetTargetFPS(60);

    Sandbox sandbox;
    
    float t = 0.0f;
    Element type = SAND;
    
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        t += dt;

        if (IsKeyPressed(KEY_ONE)) type = SAND;
        if (IsKeyPressed(KEY_TWO)) type = WATER;
        if (IsKeyPressed(KEY_THREE)) type = STONE;
        if (IsKeyPressed(KEY_FOUR)) type = FIRE;
        if (IsKeyPressed(KEY_FIVE)) type = ACID;


        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 m = GetMousePosition();
            int gx = int(m.x / cellSize);
            int gy = int(m.y / cellSize);
            for (int i = -3; i <=3; i++){
                for (int j = -3; j <= 3; j++){
                    if (gx+i >= 0 && gx+i < gridx && gy+j >= 0 && gy+j < gridy)
                        sandbox.at(gx+i, gy+j).type = type;
                }
            }
        }
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 m = GetMousePosition();
            int gx = int(m.x / cellSize);
            int gy = int(m.y / cellSize);
            for (int i = -3; i <=3; i++){
                for (int j = -3; j <= 3; j++){
                    if (gx+i >= 0 && gx+i < gridx && gy+j >= 0 && gy+j < gridy)
                        sandbox.at(gx+i, gy+j).type = EMPTY;
                }
            }
        }

        sandbox.update();
        
        BeginDrawing();
        ClearBackground(BLACK);
        sandbox.draw();
        DrawFPS(10, 10);
        
        EndDrawing();
    }

    CloseWindow();
    return 0;
}