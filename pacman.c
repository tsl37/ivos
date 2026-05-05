// pacman.c
// A freestanding bare-metal Pac-Man game with Ghosts

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;


__asm__(
    ".section .text.entry\n"
    ".global _start\n"
    "_start:\n"
    "  call main\n"
    "  ret\n"
);

// 1. Hardware I/O function
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

// 2. VGA Text Mode Definitions
#define VGA_ADDR 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

volatile uint16_t* vga_buffer = (uint16_t*)VGA_ADDR;

void draw_char(int x, int y, char c, uint8_t color) {
    if (x >= 0 && x < VGA_WIDTH && y >= 0 && y < VGA_HEIGHT) {
        vga_buffer[y * VGA_WIDTH + x] = (color << 8) | c;
    }
}

void draw_string(int x, int y, const char* str, uint8_t color) {
    while (*str) {
        draw_char(x++, y, *str++, color);
    }
}

void draw_number(int x, int y, int num, uint8_t color) {
    char buf[10];
    int i = 0;
    if (num == 0) buf[i++] = '0';
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    for (int j = i - 1; j >= 0; j--) {
        draw_char(x++, y, buf[j], color);
    }
}

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (0x0F << 8) | ' ';
    }
}

// 3. Game Map
#define MAP_WIDTH 21
#define MAP_HEIGHT 11

char map[MAP_HEIGHT][MAP_WIDTH] = {
    "####################",
    "#........#.........#",
    "#.##.###.#.###.##..#",
    "#..................#",
    "#.##.#.#####.#.##..#",
    "#....#.......#.....#",
    "####.###   ###.#####",
    "#.................##",
    "#.##.###.#.###.##..#",
    "#........#.........#",
    "####################"
};

void draw_map(int offset_x, int offset_y) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH - 1; x++) {
            char c = map[y][x];
            uint8_t color = 0x0F; // White text
            
            if (c == '#') color = 0x01;      // Blue walls
            else if (c == '.') color = 0x07; // Light grey dots
            
            draw_char(x + offset_x, y + offset_y, c, color);
        }
    }
}

// 4. Ghost Definitions
#define NUM_GHOSTS 3

typedef struct {
    int x;
    int y;
    uint8_t color;
} Ghost;

// 5. Main Game Loop
void main() {
    clear_screen();

    int pac_x = 9;
    int pac_y = 6;
    int score = 0;
    
    int map_off_x = 5;
    int map_off_y = 5;
    
    int tick_counter = 0;
    int game_over = 0;

    // Initialize 3 ghosts in the center area
    Ghost ghosts[NUM_GHOSTS] = {
        {1, 5, 0x0C}, // Red (Blinky)
        {15,  5, 0x0D}, // Pink (Pinky)
        {11, 1, 0x0B}  // Cyan (Inky)
    };

    while (1) {
        // If dead, halt everything and show game over message
        if (game_over) {
            draw_string(map_off_x + 4, map_off_y + 4, " GAME OVER! ", 0x4F); // White text on Red background
            for (volatile int i = 0; i < 8000; i++) {}
            break;
        }

        // Draw Map and Score
        draw_map(map_off_x, map_off_y);
        draw_string(map_off_x, map_off_y - 2, "SCORE:", 0x0F);
        draw_number(map_off_x + 7, map_off_y - 2, score, 0x0A); // Green score
        
        // Draw Ghosts (Using 'M' to look like a ghost)
        for (int g = 0; g < NUM_GHOSTS; g++) {
            draw_char(ghosts[g].x + map_off_x, ghosts[g].y + map_off_y, 'M', ghosts[g].color);
        }

        // Draw Pac-Man
        draw_char(pac_x + map_off_x, pac_y + map_off_y, 'C', 0x0E); 

        // Delay Loop (Controls the speed of the whole game)
        for (volatile int i = 0; i < 8000000; i++) {}
        tick_counter++;

        // --- PAC-MAN MOVEMENT (Keyboard Polling) ---
        uint8_t status = inb(0x64);
        if (status & 1) { 
            uint8_t scancode = inb(0x60); 
            if (!(scancode & 0x80)) { 
                int next_x = pac_x;
                int next_y = pac_y;

                if (scancode == 0x11 || scancode == 0x48) next_y--; // Up
                if (scancode == 0x1F || scancode == 0x50) next_y++; // Down
                if (scancode == 0x1E || scancode == 0x4B) next_x--; // Left
                if (scancode == 0x20 || scancode == 0x4D) next_x++; // Right

                if (map[next_y][next_x] != '#') {
                    pac_x = next_x;
                    pac_y = next_y;
                    
                    if (map[pac_y][pac_x] == '.') {
                        map[pac_y][pac_x] = ' '; 
                        score += 10;
                    }
                }
            }
        }

        // --- GHOST MOVEMENT AI ---
        // Move ghosts automatically every 15 ticks (Adjust this to make them faster/slower!)
        if (tick_counter % 15 == 0) {
            for (int g = 0; g < NUM_GHOSTS; g++) {
                int gx = ghosts[g].x;
                int gy = ghosts[g].y;
                
                // Calculate distance to Pac-Man
                int dx = pac_x - gx;
                int dy = pac_y - gy;
                int abs_dx = dx > 0 ? dx : -dx;
                int abs_dy = dy > 0 ? dy : -dy;

                int moved = 0;

                // Simple Pathfinding: Try closing the largest distance first
                if (abs_dx > abs_dy) {
                    if (dx > 0 && map[gy][gx + 1] != '#') { ghosts[g].x++; moved = 1; }
                    else if (dx < 0 && map[gy][gx - 1] != '#') { ghosts[g].x--; moved = 1; }
                    
                    // If blocked horizontally, try vertically
                    if (!moved) {
                        if (dy > 0 && map[gy + 1][gx] != '#') { ghosts[g].y++; moved = 1; }
                        else if (dy < 0 && map[gy - 1][gx] != '#') { ghosts[g].y--; moved = 1; }
                    }
                } else {
                    if (dy > 0 && map[gy + 1][gx] != '#') { ghosts[g].y++; moved = 1; }
                    else if (dy < 0 && map[gy - 1][gx] != '#') { ghosts[g].y--; moved = 1; }
                    
                    // If blocked vertically, try horizontally
                    if (!moved) {
                        if (dx > 0 && map[gy][gx + 1] != '#') { ghosts[g].x++; moved = 1; }
                        else if (dx < 0 && map[gy][gx - 1] != '#') { ghosts[g].x--; moved = 1; }
                    }
                }
                
                // If completely stuck in a corner, wiggle into any open space
                if (!moved) {
                    if (map[gy][gx + 1] != '#') ghosts[g].x++;
                    else if (map[gy][gx - 1] != '#') ghosts[g].x--;
                    else if (map[gy + 1][gx] != '#') ghosts[g].y++;
                    else if (map[gy - 1][gx] != '#') ghosts[g].y--;
                }
            }
        }

        // --- COLLISION DETECTION ---
        // Check if a ghost just touched Pac-Man
        for (int g = 0; g < NUM_GHOSTS; g++) {
            if (pac_x == ghosts[g].x && pac_y == ghosts[g].y) {
                game_over = 1;
            }
        }
    }
}