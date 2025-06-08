/*
 * map_generator/generator.c
 * --------------------------
 * A simple procedural map generator for the so_long game.
 * It creates random mazes, places player, exit, and collectibles,
 * validates map playability (P->E, P->all C's, map rules),
 * and outputs the map to a .ber file.
 *
 * Compilation:
 *   gcc generator.c -o mapgen
 *
 * Usage:
 *   ./mapgen <width> <height> <filename.ber> [num_collectibles]
 *
 * Arguments:
 *   width:             The width of the map (e.g., 25). Min recommended: 5.
 *                      Odd numbers often produce better mazes with the current DFS.
 *   height:            The height of the map (e.g., 15). Min recommended: 5.
 *                      Odd numbers often produce better mazes.
 *   filename.ber:      The name of the .ber file to create (e.g., mymap.ber).
 *   num_collectibles:  (Optional) The number of collectibles to place.
 *                      Defaults to 5 if not specified. Must be at least 1.
 *
 * The generator attempts to ensure maps are playable by checking paths
 * and basic map rules. If generation fails validation, it will print
 * an error. It uses a retry loop (up to a set number of attempts) to find a playable configuration.
 */
#include <stdio.h>
#include <stdlib.h> // For malloc, free, rand, srand
#include <time.h>   // For srand(time(NULL))
#include <string.h> // For memset, strcpy (if needed later)

#define MAX_COLLECTIBLES_BUFFER_SIZE 256 // Buffer for validating collectibles
#define MAX_GENERATION_ATTEMPTS 50

// Define cell types for map characters
#define MAP_WALL '1'
#define MAP_PATH '0'

// Simple struct for points/coordinates
typedef struct {
    int r;
    int c;
} Point;

// --- Simple Queue Implementation for Points (Array-based) ---
#define MAX_QUEUE_SIZE_VALIDATION 256 // Max items in BFS queue for validation

typedef struct {
    Point items[MAX_QUEUE_SIZE_VALIDATION];
    int front;
    int rear;
    int count;
} PointQueue;

static void init_point_queue(PointQueue *q) {
    q->front = 0;
    q->rear = -1;
    q->count = 0;
}

static int is_point_queue_empty(PointQueue *q) {
    return q->count == 0;
}

static int is_point_queue_full(PointQueue *q) {
    return q->count == MAX_QUEUE_SIZE_VALIDATION;
}

static void enqueue_point(PointQueue *q, Point p) {
    if (is_point_queue_full(q)) {
        // Optional: fprintf(stderr, "Validation BFS Queue overflow\n");
        return;
    }
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE_VALIDATION;
    q->items[q->rear] = p;
    q->count++;
}

static Point dequeue_point(PointQueue *q) {
    // Assumes is_point_queue_empty is checked before calling.
    Point item = q->items[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE_VALIDATION;
    q->count--;
    return item;
}
// --- End Point Queue Implementation ---

// Helper for validation BFS: checks if a cell is valid to traverse
// visited_bfs is char** here, 0 for not visited, 1 for visited.
static int is_valid_for_validation_bfs(char cell_char, int r, int c, int height, int width, char **visited_bfs) {
    if (r < 0 || r >= height || c < 0 || c >= width) return 0; // Bounds
    if (visited_bfs[r][c]) return 0; // Already visited
    if (cell_char == MAP_WALL || cell_char == 'X') return 0; // Wall or Enemy blocks these specific validation paths
    return 1; // Path, Player, Exit, Collectible are traversable for P->E, P->C checks
}

// BFS to check if a path exists between start and target
// Returns 1 if path exists, 0 otherwise.
static int path_exists_bfs(char **grid, int height, int width, Point start, Point target) {
    if (start.r == target.r && start.c == target.c) return 1; // Start is target

    char **visited_bfs = (char **)malloc(height * sizeof(char *));
    if (!visited_bfs) return 0; // Malloc failure
    for (int i = 0; i < height; i++) {
        visited_bfs[i] = (char *)calloc(width, sizeof(char));
        if (!visited_bfs[i]) {
            for (int k = 0; k < i; k++) free(visited_bfs[k]);
            free(visited_bfs);
            return 0; // Malloc failure
        }
    }

    PointQueue point_q;
    init_point_queue(&point_q);
    enqueue_point(&point_q, start);

    visited_bfs[start.r][start.c] = 1;

    int dr[] = {-1, 1, 0, 0}; // Change in row (Up, Down)
    int dc[] = {0, 0, -1, 1}; // Change in col (Left, Right)
    int path_found = 0;

    while (!is_point_queue_empty(&point_q)) {
        Point curr = dequeue_point(&point_q);

        if (curr.r == target.r && curr.c == target.c) {
            path_found = 1;
            break;
        }

        for (int i = 0; i < 4; i++) {
            int next_r = curr.r + dr[i];
            int next_c = curr.c + dc[i];

            // Need to check bounds BEFORE accessing grid[next_r][next_c]
            if (next_r >= 0 && next_r < height && next_c >= 0 && next_c < width) {
               if (is_valid_for_validation_bfs(grid[next_r][next_c], next_r, next_c, height, width, visited_bfs)) {
                    visited_bfs[next_r][next_c] = 1;
                    enqueue_point(&point_q, (Point){next_r, next_c});
                }
            }
        }
    }

    for (int i = 0; i < height; i++) free(visited_bfs[i]);
    free(visited_bfs);
    return path_found;
}

// Main validation function
static int is_map_playable(char **grid, int height, int width, int num_c_expected) {
    Point player_pos = {-1, -1};
    Point exit_pos = {-1, -1};
    Point collectibles[MAX_COLLECTIBLES_BUFFER_SIZE];
    int num_c_found = 0;
    int player_found = 0;
    int exit_found = 0;

    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            if (grid[r][c] == 'P') {
                if (player_found) {
                    fprintf(stderr, "Validation Error: Multiple players found.\n");
                    return 0;
                }
                player_pos = (Point){r, c};
                player_found = 1;
            } else if (grid[r][c] == 'E') {
                if (!exit_found) exit_pos = (Point){r,c}; // Store first one for path check
                exit_found = 1; // Allow multiple exits conceptually, path to one is fine
            } else if (grid[r][c] == 'C') {
                if (num_c_found < MAX_COLLECTIBLES_BUFFER_SIZE) {
                    collectibles[num_c_found++] = (Point){r, c};
                } else {
                    fprintf(stderr, "Warning: Found more collectibles than buffer can hold for validation (%d).\n", MAX_COLLECTIBLES_BUFFER_SIZE);
                }
            } else if (grid[r][c] == MAP_PATH) { /* path is fine */ }
            else if (grid[r][c] == MAP_WALL) { /* wall is fine */ }
            else if (grid[r][c] == 'X') { /* enemy is fine for char validation */ }
            else { // Any other character is invalid
                fprintf(stderr, "Validation Error: Invalid character '%c' at (%d,%d).\n", grid[r][c], r, c);
                return 0;
            }
            // Wall enclosure check (basic: check borders)
            if ((r == 0 || r == height - 1 || c == 0 || c == width - 1) && grid[r][c] != MAP_WALL) {
                fprintf(stderr, "Validation Error: Map not enclosed by walls at (%d,%d).\n", r,c);
                return 0;
            }
        }
    }

    if (!player_found || !exit_found || num_c_found != num_c_expected) {
         fprintf(stderr, "Validation Error: P found (%d), E found (%d), C found (%d vs %d expected).\n", player_found, exit_found, num_c_found, num_c_expected);
        return 0; // Missing P, E, or wrong number of C
    }

    // Pathfinding: P to E
    if (!path_exists_bfs(grid, height, width, player_pos, exit_pos)) {
        fprintf(stderr, "Validation Error: Path from Player to Exit not found.\n");
        return 0;
    }
    // Pathfinding: P to all C's
    for (int i = 0; i < num_c_found; i++) { // Iterate based on actual C's found and counted
        if (!path_exists_bfs(grid, height, width, player_pos, collectibles[i])) {
            fprintf(stderr, "Validation Error: Path from Player to Collectible at (%d,%d) not found.\n", collectibles[i].r, collectibles[i].c);
            return 0;
        }
    }
    return 1; // Playable
}

// Writes the grid to the specified file
static int write_map_to_file(char **grid, int height, int width, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file for writing");
        return 1; // Error
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (fputc(grid[i][j], file) == EOF) {
                perror("Error writing character to file");
                fclose(file);
                return 1;
            }
        }
        // Add newline after each row
        if (fputc('\n', file) == EOF) {
            perror("Error writing newline to file");
            fclose(file);
            return 1;
        }
    }

    if (fclose(file) == EOF) {
        perror("Error closing file");
        return 1; // Error
    }
    printf("Map successfully written to %s\n", filename);
    return 0; // Success
}

// Allocates and initializes the grid with walls
char **create_grid(int height, int width) {
    char **grid = (char **)malloc(height * sizeof(char *));
    if (!grid) return NULL;
    for (int i = 0; i < height; i++) {
        grid[i] = (char *)malloc(width * sizeof(char));
        if (!grid[i]) {
            // Free previously allocated rows
            for (int j = 0; j < i; j++) free(grid[j]);
            free(grid);
            return NULL;
        }
        // Initialize all cells to walls
        for (int j = 0; j < width; j++) {
            grid[i][j] = MAP_WALL;
        }
    }
    return grid;
}

void free_grid(char **grid, int height) {
    if (!grid) return;
    for (int i = 0; i < height; i++) {
        free(grid[i]);
    }
    free(grid);
}

Point *get_floor_tiles(char **grid, int height, int width, int *num_floor_tiles) {
    *num_floor_tiles = 0;
    // First pass: count floor tiles
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            if (grid[r][c] == MAP_PATH) {
                (*num_floor_tiles)++;
            }
        }
    }

    if (*num_floor_tiles == 0) return NULL;

    Point *floor_tiles_array = (Point *)malloc(*num_floor_tiles * sizeof(Point));
    if (!floor_tiles_array) {
        *num_floor_tiles = 0;
        return NULL;
    }

    // Second pass: store coordinates
    int current_tile = 0;
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            if (grid[r][c] == MAP_PATH) {
                floor_tiles_array[current_tile].r = r;
                floor_tiles_array[current_tile].c = c;
                current_tile++;
            }
        }
    }
    return floor_tiles_array;
}

// Fisher-Yates shuffle for randomizing neighbor order
void shuffle_directions(int dirs[][2], int count) {
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp_r = dirs[i][0];
        int temp_c = dirs[i][1];
        dirs[i][0] = dirs[j][0];
        dirs[i][1] = dirs[j][1];
        dirs[j][0] = temp_r;
        dirs[j][1] = temp_c;
    }
}

void shuffle_points(Point points[], int count) {
    if (count <= 1) return;
    for (int i = count - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Point temp = points[i];
        points[i] = points[j];
        points[j] = temp;
    }
}

// Randomized DFS function
void generate_maze_dfs(char **grid, int height, int width, int r, int c, char **visited) {
    visited[r][c] = 1; // Mark as visited
    grid[r][c] = MAP_PATH; // Carve path at current cell

    // Neighbors: Up, Right, Down, Left (row, col offsets)
    // Order will be shuffled to ensure randomness
    int directions[4][2] = {{-2, 0}, {0, 2}, {2, 0}, {0, -2}};
    shuffle_directions(directions, 4);

    for (int i = 0; i < 4; i++) {
        int next_r = r + directions[i][0];
        int next_c = c + directions[i][1];

        // Check bounds for next_r, next_c
        if (next_r > 0 && next_r < height - 1 &&
            next_c > 0 && next_c < width - 1 &&
            !visited[next_r][next_c]) {

            // Carve path to the wall between current and next cell
            grid[r + directions[i][0] / 2][c + directions[i][1] / 2] = MAP_PATH;

            generate_maze_dfs(grid, height, width, next_r, next_c, visited);
        }
    }
}

// Forward declarations or definitions will go here

int main(int argc, char *argv[]) {
    // Basic argument check (example)
    if (argc < 4) { // Essential args: width, height, filename
        fprintf(stderr, "Usage: %s <width> <height> <filename.ber> [num_collectibles] [num_enemies]\n", argv[0]);
        fprintf(stderr, "  num_collectibles: defaults to 5. Min 1.\n");
        fprintf(stderr, "  num_enemies: defaults to 3. Min 0.\n");
        return 1;
    }

    // Seed random number generator
    srand(time(NULL));

    // Argument parsing
    int width = atoi(argv[1]);
    int height = atoi(argv[2]);
    const char *filename = argv[3];
    int num_collectibles = 5; // Default
    if (argc > 4) {
        num_collectibles = atoi(argv[4]);
        if (num_collectibles < 1) {
            fprintf(stderr, "Warning: Number of collectibles must be at least 1. Setting to 1.\n");
            num_collectibles = 1;
        }
    }
    int num_enemies = 3; // Default for enemies
    if (argc > 5) {
        num_enemies = atoi(argv[5]);
        if (num_enemies < 0) {
            fprintf(stderr, "Warning: Number of enemies cannot be negative. Setting to 0.\n");
            num_enemies = 0;
        }
    }

    printf("Generator Parameters:\n");
    printf("  Width: %d\n", width);
    printf("  Height: %d\n", height);
    printf("  Output File: %s\n", filename);
    printf("  Collectibles: %d\n", num_collectibles);
    printf("  Enemies: %d\n", num_enemies);
    printf("------------------------------------\n");

    if (width < 5 || height < 5) {
        fprintf(stderr, "Error: Width and height must be at least 5.\n");
        return 1;
    }

    char **grid = NULL;
    char **visited = NULL;
    Point *floor_tiles = NULL;
    int playable_map_generated = 0;
    int attempt;

    for (attempt = 0; attempt < MAX_GENERATION_ATTEMPTS; attempt++) {
        printf("Generation Attempt %d/%d...\n", attempt + 1, MAX_GENERATION_ATTEMPTS);

        grid = create_grid(height, width);
        if (!grid) {
            fprintf(stderr, "Error: Could not create grid on attempt %d.\n", attempt + 1);
            continue;
        }

        visited = (char **)malloc(height * sizeof(char *));
        if (!visited) {
            fprintf(stderr, "Error: Malloc failed for visited rows on attempt %d.\n", attempt + 1);
            free_grid(grid, height); grid = NULL;
            continue;
        }
        int visited_row_allocated_until = -1;
        for (int i = 0; i < height; i++) {
            visited[i] = (char *)calloc(width, sizeof(char));
            if (!visited[i]) {
                fprintf(stderr, "Error: Calloc failed for visited col on attempt %d, row %d.\n", attempt + 1, i);
                for(int j=0; j<=visited_row_allocated_until; j++) free(visited[j]);
                free(visited); visited = NULL;
                free_grid(grid, height); grid = NULL;
                goto next_attempt;
            }
            visited_row_allocated_until = i;
        }

        generate_maze_dfs(grid, height, width, 1, 1, visited);

        int num_floor_tiles = 0;
        floor_tiles = get_floor_tiles(grid, height, width, &num_floor_tiles);

        int required_tiles = 1 + 1 + num_collectibles + num_enemies;
        if (!floor_tiles || num_floor_tiles < required_tiles) {
            fprintf(stderr, "Attempt %d: Not enough floor tiles (%d) for P, E, %d C, %d X. Required: %d.\n",
                    attempt + 1, num_floor_tiles, num_collectibles, num_enemies, required_tiles);
            if (floor_tiles) free(floor_tiles); floor_tiles = NULL;
            for (int i = 0; i < height; i++) free(visited[i]); // Visited is fully allocated here
            free(visited); visited = NULL;
            free_grid(grid, height); grid = NULL;
            continue;
        }

        shuffle_points(floor_tiles, num_floor_tiles);

        Point player_pos = floor_tiles[0];
        grid[player_pos.r][player_pos.c] = 'P';
        grid[floor_tiles[1].r][floor_tiles[1].c] = 'E'; // Directly use, exit_pos var not strictly needed

        if (num_collectibles > 0) {
            // printf("Placing %d collectibles.\n", num_collectibles); // Optional, already in params
            for (int i = 0; i < num_collectibles; i++) {
                Point collectible_pos = floor_tiles[2 + i];
                grid[collectible_pos.r][collectible_pos.c] = 'C';
            }
        }

        if (num_enemies > 0) {
            // printf("Placing %d enemies.\n", num_enemies); // Optional
            for (int i = 0; i < num_enemies; i++) {
                Point enemy_pos = floor_tiles[2 + num_collectibles + i];
                grid[enemy_pos.r][enemy_pos.c] = 'X';
            }
        }

        free(floor_tiles); floor_tiles = NULL;

        if (is_map_playable(grid, height, width, num_collectibles)) {
            printf("Playable map generated on attempt %d!\n", attempt + 1);
            // Temporary: Print grid to console here to see the valid one
            printf("Generated Playable Maze (Attempt %d):\n", attempt + 1);
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++) {
                    printf("%c", grid[i][j]);
                }
                printf("\n");
            }
            playable_map_generated = 1;
            for (int i = 0; i < height; i++) free(visited[i]);
            free(visited); visited = NULL;
            break;
        } else {
            printf("Attempt %d: Generated map is NOT playable (check stderr for details from is_map_playable).\n", attempt + 1);
            for (int i = 0; i < height; i++) free(visited[i]);
            free(visited); visited = NULL;
            free_grid(grid, height); grid = NULL;
        }
    next_attempt:;
    }

    if (playable_map_generated) {
        if (write_map_to_file(grid, height, width, filename) == 0) {
            // Success message is in write_map_to_file
        } else {
            fprintf(stderr, "Failed to write map to file %s.\n", filename);
        }
        free_grid(grid, height);
    } else {
        fprintf(stderr, "Failed to generate a playable map after %d attempts.\n", MAX_GENERATION_ATTEMPTS);
        if (grid) free_grid(grid, height);
        if (visited) {
             for (int i = 0; i < height; i++) if(visited[i]) free(visited[i]);
             free(visited);
        }
        if (floor_tiles) free(floor_tiles);
        return 1;
    }

    printf("Map generator finished.\n"); // Changed from "finished maze generation phase"
    return 0;
}
