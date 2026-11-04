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

int solveOpt(int grid[9][9]) {
    nodes++;
    int bestRow = -1, bestCol = -1, bestCount = 10;
    for (int row = 0; row < 9; row++) {
        for (int col = 0; col < 9; col++) {
            if (grid[row][col] != 0) continue;
            int count = 0;
            for (int num = 1; num <= 9; num++) {
                if (isSafeBot(grid, row, col, num)) count++;
            }
            if (count == 0) return 0;
            if (count < bestCount) {
                bestCount = count;
                bestRow = row;
                bestCol = col;
            }
        }
    }
    if (bestRow == -1) return 1;
    for (int num = 1; num <= 9; num++) {
        if (!isSafeBot(grid, bestRow, bestCol, num)) continue;
        grid[bestRow][bestCol] = num;
        if (solveOpt(grid)) return 1;
        grid[bestRow][bestCol] = 0;
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
            solveOpt(work);
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

    FILE* f = fopen("bench_result.csv", "w");
    if (f) {
        fprintf(f, "level,puzzle,calls\n");
        for (int L = 0; L < 3; L++)
            for (int t = 0; t < N_PUZZLES; t++)
                fprintf(f, "%s,%d,%lld\n", names[L], t, perPuzzle[L][t]);
        fclose(f);
        printf("\nPer-puzzle data written to bench_result.csv\n");
    }
    return 0;
}