#include "raylib.h"
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>
#include <functional>
#include <unordered_map>
//g++ sandbox.cpp -o game.exe -I C:\msys64\mingw64\include -L C:\msys64\mingw64\lib -lraylib -lopengl32 -lgdi32 -lwinmm


/*
      v
    x<X

   %% %%    %%0%%    %% %%    %% %%    %% %%    %%0%%    %%0%%    %%0%%
   %   % |  %   % |  % 0 %    % 0 % |  %0  % |  % 0 %    % 1 %    % 1 %
   %000%    %000%    %000%    %111%    %111%    %111%    %111%    %111%
   %%%%%    %%%%%    %%%%%    %%%%%    %%%%%    %%%%%    %%%%%    %%%%%

    000
    vvv
    000

    000
    111

    .0.
    111
    222



    000000000
    111111111...
    222222222.2.
    33333333333.

    try(go down)
    if cant:
    {
        try(go left)
        if cant:
        try(go right)
        if cant:
        add +1 pressure to less pressured neighbour (down->left->right->up)
    }
    


    def try (dir){
        for i in range(pressure):
            go in dir 1 step
            pressure--
            if cant move:
                break
        go in girection pressure steps

        if cant go all pressure
    }
    
    def add pressure to less pressured neighbour (num){
        for i in range(num):
            min_neighbour = neighbour with least pressure
            if min_neighbour.pressure < self.pressure:
                min_neighbour.pressure++
            else:
                self.pressure += num-i
                break
    }


fire logic
check around neighbours is flameble 
if so, make it fire with lifetime with chance
if above 


*/


const int W = 1600;
const int H = 1000;

const Color BG = BLACK;

const int cellSize = 10;

const int gridx = W/cellSize;
const int gridy = H/cellSize;

const float GRAVITY = 60.f;
const float MAX_VY = 8.f;
const float MAX_VX = 3.f;

enum Element {
    EMPTY,
    SAND,
    WATER,
    FIRE,
    WOOD,
    STONE,
    ACID,
    EXPLOSION
};

std::unordered_map<Element, int> blastRes = {
    {EMPTY, 0},
    {SAND, 3},
    {WATER, 1},
    {FIRE, 0},
    {WOOD, 2},
    {STONE, 1},
    {ACID, 1}
};

Element current = SAND;

std::random_device rd;
std::mt19937 gen(rd());

std::uniform_int_distribution<int>    coin(0, 1);
std::uniform_int_distribution<int>    acid_r(0, 100);

float dist(int x1, int y1, int x2, int y2){
    return (sqrtf(powf(x2-x1, 2)+powf(y2-y1, 2)));
}

struct Particle
{
    Element type = EMPTY;
    int lifeTime;
    float vx = 5.0f, vy = 0.0f;
    int dir = 0;
    bool moved = false;
    int temperature = 0;

    Particle(Element type = EMPTY)
    : type(type), lifeTime(0) {}
    
    
};

//std::vector<Particle> particles;

struct Sandbox
{
    std::vector<Particle> data;
    Particle&       at(int x, int y)       { return data[y * gridx + x]; }
    const Particle& at(int x, int y) const { return data[y * gridx + x]; }
    
    bool in_bounds(int x, int y) const {
    return x >= 0 && x < gridx && y >= 0 && y < gridy; }


    Sandbox()
    : data(gridx*gridy){}
    
    void update(int frame)
    {
        for (int i = 0; i < gridx*gridy; i++) data[i].moved = false;
        for ( int y = gridy - 1; y >= 0; y--){
            for ( int i = 0; i < gridx; i++){
                int x = (frame) ? gridx-1-i : i;
                Particle& p = at(x, y);
                if (p.moved) continue;
                updateParticle(x, y, frame);
            }
        }
    }
    
    bool is_sand_moveable(int x, int y) {
        if (!in_bounds(x, y)) return false;
        return free(x, y, EMPTY) || free(x, y, WATER) || free(x, y, FIRE)|| free(x,y, ACID);
    }
    
    bool is_water_moveable(int x, int y) {
        if (!in_bounds(x, y)) return false;
        int a = coin(gen);
        return free(x, y, EMPTY) || (free(x, y, ACID) && a);
    }
    
    bool is_acid_moveable(int x, int y) {
        if (!in_bounds(x, y)) return false;
        int a = coin(gen);
        if (free(x, y, STONE) || free(x, y, WOOD)){
            int b = acid_r(gen);
            if (b==0){
                at(x, y).type = EMPTY;
            }
        }
        return free(x, y, EMPTY) || (free(x, y, WATER) && a);
    }
    
    bool is_fire_moveable(int x, int y) {
        if (!in_bounds(x, y)) return false;
        int a = coin(gen);
        if (free(x, y, STONE) || free(x, y, WOOD)){
            int b = acid_r(gen);
            if (b==0){
                at(x, y).type = EMPTY;
            }
        }
        return free(x, y, EMPTY) || (free(x, y, WATER) && a);
    }

    void updateParticle(int x, int y, int frame){
        Particle& p = at(x, y);
        if (p.moved) return;
        p.moved = true;
        switch (p.type)
        {
            case SAND:
                updateSolid(x, y, [this](int x, int y) { return is_water_moveable(x, y); });
                break;
            case WATER:
                updateLiquid(x, y, [this](int x, int y) { return is_water_moveable(x, y); });
                break;
            case ACID:
                updateLiquid(x, y, [this](int x, int y) { return is_acid_moveable(x, y); });
                break;
            case FIRE:
                updateFire(x, y);
                break;
            
                default:
                break;
            }
        }
        
    bool free(int x, int y, Element t) const {
        int i = y*gridx + x;
        return data[i].type == t;
    }

    Particle& TraverseMatrix(int sx, int sy, int ex, int ey){
        float start_x = (float) sx;
        float start_y = (float) sy;
        float end_x   = (float) ex;
        float end_y   = (float) ey;
        int prevx = start_x, prevy = start_y;
        if (std::abs(end_y-start_y)<=std::abs(end_x-start_x)){
            float slope = (end_y-start_y)/(end_x-start_x);
            if (end_x > start_x) {
                for (int x = start_x+1; x < end_x; x++){
                    int y = round(slope*(x - start_x) + start_y);
                    if (!in_bounds(x, y)) return at((int)x-1, (int)prevy);
                    if (at((int)x, (int)y).type != EMPTY){
                        return at((int)x-1, (int)prevy);
                    }
                    prevy = y;
                }
            } else {
                for (int x = start_x-1; x > end_x; x--){
                    int y = round(slope*(x - start_x) + start_y);
                    if (!in_bounds(x, y)) return at((int)x+1, (int)prevy);
                    if (at((int)x, (int)y).type != EMPTY){
                        return at((int)x+1, (int)prevy);
                    }
                    prevy = y;
                }
            }
        } else {
            float slope = (end_x-start_x)/(end_y-start_y);
            if (end_y > start_y) {
                for (int y = start_y+1; y < end_y; y++){
                    int x = round(slope*(y - start_y) + start_x);
                    if (!in_bounds(x, y)) return at((int)prevx, (int)y-1);
                    if (at((int)x, (int)y).type != EMPTY){
                        return at((int)prevx, (int)y-1);
                    }
                    prevx = x;
                }
            } else {
                for (int y = start_y-1; y > end_y; y--){
                    int x = round(slope*(y - start_y) + start_x);
                    if (!in_bounds(x, y)) return at((int)prevx, (int)y+1);
                    if (at((int)x, (int)y).type != EMPTY){
                        return at((int)prevx, (int)y+1);
                    }
                    prevx = x;
                }
            }
        }
        return at((int)end_x, (int)end_y);
    }
    
    void moveTo(int x1, int y1, int x2, int y2){
        Particle& target = TraverseMatrix(x1, y1, x2, y2);
        std::swap(at(x1, y1), target);
        target.moved = true;
    }
    
    void updateFire(int x, int y)
    {
        int a = coin(gen) * 2 - 1;
        

    }
    
    void updateSolid(int x, int y, std::function<bool(int, int)> is_solid_moveable)
    {
        int a = coin(gen) * 2 - 1;
        if      (is_solid_moveable(x,     y + 1)) moveTo(x, y, x,     y + 1);
        else if (is_solid_moveable(x + a, y + 1) && is_solid_moveable(x + a, y)) moveTo(x, y, x + a, y + 1);
        else if (is_solid_moveable(x - a, y + 1) && is_solid_moveable(x - a, y)) moveTo(x, y, x - a, y + 1);
        else at(x, y).moved = true;
    }
    
    void updateLiquid(int x, int y, std::function<bool(int, int)> is_liquid_moveable)
    {
        Particle& p = at(x, y);
        if (p.dir == 0) p.dir = coin(gen) * 2 - 1;
        int a = p.dir;
        if      (is_liquid_moveable(x,     y + 1)) {moveTo(x, y, x,     y + 1); return;}

        if (is_liquid_moveable(x + a, y + 1) && is_liquid_moveable(x + a, y)) {moveTo(x, y, x + a, y + 1); return;}
        if (is_liquid_moveable(x - a, y + 1) && is_liquid_moveable(x - a, y)) {moveTo(x, y, x - a, y + 1); return;}

        int disp = round(p.vx);
        int tx = x;
        for (int i = 1; i <=disp; i++){
            if(is_liquid_moveable(x + a * i, y)) {
                tx = x + a * i;
            }
            if (is_liquid_moveable(x + a * i, y + 1)) break;
            else break;
        }
        if (tx!=x) {moveTo(x, y, tx, y); return;}
        p.dir *= -1;
        a *= -1;
        for (int i = 1; i <=disp; i++){
            if(is_liquid_moveable(x + a * i, y)) {
                tx = x + a * i;
                if (is_liquid_moveable(tx, y + 1)) break;
            }
            if (is_liquid_moveable(x + a * i, y + 1)) break;
            else break;
        }
        if (tx!=x) {moveTo(x, y, tx, y); return;}
        p.moved = true;
    }

    void blastRow(int sx, int sy, int ex, int ey, int power){
        float start_x = (float) sx;
        float start_y = (float) sy;
        float end_x   = (float) ex;
        float end_y   = (float) ey;
        int remainingPower = power;
        if (std::abs(end_y-start_y)<=std::abs(end_x-start_x)){
            float slope = (end_y-start_y)/(end_x-start_x);
            if (end_x > start_x) {
                for (int x = start_x+1; x < end_x; x++){
                    int y = round(slope*(x - start_x) + start_y);
                    if (!in_bounds(x, y)) return;
                    if (dist(sx, sy, (int) x, (int) y)>power*2) return;
                    int res = blastRes[at((int)x, (int)y).type];
                    if (res >= 0){
                        if (res < remainingPower){
                            remainingPower -= res;
                            at((int)x, (int)y) = Particle();
                        } else return;
                    } else return;
                }
            } else {
                for (int x = start_x-1; x > end_x; x--){
                    int y = round(slope*(x - start_x) + start_y);
                    if (!in_bounds(x, y)) return;
                    if (dist(sx, sy, (int) x, (int) y)>power*2) return;
                    int res = blastRes[at((int)x, (int)y).type];
                    if (res >= 0){
                        if (res < remainingPower){
                            remainingPower -= res;
                            at((int)x, (int)y) = Particle();
                        } else return;
                    } else return;
                }
            }
        } else {
            float slope = (end_x-start_x)/(end_y-start_y);
            if (end_y > start_y) {
                for (int y = start_y+1; y < end_y; y++){
                    int x = round(slope*(y - start_y) + start_x);
                    if (!in_bounds(x, y)) return;
                    if (dist(sx, sy, (int) x, (int) y)>power*2) return;
                    int res = blastRes[at((int)x, (int)y).type];
                    if (res >= 0){
                        if (res < remainingPower){
                            remainingPower -= res;
                            at((int)x, (int)y) = Particle();
                        } else return;
                    } else return;
                }
            } else {
                for (int y = start_y-1; y > end_y; y--){
                    int x = round(slope*(y - start_y) + start_x);
                    if (!in_bounds(x, y)) return;
                    if (dist(sx, sy, (int) x, (int) y)>power*2) return;
                    int res = blastRes[at((int)x, (int)y).type];
                    if (res >= 0){
                        if (res < remainingPower){
                            remainingPower -= res;
                            at((int)x, (int)y) = Particle();
                        } else return;
                    } else return;
                }
            }
        }
    }

    void makeExplosion(int x, int y, int power){
        int remainingPower = power;
        if (!in_bounds(x, y)) return;
        int res = blastRes[at((int)x, (int)y).type];
        if (res >= 0){
            if (res < remainingPower){
                remainingPower -= res;
                at((int)x, (int)y) = Particle();
                } else return;
            } else return;
        for (int ix = x - power*2; ix < x+power*2; ix++) {
            blastRow(x, y, ix, y + power*2, power);
        }
        for (int iy = y +power*2-1; iy > y-power*2; iy--) {
            blastRow(x, y, x+power*2, iy, power);
        }
        for (int ix = x + power*2-1; ix > x-power*2; ix--) {
            blastRow(x, y, ix, y - power*2, power);
        }
        for (int iy = y - power*2 + 1; iy < y+power*2-1; iy++) {
            blastRow(x, y, x-power*2, iy, power);
        }
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

    void clear(){
        for (int i = 0; i < gridx*gridy; i++){
            data[i].type = EMPTY;
        }
    }
};

struct Button
{
    Color color;
    Element type;
    bool isActive = false;
    bool isSelected = false;
    int x, y;
    int sizex, sizey;

    Button(Element type, int x, int y, int sizex, int sizey, Color color)
    : type(type), x(x), y(y), sizex(sizex), sizey(sizey), color(color) {}

    bool isMouseOver(){
        Vector2 m = GetMousePosition();
        return (m.x >= x && m.x <= x+sizex && m.y >= y && m.y <= y+sizey);
    }

    void update()
    {
        if (isMouseOver()){
            isSelected = true;
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                current = type;
                isActive = true;
            }
        } else
        {
            isSelected = false;
        }
        if (current == type){
            isActive = true;
        } else {
            isActive = false;
        }
        render();
    }

    void render()
    {
        int nx = x, ny = y;
        int nsx = sizex, nsy = sizey;
        int a = 5;
        int border = 5;
        if (isSelected){
            nx -= a;
            ny -= a;
            nsx += 2*a;
            nsy += 2*a;
        }
        if (isActive) {
            DrawRectangle(nx-border, ny-border, nsx+2*border, nsy+2*border, {255, 255, 255, 255});
            DrawRectangle(nx-1, ny-1, nsx+2, nsy+2, BG);
        }
        DrawRectangle(nx, ny, nsx, nsy, color);
        
    }
};


int main()
{

    InitWindow(W, H, "Sandbox");
    SetTargetFPS(60);

    Sandbox sandbox;
    
    float t = 0.0f;
    int frame = 0;
    int pause = 0;

    int buttonSpacing = 100;
    int buttonSize = 50;

    std::vector<Button> buttons;
    Button sandButton = {SAND, W-buttonSpacing, buttonSpacing, buttonSize, buttonSize, {203, 189, 147, 255}};
    buttons.push_back(sandButton);
    Button waterButton = {WATER, W-buttonSpacing, buttonSpacing*2, buttonSize, buttonSize, { 15,  94, 156, 255}};
    buttons.push_back(waterButton);
    Button stoneButton = {STONE, W-buttonSpacing, buttonSpacing*3, buttonSize, buttonSize, {128, 128, 128, 255}};
    buttons.push_back(stoneButton);
    Button fireButton = {FIRE, W-buttonSpacing, buttonSpacing*4, buttonSize, buttonSize, {255,   0,   0, 255}};
    buttons.push_back(fireButton);
    Button acidButton = {ACID, W-buttonSpacing, buttonSpacing*5, buttonSize, buttonSize, {120, 255,   0, 255}};
    buttons.push_back(acidButton);
    Button explosionButton = {EXPLOSION, W-buttonSpacing, buttonSpacing*6, buttonSize, buttonSize, {120, 255,   0, 255}};
    buttons.push_back(explosionButton);
    Button woodButton = {WOOD, W-buttonSpacing, buttonSpacing*7, buttonSize, buttonSize, {139,  90,  43, 255}};
    buttons.push_back(woodButton);
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();
        t += dt;
        frame = (!frame);

        if (IsKeyPressed(KEY_ONE)) current = SAND;
        if (IsKeyPressed(KEY_TWO)) current = WATER;
        if (IsKeyPressed(KEY_THREE)) current = STONE;
        if (IsKeyPressed(KEY_FOUR)) current = FIRE;
        if (IsKeyPressed(KEY_FIVE)) current = ACID;
        if (IsKeyPressed(KEY_SIX)) current = EXPLOSION;
        if (IsKeyPressed(KEY_SEVEN)) current = EXPLOSION;
        if (IsKeyPressed(KEY_EIGHT)) current = WOOD;

        if (IsKeyPressed(KEY_SPACE)) pause = (!pause);
        if (IsKeyPressed(KEY_R)) sandbox.clear();

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 m = GetMousePosition();
            int gx = int(m.x / cellSize);
            int gy = int(m.y / cellSize);
            if (current != EXPLOSION){
                for (int i = -10; i <=10; i++){
                    for (int j = -10; j <= 10; j++){
                        if (gx+i >= 0 && gx+i < gridx && gy+j >= 0 && gy+j < gridy)
                        {
                            bool k = false;
                            for (Button b : buttons){
                                if (b.isMouseOver()) k = true;
                        }
                        if (!k) sandbox.at(gx+i, gy+j).type = current;
                    }
                }
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
        
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            if (current == EXPLOSION) {
                Vector2 m = GetMousePosition();
                int gx = int(m.x / cellSize);
                int gy = int(m.y / cellSize);
                int power = 10;
                sandbox.makeExplosion(gx, gy, power);
            }
        }
        
        if (!pause) sandbox.update(frame);
        
        
        BeginDrawing();
        ClearBackground(BG);
        sandbox.draw();
        DrawFPS(10, 10);
        
        for (Button b : buttons){
            b.update();
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}