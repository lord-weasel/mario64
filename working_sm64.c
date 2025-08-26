#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/em_asm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Game state
int gameRunning = 0;
int frameCount = 0;
int gameInitialized = 0;

// Mario state
float marioX = 0.0f;
float marioY = 100.0f;
int marioCoins = 0;
int marioHealth = 4;

// Controller state
int controllerLeft = 0;
int controllerRight = 0;
int controllerA = 0;
int controllerB = 0;

// Game objects
typedef struct {
    float x, y;
    float size;
    int type; // 0=empty, 1=block, 2=coin
    int active;
} GameObject;

GameObject gameObjects[50];
int objectCount = 0;

// Function declarations
void init_game(void);
void update_game(void);
void render_game(void);
void setup_input(void);
void create_objects(void);
void update_mario(void);

// Main game loop
void main_loop() {
    if (!gameRunning || !gameInitialized) return;
    
    frameCount++;
    
    // Update game logic
    update_game();
    
    // Render frame
    render_game();
}

// Initialize the game
void init_game(void) {
    printf("Initializing Super Mario 64...\n");
    
    // Set up input
    setup_input();
    
    // Create game objects
    create_objects();
    
    printf("Super Mario 64 initialized successfully!\n");
    gameInitialized = 1;
    gameRunning = 1;
}

// Set up input handling
void setup_input(void) {
    EM_ASM({
        // Keyboard input mapping
        document.addEventListener('keydown', function(event) {
            switch(event.code) {
                case 'ArrowLeft':
                    Module.controllerLeft = 1;
                    break;
                case 'ArrowRight':
                    Module.controllerRight = 1;
                    break;
                case 'Space':
                    Module.controllerA = 1;
                    break;
                case 'KeyZ':
                    Module.controllerB = 1;
                    break;
            }
        });
        
        document.addEventListener('keyup', function(event) {
            switch(event.code) {
                case 'ArrowLeft':
                    Module.controllerLeft = 0;
                    break;
                case 'ArrowRight':
                    Module.controllerRight = 0;
                    break;
                case 'Space':
                    Module.controllerA = 0;
                    break;
                case 'KeyZ':
                    Module.controllerB = 0;
                    break;
            }
        });
    });
}

// Create initial game objects
void create_objects(void) {
    objectCount = 0;
    
    // Create some blocks
    for (int i = 0; i < 10; i++) {
        gameObjects[objectCount].x = (i - 5) * 50.0f;
        gameObjects[objectCount].y = 0.0f;
        gameObjects[objectCount].size = 25.0f;
        gameObjects[objectCount].type = 1; // Block
        gameObjects[objectCount].active = 1;
        objectCount++;
    }
    
    // Create some coins
    for (int i = 0; i < 5; i++) {
        gameObjects[objectCount].x = (i - 2) * 100.0f;
        gameObjects[objectCount].y = 50.0f;
        gameObjects[objectCount].size = 15.0f;
        gameObjects[objectCount].type = 2; // Coin
        gameObjects[objectCount].active = 1;
        objectCount++;
    }
    
    printf("Created %d game objects\n", objectCount);
}

// Update Mario physics and logic
void update_mario(void) {
    // Handle movement
    if (controllerLeft) {
        marioX -= 3.0f;
    }
    if (controllerRight) {
        marioX += 3.0f;
    }
    
    // Handle jumping
    if (controllerA && marioY <= 100.0f) {
        marioY = 150.0f;
    }
    
    // Gravity
    if (marioY > 100.0f) {
        marioY -= 4.0f;
    }
    if (marioY < 100.0f) {
        marioY = 100.0f;
    }
    
    // Collect coins
    for (int i = 0; i < objectCount; i++) {
        if (gameObjects[i].type == 2 && gameObjects[i].active) { // Coin
            float dx = marioX - gameObjects[i].x;
            float dy = marioY - gameObjects[i].y;
            float distance = sqrt(dx*dx + dy*dy);
            if (distance < 30.0f) {
                gameObjects[i].active = 0; // Collected
                marioCoins++;
                printf("Coin collected! Total: %d\n", marioCoins);
            }
        }
    }
}

// Update game logic
void update_game(void) {
    // Update Mario
    update_mario();
    
    // Update game objects
    for (int i = 0; i < objectCount; i++) {
        if (gameObjects[i].type == 2 && gameObjects[i].active) { // Coins rotate
            gameObjects[i].y += sin(frameCount * 0.1f) * 2.0f;
        }
    }
}

// Render the game
void render_game(void) {
    EM_ASM({
        var gl = Module.canvas.getContext('webgl');
        
        // Clear screen
        gl.clearColor(0.0, 0.0, 0.0, 1.0);
        gl.clear(gl.COLOR_BUFFER_BIT);
        
        // Create shader program
        var vertexShader = gl.createShader(gl.VERTEX_SHADER);
        gl.shaderSource(vertexShader, `
            attribute vec2 a_position;
            uniform vec2 u_offset;
            uniform float u_scale;
            void main() {
                vec2 pos = a_position * u_scale + u_offset;
                gl_Position = vec4(pos, 0.0, 1.0);
            }
        `);
        gl.compileShader(vertexShader);
        
        var fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
        gl.shaderSource(fragmentShader, `
            precision mediump float;
            uniform vec3 u_color;
            void main() {
                gl_FragColor = vec4(u_color, 1.0);
            }
        `);
        gl.compileShader(fragmentShader);
        
        var program = gl.createProgram();
        gl.attachShader(program, vertexShader);
        gl.attachShader(program, fragmentShader);
        gl.linkProgram(program);
        gl.useProgram(program);
        
        var offsetLocation = gl.getUniformLocation(program, 'u_offset');
        var scaleLocation = gl.getUniformLocation(program, 'u_scale');
        var colorLocation = gl.getUniformLocation(program, 'u_color');
        
        // Render Mario
        var marioX = (Module.marioX / 320.0) - 1.0;
        var marioY = (Module.marioY / 240.0) - 1.0;
        
        gl.uniform2f(offsetLocation, marioX, marioY);
        gl.uniform1f(scaleLocation, 0.1);
        gl.uniform3f(colorLocation, 1.0, 0.5, 0.0); // Mario's color
        
        var vertices = new Float32Array([
            -1.0, -1.0,
             1.0, -1.0,
             1.0,  1.0,
            -1.0,  1.0
        ]);
        
        var buffer = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
        gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
        
        var positionLocation = gl.getAttribLocation(program, 'a_position');
        gl.enableVertexAttribArray(positionLocation);
        gl.vertexAttribPointer(positionLocation, 2, gl.FLOAT, false, 0, 0);
        
        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
        
        // Render game objects
        for (var i = 0; i < Module.objectCount; i++) {
            var obj = Module.gameObjects[i];
            if (!obj.active) continue;
            
            var objX = (obj.x / 320.0) - 1.0;
            var objY = (obj.y / 240.0) - 1.0;
            var objScale = obj.size / 320.0;
            
            gl.uniform2f(offsetLocation, objX, objY);
            gl.uniform1f(scaleLocation, objScale);
            
            if (obj.type === 1) { // Block
                gl.uniform3f(colorLocation, 0.0, 0.8, 0.0); // Green
            } else if (obj.type === 2) { // Coin
                gl.uniform3f(colorLocation, 1.0, 1.0, 0.0); // Yellow
            }
            
            gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
        }
    });
}

// Main entry point
int main(int argc, char** argv) {
    printf("Super Mario 64 Web starting...\n");
    
    // Initialize the game
    init_game();
    
    // Set up the main loop
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
int is_game_initialized() {
    return gameInitialized;
}

EMSCRIPTEN_KEEPALIVE
int get_mario_coins() {
    return marioCoins;
}

EMSCRIPTEN_KEEPALIVE
float get_mario_x() {
    return marioX;
}

EMSCRIPTEN_KEEPALIVE
float get_mario_y() {
    return marioY;
}

EMSCRIPTEN_KEEPALIVE
int get_mario_health() {
    return marioHealth;
}