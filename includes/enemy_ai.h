#ifndef ENEMY_AI_H
# define ENEMY_AI_H

# include "so_long.h" // For t_so_long and map access

// Define a point structure for coordinates
typedef struct s_point {
    int r; // row
    int c; // col
} t_point;

// Define a node for BFS queue and path reconstruction
typedef struct s_path_node {
    t_point pos;
    t_point parent_pos; // To reconstruct path, tracks where we came from
    int     is_valid_parent; // Flag to check if parent_pos is set
} t_path_node;

// Function declarations
// Main BFS function: returns the next step towards target
t_point find_path_bfs(t_so_long *sl, t_point start, t_point target);

#endif
