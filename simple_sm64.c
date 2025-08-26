#include <emscripten.h>
#include <stdio.h>
#include <stdlib.h>

// Simple game state
int gameRunning = 0;
int frameCount = 0;
int marioX = 0;
int marioY = 100;
int marioCoins = 0;

// Simple main loop
void main_loop() {
    if (!gameRunning) return;
    
    frameCount++;
    
    // Simple movement
    if (frameCount % 60 == 0) {
        marioX += 1;
        if (marioX > 100) marioX = 0;
    }
    
    // Simple coin collection
    if (frameCount % 120 == 0) {
        marioCoins++;
    }
}

// Initialize game
void init_game() {
    printf("Simple SM64 initialized!\n");
    gameRunning = 1;
}

// Main entry point
int main(int argc, char** argv) {
    printf("Simple SM64 starting...\n");
    
    init_game();
    
    // Set up main loop
    emscripten_set_main_loop(main_loop, 60, 1);
    
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