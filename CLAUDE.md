# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Run

```bash
# Compile
gcc nbi.c -o nbi

# Run (requires number of recommendations as argument)
./nbi 10
```

The program reads `training.txt` and `check.txt` from the current directory and outputs results to `result.txt`.

## Architecture

This is a Network-Based Inference (NBI) recommendation system implementing bipartite network projection, based on:
> Tao Zhou et al., "Bipartite network projection and personal recommendation", Phys. Rev. E 76, 046115 (2007)

### Algorithm Flow

1. **Network Construction** (`network_making`): Builds bidirectional bipartite graph from user-item interactions
2. **NBI Recommendation** (`nbi_recommendation`): Implements the full personalized recommendation flow for each user, including two-pass resource diffusion, item ranking, and generating the top-N recommendation list

### Core Data Structure

```c
typedef struct net {
    int node;          // Node ID
    int degree;        // Number of connections
    int* neighbor;     // Connected node IDs
    double value;      // Heat diffusion score
} bipatite;
```

The graph is represented as two arrays: `users` and `items`, each containing bipartite nodes with neighbor lists.

### Data Format

Both `training.txt` and `check.txt` use tab-separated format:
```
[UserID]    [ItemID]    [Timestamp]
```

Data is from MovieLens (http://www.grouplens.org/node/12).

### Key Functions

- `QuickSort` / `QuickSort_dual_desc`: Sorting utilities (dual version maintains parallel arrays in descending order)
- `BinarySearch` / `BinarySearch_raw`: Search in sorted node arrays vs raw int arrays
- `append`: Dynamic array manipulation using realloc
- `unique`: Count distinct values in array
