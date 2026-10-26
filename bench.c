#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef GAME_SOURCE
#define GAME_SOURCE "main.c"
#endif
#define main game_main
#include GAME_SOURCE
#undef main
#define N_PUZZLES 1000
#define SEED 123

int puzzles[3][N_PUZZLES][9][9];
long long nodes;

int solveBase(int grid[9][9], int row, int col) {
    nodes++;
    if (col == 9) {
        row++;
        col = 0;
    }
    if (row == 9) return 1;
    if (grid[row][col] > 0) return solveBase(grid, row, col + 1);
    for (int num = 1; num <= 9; num++) {
        if (isSafeBot(grid, row, col, num)) {
            grid[row][col] = num;
            if (solveBase(grid, row, col + 1)) return 1;
        }
        grid[row][col] = 0;
    }
    return 0;
}

int cmpLL(const void* a, const void* b) {
    long long x = *(long long*)a, y = *(long long*)b;
    return (x > y) - (x < y);
}

int main(void) {
    char* names[3] = { "easy", "medium", "hard" };
    long long perPuzzle[3][N_PUZZLES];
    double totalMs[3];
    int work[9][9];
    long long sorted[N_PUZZLES];

    srand(SEED);
    for (int L = 0; L < 3; L++)
        for (int t = 0; t < N_PUZZLES; t++) {
            createResSud(resSud);
            createSud(resSud, sud, L + 1);
            memcpy(puzzles[L][t], sud, sizeof(sud));
        }

    printf("N = %d puzzles per level, seed = %d\n\n", N_PUZZLES, SEED);
    printf("%-7s %12s %14s %14s %14s %14s\n", "level", "total, ms", "ms/puzzle", "avg calls", "median calls", "max calls");
    for (int L = 0; L < 3; L++) {
        long long sum = 0, mx = 0;
        clock_t t0 = clock();
        for (int t = 0; t < N_PUZZLES; t++) {
            memcpy(work, puzzles[L][t], sizeof(work));
            nodes = 0;
            solveBase(work, 0, 0);
            perPuzzle[L][t] = nodes;
            sum += nodes;
            if (nodes > mx) mx = nodes;
        }
        totalMs[L] = (double)(clock() - t0) * 1000.0 / CLOCKS_PER_SEC;

        memcpy(sorted, perPuzzle[L], sizeof(sorted));
        qsort(sorted, N_PUZZLES, sizeof(long long), cmpLL);
        printf("%-7s %12.1f %14.4f %14lld %14lld %14lld\n", names[L], totalMs[L], totalMs[L] / N_PUZZLES,
            sum / N_PUZZLES, sorted[N_PUZZLES / 2], mx);
    }

    FILE* f = fopen("bench_base.csv", "w");
    if (f) {
        fprintf(f, "level,puzzle,calls\n");
        for (int L = 0; L < 3; L++)
            for (int t = 0; t < N_PUZZLES; t++)
                fprintf(f, "%s,%d,%lld\n", names[L], t, perPuzzle[L][t]);
        fclose(f);
        printf("\nPer-puzzle data written to bench_base.csv\n");
    }
    return 0;
}
