#include "../includes/enemy_ai.h"
#include "../includes/so_long.h"  // For game state, map, lose_game etc.
#include <stdlib.h> // For malloc/free
#include <stdbool.h> // For bool type

#define MAX_ENEMIES_TRACKED 50 // Or another reasonable limit

// --- Simple Queue Implementation (Array-based) ---
#define MAX_QUEUE_SIZE 256 // Adjust if maps can be much larger

typedef struct s_queue {
    t_path_node items[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int count;
} t_queue;

static void init_queue(t_queue *q) {
    q->front = 0;
    q->rear = -1;
    q->count = 0;
}

static bool is_queue_empty(t_queue *q) {
    return q->count == 0;
}

static bool is_queue_full(t_queue *q) {
    return q->count == MAX_QUEUE_SIZE;
}

static void enqueue(t_queue *q, t_path_node node) {
    if (is_queue_full(q)) {
        // ft_printf("Error: Queue overflow\n"); // Optional: error message
        return;
    }
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->items[q->rear] = node;
    q->count++;
}

static t_path_node dequeue(t_queue *q) {
    // Does not handle underflow explicitly here for brevity,
    // assuming is_queue_empty is checked before calling.
    t_path_node item = q->items[q->front];
    q->front = (q->front + 1) % MAX_QUEUE_SIZE;
    q->count--;
    return item;
}
// --- End Queue Implementation ---

// Helper to check if a cell is valid for BFS exploration
static bool is_valid_for_bfs(t_so_long *sl, int r, int c, bool **visited, t_point original_enemy_pos) {
    // Check bounds
    int map_cols = 0;
    if (sl->lns > 0 && sl->mapa[0]) {
        map_cols = ft_strlen(sl->mapa[0]);
        if (sl->mapa[0][map_cols-1] == '\n') {
             map_cols--;
        }
    }
    if (r < 0 || r >= sl->lns || c < 0 || c >= map_cols ) {
        return false;
    }
    // Already visited
    if (visited[r][c]) {
        return false;
    }
    // Is it a wall?
    if (sl->mapa[r][c] == '1') {
        return false;
    }
    // Is it another enemy (but not the current enemy's starting position)?
    if (sl->mapa[r][c] == 'X' && (r != original_enemy_pos.r || c != original_enemy_pos.c)) {
        return false;
    }
    // Valid to explore (can be '0', 'C', 'E', 'P', or current enemy's start pos)
    return true;
}

// Main BFS function
t_point find_path_bfs(t_so_long *sl, t_point start, t_point target) {
    t_point no_path = {-1, -1};

    if (start.r == target.r && start.c == target.c) {
        return no_path;
    }

    int rows = sl->lns;
    int cols = 0;
    if (rows > 0 && sl->mapa[0]) {
        cols = ft_strlen(sl->mapa[0]);
        if (sl->mapa[0][cols-1] == '\n') {
             cols--;
        }
    }
    if (cols == 0) return no_path;

    bool **visited = malloc(rows * sizeof(bool *));
    if (!visited) return no_path;
    for (int i = 0; i < rows; i++) {
        visited[i] = calloc(cols, sizeof(bool));
        if (!visited[i]) {
            for (int k = 0; k < i; k++) free(visited[k]);
            free(visited);
            return no_path;
        }
    }

    t_path_node **parents_map = malloc(rows * sizeof(t_path_node *));
    if (!parents_map) {
        for (int i = 0; i < rows; i++) free(visited[i]);
        free(visited);
        return no_path;
    }
    for (int i = 0; i < rows; i++) {
        parents_map[i] = malloc(cols * sizeof(t_path_node));
        if (!parents_map[i]) {
            for (int k = 0; k < i; k++) free(parents_map[k]);
            free(parents_map);
            for (int k_vis = 0; k_vis < rows; k_vis++) free(visited[k_vis]);
            free(visited);
            return no_path;
        }
        for (int j = 0; j < cols; j++) {
            parents_map[i][j].is_valid_parent = false;
        }
    }

    t_queue q;
    init_queue(&q);

    t_path_node start_node = {start, {-1,-1}, false};
    enqueue(&q, start_node);
    visited[start.r][start.c] = true;
    parents_map[start.r][start.c] = start_node;

    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    t_point first_step_to_target = no_path;

    while (!is_queue_empty(&q)) {
        t_path_node current_node = dequeue(&q);
        t_point current_pos = current_node.pos;

        if (current_pos.r == target.r && current_pos.c == target.c) {
            t_point trace_back_pos = target;
            while (parents_map[trace_back_pos.r][trace_back_pos.c].is_valid_parent &&
                   (parents_map[trace_back_pos.r][trace_back_pos.c].parent_pos.r != start.r ||
                    parents_map[trace_back_pos.r][trace_back_pos.c].parent_pos.c != start.c)) {
                trace_back_pos = parents_map[trace_back_pos.r][trace_back_pos.c].parent_pos;
            }
            first_step_to_target = trace_back_pos;
            goto cleanup;
        }

        for (int i = 0; i < 4; i++) {
            int next_r = current_pos.r + dr[i];
            int next_c = current_pos.c + dc[i];

            if (is_valid_for_bfs(sl, next_r, next_c, visited, start)) {
                visited[next_r][next_c] = true;

                t_path_node neighbor_node;
                neighbor_node.pos.r = next_r;
                neighbor_node.pos.c = next_c;
                neighbor_node.parent_pos = current_pos;
                neighbor_node.is_valid_parent = true;

                enqueue(&q, neighbor_node);
                parents_map[next_r][next_c] = neighbor_node;
            }
        }
    }

cleanup:
    for (int i = 0; i < rows; i++) {
        if (visited[i]) free(visited[i]);
        if (parents_map[i]) free(parents_map[i]);
    }
    if (visited) free(visited);
    if (parents_map) free(parents_map);

    ft_printf("BFS: start=(%d,%d), target=(%d,%d) -> first_step=(%d,%d)\n",
              start.r, start.c, target.r, target.c,
              first_step_to_target.r, first_step_to_target.c);
    return first_step_to_target;
}

// Helper function to find a character on the map
static t_point find_char_on_map(t_so_long *sl, char target_char, t_point start_search_from) {
    for (int r = start_search_from.r; r < sl->lns; r++) {
        int c_start = (r == start_search_from.r) ? start_search_from.c : 0;
        for (int c = c_start; sl->mapa[r][c] && sl->mapa[r][c] != '\n'; c++) {
            if (sl->mapa[r][c] == target_char) {
                return (t_point){r, c};
            }
        }
    }
    return (t_point){-1, -1}; // Not found
}

void update_enemies(t_so_long *sl) {
    t_point player_pos;
    t_point enemy_positions[MAX_ENEMIES_TRACKED];
    int num_enemies_found = 0;
    int r, c, i; // Loop iterators

    // Cooldown logic: if it's not the enemies' turn to move, set up for next turn and exit.
    if (!sl->enemy_can_move_this_turn) {
        sl->enemy_can_move_this_turn = 1;
        return;
    }

    // Find player's current position once.
    player_pos = find_char_on_map(sl, 'P', (t_point){0,0});
    if (player_pos.r == -1 && player_pos.c == -1) { // Player not found
        sl->enemy_can_move_this_turn = 0; // Toggle for next turn even if no player
        return; // No player, enemies do nothing.
    }

    // 1. Collect all current enemy positions.
    for (r = 0; r < sl->lns; r++) {
        for (c = 0; sl->mapa[r][c] && sl->mapa[r][c] != '\n'; c++) {
            if (sl->mapa[r][c] == 'X') {
                if (num_enemies_found < MAX_ENEMIES_TRACKED) {
                    enemy_positions[num_enemies_found].r = r;
                    enemy_positions[num_enemies_found].c = c;
                    num_enemies_found++;
                } else {
                    // Optional: Log if too many enemies for the tracking array
                    ft_printf("WARNING: Exceeded MAX_ENEMIES_TRACKED (%d). Some enemies may not move.\n", MAX_ENEMIES_TRACKED);
                    break; // Stop collecting if array is full for this row
                }
            }
        }
        if (num_enemies_found >= MAX_ENEMIES_TRACKED && sl->mapa[r][c] && sl->mapa[r][c] != '\n') {
             // If inner loop broke due to full array, break outer too.
            break;
        }
    }

    // 2. Process each found enemy from the collected list.
    for (i = 0; i < num_enemies_found; i++) {
        t_point enemy_pos = enemy_positions[i]; // Original position for this enemy this turn
        t_point next_step;

        // Important: Check if the enemy at its original position still exists.
        // It might have been "killed" or moved by another enemy if interactions were more complex
        // (not an issue in current design where enemies don't affect each other, but good practice).
        // For this game, the main concern is if it was the player, which is handled by lose_game return.
        // So, we assume sl->mapa[enemy_pos.r][enemy_pos.c] is still 'X' unless game ended.
        // If an enemy moved into another enemy's spot, that's more complex (not handled yet).
        // The critical part is that `enemy_pos` is from the *start-of-turn snapshot*.

        ft_printf("UPDATE_ENEMIES: Processing enemy originally at (%d,%d), player_pos=(%d,%d)\n",
                  enemy_pos.r, enemy_pos.c, player_pos.r, player_pos.c);

        next_step = find_path_bfs(sl, enemy_pos, player_pos);

        ft_printf("UPDATE_ENEMIES: BFS returned next_step=(%d,%d) for enemy originally at (%d,%d)\n",
                  next_step.r, next_step.c, enemy_pos.r, enemy_pos.c);

        if (next_step.r != -1 && next_step.c != -1) { // Valid step from BFS
            int dist_r = next_step.r - enemy_pos.r;
            int dist_c = next_step.c - enemy_pos.c;
            int abs_dist_r = (dist_r > 0) ? dist_r : -dist_r;
            int abs_dist_c = (dist_c > 0) ? dist_c : -dist_c;

            if ((abs_dist_r + abs_dist_c) != 1) { // Teleport/non-adjacent check
                ft_printf("DEBUG_TELEPORT_PREVENTION: next_step (%d,%d) is NOT adjacent to enemy_pos (%d,%d). Preventing move.\n",
                          next_step.r, next_step.c, enemy_pos.r, enemy_pos.c);
            } else { // Step IS adjacent, proceed with move logic
                if (next_step.r == player_pos.r && next_step.c == player_pos.c) { // Collision with player
                    ft_printf("UPDATE_ENEMIES: Enemy originally at (%d,%d) moving to player at (%d,%d). Game Over.\n",
                              enemy_pos.r, enemy_pos.c, next_step.r, next_step.c);
                    lose_game(sl);
                    return; // Game over, exit function immediately
                }

                // Check if tile is valid to move onto (floor, collectible, exit)
                // Also ensure the target tile isn't now occupied by another enemy that moved earlier in this same turn's enemy phase.
                if ((sl->mapa[next_step.r][next_step.c] == '0' ||
                     sl->mapa[next_step.r][next_step.c] == 'C' ||
                     sl->mapa[next_step.r][next_step.c] == 'E')) {
                    // AND sl->mapa[enemy_pos.r][enemy_pos.c] must still be 'X' (the one we are processing)
                    // This check is important because the map is live.
                    if (sl->mapa[enemy_pos.r][enemy_pos.c] != 'X') {
                         ft_printf("UPDATE_ENEMIES: Enemy at (%d,%d) already moved or was altered. Skipping move to (%d,%d).\n", enemy_pos.r, enemy_pos.c, next_step.r, next_step.c);
                         continue; // This specific enemy might have been involved in a previous enemy's move (e.g. got overwritten)
                    }

                    ft_printf("UPDATE_ENEMIES: Moving enemy from (%d,%d) to (%d,%d)\n",
                              enemy_pos.r, enemy_pos.c, next_step.r, next_step.c);

                    sl->mapa[enemy_pos.r][enemy_pos.c] = '0';       // Clear original position
                    sl->mapa[next_step.r][next_step.c] = 'X';       // Set new position

                    // Render floor at original position
                    mlx_put_image_to_window(sl->mlx, sl->mlx_win, sl->imgs.flr.img,
                        sl->imgs.flr.h * enemy_pos.c, sl->imgs.flr.w * enemy_pos.r);
                    // Render enemy at new position
                    mlx_put_image_to_window(sl->mlx, sl->mlx_win, sl->imgs.enemy.img,
                        sl->imgs.flr.h * next_step.c + (sl->imgs.flr.h / 2 - sl->imgs.enemy.h / 2),
                        sl->imgs.flr.w * next_step.r + (sl->imgs.flr.w / 2 - sl->imgs.enemy.w / 2));
                }
                // else: BFS suggested an adjacent step, but it's blocked by wall / other enemy, or invalid tile.
                // Enemy doesn't move.
            }
        } // End if (valid step from BFS)
    } // End for (processing collected enemies i)

    sl->enemy_can_move_this_turn = 0; // Enemies skip next player turn
} // End update_enemies function
