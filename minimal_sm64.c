#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>

// Simple game state
int gameRunning = 0;
int frameCount = 0;
int marioX = 0;
int marioY = 100;
int marioCoins = 0;

// Initialize game
void init_game() {
    printf("Minimal SM64 initialized!\n");
    gameRunning = 1;
    frameCount = 0;
}

// Main entry point
int main(int argc, char** argv) {
    printf("Minimal SM64 starting...\n");
    
    init_game();
    
    printf("Minimal SM64 started successfully!\n");
    return 0;
}

// Emscripten callbacks
EMSCRIPTEN_KEEPALIVE
void start_game() {
    printf("Game started!\n");
    gameRunning = 1;
}

EMSCRIPTEN_KEEPALIVE
void pause_game() {
    printf("Game paused!\n");
    gameRunning = 0;
}

EMSCRIPTEN_KEEPALIVE
int get_frame_count() {
    return frameCount;
}

EMSCRIPTEN_KEEPALIVE
int get_mario_x() {
    return marioX;
}

EMSCRIPTEN_KEEPALIVE
int get_mario_y() {
    return marioY;
}

EMSCRIPTEN_KEEPALIVE
int get_mario_coins() {
    return marioCoins;
}

EMSCRIPTEN_KEEPALIVE
void increment_frame() {
    frameCount++;
    printf("Frame incremented to: %d\n", frameCount);
}

EMSCRIPTEN_KEEPALIVE
void move_mario_right() {
    marioX += 10;
    printf("Mario moved right to: %d\n", marioX);
}

EMSCRIPTEN_KEEPALIVE
void collect_coin() {
    marioCoins++;
    printf("Coin collected! Total: %d\n", marioCoins);
}