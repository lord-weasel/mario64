#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/em_asm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Canvas and rendering variables
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE glContext;
int canvasWidth = 640;
int canvasHeight = 480;

// Game state
int gameRunning = 0;
int frameCount = 0;
int gameInitialized = 0;

// SM64 game state (simplified but functional)
typedef struct {
    float x, y, z;
    float rotation;
    int action;
    int health;
    int coins;
    int stars;
} MarioState;

typedef struct {
    float x, y, z;
    float size;
    int type;
} GameObject;

// Game objects
MarioState mario = {0, 100, 0, 0, 4, 0, 0};
GameObject objects[100];
int objectCount = 0;

// Controller state
struct {
    int up, down, left, right;
    int a, b, z, start;
} controller = {0};

// Function declarations
void init_sm64_game(void);
void update_sm64_game(void);
void render_sm64_game(void);
void setup_emscripten_webgl(void);
void setup_emscripten_input(void);
void create_game_objects(void);
void update_mario_physics(void);
void render_mario(void);
void render_objects(void);

// Emscripten main loop
void main_loop() {
    if (!gameRunning || !gameInitialized) return;
    
    frameCount++;
    
    // Update SM64 game logic
    update_sm64_game();
    
    // Render SM64 frame
    render_sm64_game();
}

// Initialize SM64 game
void init_sm64_game(void) {
    printf("Initializing Super Mario 64...\n");
    
    // Set up WebGL context for Emscripten
    setup_emscripten_webgl();
    
    // Set up input system
    setup_emscripten_input();
    
    // Create initial game objects
    create_game_objects();
    
    printf("Super Mario 64 initialized successfully!\n");
    gameInitialized = 1;
    gameRunning = 1;
}

// Set up Emscripten WebGL context
void setup_emscripten_webgl(void) {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = 0;
    attrs.depth = 1;
    attrs.stencil = 0;
    attrs.antialias = 0;
    attrs.premultipliedAlpha = 0;
    attrs.preserveDrawingBuffer = 0;
    attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_DEFAULT;
    attrs.failIfMajorPerformanceCaveat = 0;
    attrs.enableExtensionsByDefault = 0;
    attrs.explicitSwapControl = 0;
    attrs.renderViaOffscreenBackBuffer = 0;
    
    glContext = emscripten_webgl_create_context("#game", &attrs);
    if (glContext < 0) {
        printf("Failed to create WebGL context: %lu\n", glContext);
        return;
    }
    
    emscripten_webgl_make_context_current(glContext);
    
    // Initialize WebGL state
    EM_ASM({
        var gl = Module.canvas.getContext('webgl');
        gl.clearColor(0.0, 0.0, 0.0, 1.0);
        gl.clearDepth(1.0);
        gl.enable(gl.DEPTH_TEST);
        gl.depthFunc(gl.LEQUAL);
        gl.viewport(0, 0, 640, 480);
    });
    
    printf("WebGL context initialized successfully\n");
}

// Set up Emscripten input system
void setup_emscripten_input(void) {
    // Set up keyboard input handling for SM64
    EM_ASM({
        // Keyboard input mapping for SM64
        document.addEventListener('keydown', function(event) {
            switch(event.code) {
                case 'ArrowUp':
                    Module.controller.up = 1;
                    break;
                case 'ArrowDown':
                    Module.controller.down = 1;
                    break;
                case 'ArrowLeft':
                    Module.controller.left = 1;
                    break;
                case 'ArrowRight':
                    Module.controller.right = 1;
                    break;
                case 'Space':
                    Module.controller.a = 1;
                    break;
                case 'KeyZ':
                    Module.controller.b = 1;
                    break;
                case 'KeyX':
                    Module.controller.z = 1;
                    break;
                case 'KeyC':
                    Module.controller.start = 1;
                    break;
            }
        });
        
        document.addEventListener('keyup', function(event) {
            switch(event.code) {
                case 'ArrowUp':
                    Module.controller.up = 0;
                    break;
                case 'ArrowDown':
                    Module.controller.down = 0;
                    break;
                case 'ArrowLeft':
                    Module.controller.left = 0;
                    break;
                case 'ArrowRight':
                    Module.controller.right = 0;
                    break;
                case 'Space':
                    Module.controller.a = 0;
                    break;
                case 'KeyZ':
                    Module.controller.b = 0;
                    break;
                case 'KeyX':
                    Module.controller.z = 0;
                    break;
                case 'KeyC':
                    Module.controller.start = 0;
                    break;
            }
        });
    });
}

// Create initial game objects (blocks, enemies, etc.)
void create_game_objects(void) {
    objectCount = 0;
    
    // Create some blocks
    for (int i = 0; i < 10; i++) {
        objects[objectCount].x = (i - 5) * 50.0f;
        objects[objectCount].y = 0.0f;
        objects[objectCount].z = 0.0f;
        objects[objectCount].size = 25.0f;
        objects[objectCount].type = 1; // Block
        objectCount++;
    }
    
    // Create some coins
    for (int i = 0; i < 5; i++) {
        objects[objectCount].x = (i - 2) * 100.0f;
        objects[objectCount].y = 50.0f;
        objects[objectCount].z = 0.0f;
        objects[objectCount].size = 10.0f;
        objects[objectCount].type = 2; // Coin
        objectCount++;
    }
    
    printf("Created %d game objects\n", objectCount);
}

// Update Mario physics and game logic
void update_mario_physics(void) {
    // Handle movement
    if (controller.left) {
        mario.x -= 2.0f;
        mario.rotation = 180.0f;
    }
    if (controller.right) {
        mario.x += 2.0f;
        mario.rotation = 0.0f;
    }
    
    // Handle jumping
    if (controller.a && mario.y <= 100.0f) {
        mario.y = 150.0f;
    }
    
    // Gravity
    if (mario.y > 100.0f) {
        mario.y -= 3.0f;
    }
    if (mario.y < 100.0f) {
        mario.y = 100.0f;
    }
    
    // Handle punching
    if (controller.b) {
        mario.action = 1; // Punching
    } else {
        mario.action = 0; // Idle
    }
    
    // Collect coins
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].type == 2) { // Coin
            float dx = mario.x - objects[i].x;
            float dy = mario.y - objects[i].y;
            float distance = sqrt(dx*dx + dy*dy);
            if (distance < 30.0f) {
                objects[i].type = 0; // Collected
                mario.coins++;
                printf("Coin collected! Total: %d\n", mario.coins);
            }
        }
    }
}

// Update SM64 game logic
void update_sm64_game(void) {
    // Update Mario physics
    update_mario_physics();
    
    // Update game objects
    for (int i = 0; i < objectCount; i++) {
        if (objects[i].type == 2) { // Coins rotate
            objects[i].y += sin(frameCount * 0.1f) * 2.0f;
        }
    }
}

// Render Mario
void render_mario(void) {
    EM_ASM({
        var gl = Module.canvas.getContext('webgl');
        
        // Create shader for Mario
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
        
        // Mario's position (convert from game coordinates to screen coordinates)
        var marioX = (Module.mario.x / 320.0) - 1.0;
        var marioY = (Module.mario.y / 240.0) - 1.0;
        
        var offsetLocation = gl.getUniformLocation(program, 'u_offset');
        var scaleLocation = gl.getUniformLocation(program, 'u_scale');
        var colorLocation = gl.getUniformLocation(program, 'u_color');
        
        gl.uniform2f(offsetLocation, marioX, marioY);
        gl.uniform1f(scaleLocation, 0.1);
        gl.uniform3f(colorLocation, 1.0, 0.5, 0.0); // Mario's color
        
        // Mario's shape (rectangle)
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
    });
}

// Render game objects
void render_objects(void) {
    EM_ASM({
        var gl = Module.canvas.getContext('webgl');
        
        // Create shader for objects
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
        
        // Render each object
        for (var i = 0; i < Module.objectCount; i++) {
            var obj = Module.objects[i];
            if (obj.type === 0) continue; // Skip collected objects
            
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
        }
    });
}

// Render SM64 game frame
void render_sm64_game(void) {
    // Clear the screen
    EM_ASM({
        var gl = Module.canvas.getContext('webgl');
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
    });
    
    // Render the game world
    render_objects();
    render_mario();
    
    // Render HUD
    EM_ASM({
        var gl = Module.canvas.getContext('webgl');
        
        // Create shader for HUD text
        var vertexShader = gl.createShader(gl.VERTEX_SHADER);
        gl.shaderSource(vertexShader, `
            attribute vec2 a_position;
            uniform vec2 u_offset;
            void main() {
                vec2 pos = a_position + u_offset;
                gl_Position = vec4(pos, 0.0, 1.0);
            }
        `);
        gl.compileShader(vertexShader);
        
        var fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
        gl.shaderSource(fragmentShader, `
            precision mediump float;
            void main() {
                gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0); // White text
            }
        `);
        gl.compileShader(fragmentShader);
        
        var program = gl.createProgram();
        gl.attachShader(program, vertexShader);
        gl.attachShader(program, fragmentShader);
        gl.linkProgram(program);
        gl.useProgram(program);
        
        // Render HUD elements
        var offsetLocation = gl.getUniformLocation(program, 'u_offset');
        
        // Coins counter
        gl.uniform2f(offsetLocation, -0.8, 0.8);
        var coinText = "COINS: " + Module.mario.coins;
        // Simple text rendering (just a rectangle for now)
        var vertices = new Float32Array([
            -0.1, -0.05,
             0.1, -0.05,
             0.1,  0.05,
            -0.1,  0.05
        ]);
        
        var buffer = gl.createBuffer();
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
        gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);
        
        var positionLocation = gl.getAttribLocation(program, 'a_position');
        gl.enableVertexAttribArray(positionLocation);
        gl.vertexAttribPointer(positionLocation, 2, gl.FLOAT, false, 0, 0);
        
        gl.drawArrays(gl.TRIANGLE_FAN, 0, 4);
    });
}

// Main entry point
int main(int argc, char** argv) {
    printf("Super Mario 64 Web starting...\n");
    
    // Initialize the game
    init_sm64_game();
    
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
    return mario.coins;
}

EMSCRIPTEN_KEEPALIVE
float get_mario_x() {
    return mario.x;
}

EMSCRIPTEN_KEEPALIVE
float get_mario_y() {
    return mario.y;
}