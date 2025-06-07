#include "../includes/enemy_ai.h"
#include "../includes/so_long.h"  // For game state, map, lose_game etc.
#include <stdlib.h> // For malloc/free
#include <stdbool.h> // For bool type

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
    int r, c; // Loop iterators

    if (!sl->enemy_can_move_this_turn) {
        sl->enemy_can_move_this_turn = 1; // Set up for next turn
        return; // Enemies skip this turn
    }

    player_pos = find_char_on_map(sl, 'P', (t_point){0,0});
    if (player_pos.r == -1 && player_pos.c == -1) { // Player not found
        // No player, enemies do nothing. Could also be an error condition.
        sl->enemy_can_move_this_turn = 0; // Still toggle for next turn
        return;
    }

    for (r = 0; r < sl->lns; r++) {
        for (c = 0; sl->mapa[r][c] && sl->mapa[r][c] != '\n'; c++) {
            if (sl->mapa[r][c] == 'X') { // Found an enemy
                t_point enemy_pos = {r, c};
                t_point next_step;

                ft_printf("UPDATE_ENEMIES: Processing enemy at (%d,%d), player_pos=(%d,%d)\n",
                          enemy_pos.r, enemy_pos.c, player_pos.r, player_pos.c);

                next_step = find_path_bfs(sl, enemy_pos, player_pos);

                ft_printf("UPDATE_ENEMIES: BFS returned next_step=(%d,%d) for enemy at (%d,%d)\n",
                          next_step.r, next_step.c, enemy_pos.r, enemy_pos.c);

                if (next_step.r != -1 && next_step.c != -1) { // Valid step from BFS
                    int dist_r = next_step.r - enemy_pos.r;
                    int dist_c = next_step.c - enemy_pos.c;
                    int abs_dist_r = (dist_r > 0) ? dist_r : -dist_r;
                    int abs_dist_c = (dist_c > 0) ? dist_c : -dist_c;

                    if ((abs_dist_r + abs_dist_c) != 1) { // Teleport/non-adjacent check
                        ft_printf("DEBUG_TELEPORT_PREVENTION: next_step (%d,%d) is NOT adjacent to enemy_pos (%d,%d). Preventing move.\n",
                                  next_step.r, next_step.c, enemy_pos.r, enemy_pos.c);
                        // Enemy does not move if step is not adjacent
                    } else { // Step IS adjacent, proceed with move logic
                        if (next_step.r == player_pos.r && next_step.c == player_pos.c) { // Collision with player
                            ft_printf("UPDATE_ENEMIES: Enemy at (%d,%d) moving to player at (%d,%d). Game Over.\n",
                                      enemy_pos.r, enemy_pos.c, next_step.r, next_step.c);
                            lose_game(sl);
                            return; // Game over, exit function immediately
                        }

                        // Check if tile is valid to move onto (floor, collectible, exit)
                        if (sl->mapa[next_step.r][next_step.c] == '0' ||
                            sl->mapa[next_step.r][next_step.c] == 'C' ||
                            sl->mapa[next_step.r][next_step.c] == 'E') {

                            ft_printf("UPDATE_ENEMIES: Moving enemy from (%d,%d) to (%d,%d)\n",
                                      enemy_pos.r, enemy_pos.c, next_step.r, next_step.c);

                            sl->mapa[enemy_pos.r][enemy_pos.c] = '0';       // Clear old position
                            sl->mapa[next_step.r][next_step.c] = 'X';       // Set new position

                            // Render floor at old position
                            mlx_put_image_to_window(sl->mlx, sl->mlx_win, sl->imgs.flr.img,
                                sl->imgs.flr.h * enemy_pos.c, sl->imgs.flr.w * enemy_pos.r);
                            // Render enemy at new position
                            mlx_put_image_to_window(sl->mlx, sl->mlx_win, sl->imgs.enemy.img,
                                sl->imgs.flr.h * next_step.c + (sl->imgs.flr.h / 2 - sl->imgs.enemy.h / 2),
                                sl->imgs.flr.w * next_step.r + (sl->imgs.flr.w / 2 - sl->imgs.enemy.w / 2));
                        }
                        // else: BFS suggested a valid adjacent step, but it's blocked
                        // (e.g. by another enemy that moved there in the same turn, or a wall if BFS logic was imperfect).
                        // In this case, the enemy simply doesn't move.
                    }
                } // End if (valid step from BFS)
            } // End if (found an enemy 'X')
        } // End for (cols c)
    } // End for (rows r)

    sl->enemy_can_move_this_turn = 0; // Enemies skip next turn
} // End update_enemies function
