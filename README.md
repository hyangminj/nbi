# NBI - Network-Based Inference Recommendation

A C implementation of the Network-Based Inference (NBI) algorithm for personalized recommendations using bipartite network projection.

## Algorithm

Based on the paper:
> Tao Zhou, Jie Ren, Matúš Medo, Yi-Cheng Zhang, "Bipartite network projection and personal recommendation", Phys. Rev. E 76, 046115 (2007).

The NBI algorithm performs personalized recommendations through a two-step resource diffusion process on a user-item bipartite network:

1. **Resource Allocation**: Assign initial resource (f=1) to items selected by the target user
2. **Items → Users Diffusion**: `f(user) = Σ f(item) / k(item)`
3. **Users → Items Diffusion**: `f'(item) = Σ f(user) / k(user)`
4. **Ranking**: Sort items by final score, excluding already selected items

## Build & Run

```bash
# Compile
gcc nbi.c -o nbi

# Run with N recommendations per user
./nbi <N>

# Example: generate top-10 recommendations
./nbi 10
```

## Data Format

Input files use tab-separated format:
```
UserID    ItemID    Timestamp
```

| File | Description |
|------|-------------|
| `training.txt` | Training data for building the network |
| `check.txt` | Test data for evaluation |
| `result.txt` | Output recommendations (generated) |

## Dataset

The included dataset is from [MovieLens](https://grouplens.org/datasets/movielens/), randomly split into training and test sets.

## Output

```
Recommendations: 10, Hits: 11044, Precision: 0.1829
```

- **Recommendations**: Number of items recommended per user
- **Hits**: Total correct predictions in test set
- **Precision**: Hit rate = Hits / (N × Users)

## References

- [Original Paper (Phys. Rev. E)](https://journals.aps.org/pre/abstract/10.1103/PhysRevE.76.046115)
- [arXiv Preprint](https://arxiv.org/abs/0707.0540)
- [MovieLens Dataset](https://grouplens.org/datasets/movielens/)
