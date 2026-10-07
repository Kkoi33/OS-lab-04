# Exercise 2 — `fork()` in a loop (`ex3.c`)

`ex3.c` calls `fork()` `n` times (n is taken from the command line) and sleeps
5 seconds after each call. It was run in the background (`./ex3 n &`) and
`pstree` was called after every iteration.

## Run with n = 3

```
--- pstree after ~1s ---
ex3(684)---ex3(686)
--- pstree after ~6s ---
ex3(684)-+-ex3(686)---ex3(690)
         `-ex3(689)
--- pstree after ~11s ---
ex3(684)-+-ex3(686)-+-ex3(690)---ex3(698)
         |          `-ex3(695)
         |-ex3(689)---ex3(696)
         `-ex3(694)
```

**8 processes in total** (the original one + 7 created).

## Run with n = 5

`pstree` during the last iteration:

```
ex3(706)-+-ex3(708)-+-ex3(712)-+-ex3(718)-+-ex3(730)---ex3(751)
         |          |          |          `-ex3(750)
         |          |          |-ex3(724)---ex3(739)
         |          |          `-ex3(738)
         |          |-ex3(719)-+-ex3(727)---ex3(745)
         |          |          `-ex3(742)
         |          |-ex3(728)---ex3(747)
         |          `-ex3(743)
         |-ex3(711)-+-ex3(717)-+-ex3(723)---ex3(737)
         |          |          `-ex3(736)
         |          |-ex3(729)---ex3(749)
         |          `-ex3(748)
         |-ex3(716)-+-ex3(726)---ex3(746)
         |          `-ex3(744)
         |-ex3(725)---ex3(741)
         `-ex3(740)
```

**32 processes in total** (the original one + 31 created).

## Explanation

After `fork()` both the parent and the child continue executing the same loop
from the same iteration. So on every iteration **every existing process**
calls `fork()` once and the number of processes doubles:

| iteration | processes after it |
|-----------|--------------------|
| 1         | 2                  |
| 2         | 4                  |
| 3         | 8                  |
| 4         | 16                 |
| 5         | 32                 |

In general the program ends up with `2^n` processes, i.e. `2^n - 1` new
processes are created:

* n = 3 → 2³ = 8 processes (7 created);
* n = 5 → 2⁵ = 32 processes (31 created).

`pstree` shows this as a binomial tree: the original process has `n` children,
the child created on iteration `i` has `n - i` children of its own, and so on.
Because of `sleep(5)` we can see the tree grow step by step: each snapshot is
taken during the next iteration and doubles the previous one.

**Difference between the runs:** the growth is exponential, not linear.
Two extra iterations (3 → 5) give 4 times more processes (8 → 32), and
the program also runs longer (15 s vs 25 s). With a large `n` such a program
quickly exhausts the process limit (a "fork bomb").

After the loop every process calls `wait()` for its own children, so no
zombie or orphan processes are left when the program finishes.
