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
2. **Heat Diffusion** (`heat_diffusion`): Two-pass heat propagation through the network to compute item scores
3. **Ranking** (`ranking`): Sorts items by heat diffusion scores (descending)
4. **Recommendation** (`recommendation`): Generates top-N recommendations per user, excludes already-purchased items, validates against test set

### Core Data Structure

```c
typedef struct net {
    int node;          // Node ID
    int degree;        // Number of connections
    int* neighbor;     // Connected node IDs
    double value;      // Heat diffusion score
} bipatite;
```

The graph is represented as two arrays: `left` (users) and `right` (items), each containing bipartite nodes with neighbor lists.

### Data Format

Both `training.txt` and `check.txt` use tab-separated format:
```
[UserID]    [ItemID]    [Timestamp]
```

Data is from MovieLens (http://www.grouplens.org/node/12).

### Key Functions

- `QuickSort` / `QuickSort_dual`: Sorting utilities (dual version maintains parallel arrays)
- `BinarySearch` / `BinarySearch_raw`: Search in sorted node arrays vs raw int arrays
- `append` / `list_del`: Dynamic array manipulation
- `unique`: Count distinct values in array
