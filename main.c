#define _CRT_SECURE_NO_WARNINGS
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <Windows.h>

#define MENU 0
#define PLAYING 1
#define GAMEOVER 2
#define WIN 3
#define MIN_CLUES_PER_LINE 1

int resSud[9][9];
int sud[9][9];
int draftSud[9][9];
int hints = 2;
int error = 0;
int selectedDigit = 0;
int currentState = MENU;
int hitMode = 0;
int draftMode = 0;
int botUsed = 0;
int animate = 1;
long long nodes = 0;

int isSafeBot(int grid[9][9], int row, int col, int num) {
    for (int x = 0; x <= 8; x++) {
        if (grid[row][x] == num) return 0;
        if (grid[x][col] == num) return 0;
    }

    int startRow = row - row % 3, startCol = col - col % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grid[i + startRow][j + startCol] == num) return 0;
        }
    }
    return 1;
}

void createResSud(int resSud[9][9]) {
    int base[9][9] = {
        {1, 2, 3, 4, 5, 6, 7, 8, 9}, {4, 5, 6, 7, 8, 9, 1, 2, 3}, {7, 8, 9, 1, 2, 3, 4, 5, 6},
        {2, 3, 4, 5, 6, 7, 8, 9, 1}, {5, 6, 7, 8, 9, 1, 2, 3, 4}, {8, 9, 1, 2, 3, 4, 5, 6, 7},
        {3, 4, 5, 6, 7, 8, 9, 1, 2}, {6, 7, 8, 9, 1, 2, 3, 4, 5}, {9, 1, 2, 3, 4, 5, 6, 7, 8}
    };
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            resSud[i][j] = base[i][j];
        }
    }

    int iterations = 25 + rand() % 20;
    for (int i = 0; i < iterations; i++) {
        int mix_type = rand() % 5;
        if (mix_type == 0) {
            for (int r = 0; r < 9; r++)
                for (int c = r + 1; c < 9; c++) {
                    int temp = resSud[r][c];
                    resSud[r][c] = resSud[c][r];
                    resSud[c][r] = temp;
                }
        }
        else if (mix_type == 1) {
            int area = rand() % 3;
            int line1 = area * 3 + rand() % 3, line2 = area * 3 + rand() % 3;
            for (int c = 0; c < 9; c++) {
                int temp = resSud[line1][c];
                resSud[line1][c] = resSud[line2][c];
                resSud[line2][c] = temp;
            }
        }
        else if (mix_type == 2) {
            int area = rand() % 3;
            int col1 = area * 3 + rand() % 3, col2 = area * 3 + rand() % 3;
            for (int r = 0; r < 9; r++) {
                int temp = resSud[r][col1];
                resSud[r][col1] = resSud[r][col2];
                resSud[r][col2] = temp;
            }
        }
        else if (mix_type == 3) {
            int a1 = rand() % 3, a2 = rand() % 3;
            for (int r = 0; r < 3; r++) {
                for (int c = 0; c < 9; c++) {
                    int temp = resSud[a1 * 3 + r][c];
                    resSud[a1 * 3 + r][c] = resSud[a2 * 3 + r][c];
                    resSud[a2 * 3 + r][c] = temp;
                }
            }
        }
        else if (mix_type == 4) {
            int a1 = rand() % 3, a2 = rand() % 3;
            for (int c = 0; c < 3; c++) {
                for (int r = 0; r < 9; r++) {
                    int temp = resSud[r][a1 * 3 + c];
                    resSud[r][a1 * 3 + c] = resSud[r][a2 * 3 + c];
                    resSud[r][a2 * 3 + c] = temp;
                }
            }
        }
    }
}

#define MIN_CLUES_PER_LINE 1

int canRemoveCell(int grid[9][9], int r, int c) {
    int rowCnt = 0, colCnt = 0;
    for (int k = 0; k < 9; k++) {
        if (grid[r][k] != 0) rowCnt++;
        if (grid[k][c] != 0) colCnt++;
    }
    return rowCnt > MIN_CLUES_PER_LINE && colCnt > MIN_CLUES_PER_LINE;
}

void createSud(int resSud[9][9], int sud[9][9], int level) {
    int cellsToRemove = (level == 1) ? 32 : (level == 2) ? 44 : 54;
    int count = 0, attempts = 0;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            sud[i][j] = resSud[i][j];
            draftSud[i][j] = 0;
        }
    }
    while (count < cellsToRemove) {
        int r = rand() % 9, c = rand() % 9;
        if (sud[r][c] != 0 && canRemoveCell(sud, r, c)) {
            sud[r][c] = 0;
            count++;
        }
        if (++attempts > 10000) {
            for (int i = 0; i < 9; i++)
                for (int j = 0; j < 9; j++)
                    sud[i][j] = resSud[i][j];
            count = 0;
            attempts = 0;
        }
    }
}

void drawText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    for (int i = 0; text[i] != '\0'; i++)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
}

void drawRect(float x, float y, float w, float h, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x + w, y);
    glVertex2f(x + w, y + h);
    glVertex2f(x, y + h);
    glEnd();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (currentState == MENU) {
        glColor3f(0, 0, 0);
        glRasterPos2f(252, 650);
        char text[8] = "SUDOKU";
        for (int i = 0; text[i] != '\0'; i++) {
            glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[i]);
        }
        drawText(245, 600, "Choose Level:");
        drawRect(200, 370, 200, 50, 0.6, 0.85, 0.6);
        glColor3f(0, 0, 0);
        char buf[15];
        sprintf(buf, "Level easy");
        drawText(260, 388, buf);
        drawRect(200, 290, 200, 50, 0.6, 0.7, 0.9);
        glColor3f(0, 0, 0);
        sprintf(buf, "Level medium");
        drawText(248, 309, buf);
        drawRect(200, 210, 200, 50, 0.9, 0.6, 0.6);
        glColor3f(0, 0, 0);
        sprintf(buf, "Level hard");
        drawText(262, 229, buf);
    }
    else {
        glColor3f(0, 0, 0);
        for (int i = 0; i <= 9; i++) {
            glLineWidth((i % 3 == 0) ? 3.0 : 1.0);
            glBegin(GL_LINES);
            glVertex2f(75 + i * 50, 150);
            glVertex2f(75 + i * 50, 600);
            glVertex2f(75, 150 + i * 50);
            glVertex2f(525, 150 + i * 50);
            glEnd();
        }

        for (int r = 0; r < 9; r++) {
            for (int c = 0; c < 9; c++) {
                if (sud[r][c] != 0) {
                    glColor3f(0.1, 0.1, 0.5);
                    char buf[2];
                    sprintf(buf, "%d", sud[r][c]);
                    drawText(95 + c * 50, 565 - r * 50, buf);
                }
                else if (draftSud[r][c] != 0) {
                    int val = draftSud[r][c];
                    if (val < 0) {
                        glColor3f(0.8, 0.0, 0.0);
                        val = -val;
                    }
                    else glColor3f(0.5, 0.5, 0.5);

                    char buf[2];
                    sprintf(buf, "%d", val);
                    drawText(95 + c * 50, 565 - r * 50, buf);
                }
            }
        }

        for (int i = 1; i <= 10; i++) {
            if (selectedDigit == i) drawRect(45 + (i - 1) * 55, 60, 45, 45, 0.7, 0.9, 1.0);
            else drawRect(45 + (i - 1) * 55, 60, 45, 45, 0.9, 0.9, 0.9);
            glColor3f(0, 0, 0);
            if (i <= 9) {
                char buf[2];
                sprintf(buf, "%d", i);
                drawText(62 + (i - 1) * 55, 78, buf);
            }
            else {
                drawText(52 + (i - 1) * 55, 78, "Del");
            }
        }

        drawRect(230, 640, 80, 40, 0.8, 0.8, 0.8);
        glColor3f(0, 0, 0);
        drawText(243, 652, "Check");

        if (draftMode == 1) drawRect(330, 640, 90, 40, 0.5, 0.8, 1.0);
        else drawRect(330, 640, 90, 40, 0.8, 0.8, 0.8);
        glColor3f(0, 0, 0);
        drawText(350, 652, "Draft");if (hitMode == 1) drawRect(440, 640, 50, 40, 0.5, 1, 0.5);
        else drawRect(440, 640, 50, 40, 1, 0.8, 0);
        glColor3f(1, 1, 1);
        drawText(460, 652, "?");

        drawRect(500, 640, 60, 40, 0.8, 0.6, 1.0);
        glColor3f(0, 0, 0);
        drawText(512, 652, "Auto");

        glColor3f(0.7, 0, 0);
        char errBuf[20];
        sprintf(errBuf, "Errors: %d/3", error);
        drawText(50, 700, errBuf);
        glColor3f(0, 0.5, 0);
        char hntBuf[20];
        sprintf(hntBuf, "Hints: %d", hints);
        drawText(450, 700, hntBuf);

        drawRect(50, 640, 120, 40, 0.8, 0.8, 0.8);
        glColor3f(0, 0, 0);
        drawText(52, 652, "Back to Menu");

        if (currentState == GAMEOVER) {
            drawRect(50, 300, 500, 200, 1, 1, 1);
            glColor3f(1, 0, 0);
            drawText(240, 400, "GAME OVER");
        }
        if (currentState == WIN && botUsed == 0) {
            drawRect(50, 300, 500, 200, 1, 1, 1);
            glColor3f(0, 0.7, 0);
            drawText(250, 400, "YOU WIN!");
        }
    }
    glutSwapBuffers();
}

int solveSudokuBot(int grid[9][9]) {
    nodes++;
    int bestRow = -1, bestCol = -1, bestCount = 10;
    unsigned int bestMask = 0;
    for (int row = 0; row < 9; row++) {
        int sr = row - row % 3;
        for (int col = 0; col < 9; col++) {
            if (grid[row][col] != 0) continue;
            int sc = col - col % 3;
            unsigned int used = 0;
            for (int x = 0; x < 9; x++)
                used |= (1u << grid[row][x]) | (1u << grid[x][col]);
            for (int i = 0; i < 3; i++)
                for (int j = 0; j < 3; j++)
                    used |= 1u << grid[sr + i][sc + j];
            unsigned cand = ~used & 0x3FEu;
            if (cand == 0) return 0;

            int count = 0;
            for (unsigned int m = cand; m; m &= m - 1) {
                count++;
            }
            if (count < bestCount) {
                bestCount = count;
                bestRow = row;
                bestCol = col;
                bestMask = cand;
            }
        }
    }
    if (bestRow == -1) return 1;
    for (int num = 1; num <= 9; num++) {
        if (!((bestMask >> num) & 1u)) continue;
        grid[bestRow][bestCol] = num;
        if (animate) {
            display();
            Sleep(5);
        }
        if (solveSudokuBot(grid)) return 1;
        grid[bestRow][bestCol] = 0;
        if (animate) {
            display();
            Sleep(5);
        }
    }
    return 0;
}


void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float my = 800 - y;

        if (currentState == MENU) {
            if (x >= 200 && x <= 400 && my >= 370 && my <= 420) {
                createResSud(resSud);
                createSud(resSud, sud, 1);
                error = 0;
                hints = 2;
                selectedDigit = 0;
                draftMode = 0;
                botUsed = 0;
                currentState = PLAYING;
            }
            else if (x >= 200 && x <= 400 && my >= 290 && my <= 340) {
                createResSud(resSud);
                createSud(resSud, sud, 2);
                error = 0;
                hints = 2;
                selectedDigit = 0;
                draftMode = 0;
                botUsed = 0;
                currentState = PLAYING;
            }
            else if (x >= 200 && x <= 400 && my >= 210 && my <= 260) {
                createResSud(resSud);
                createSud(resSud, sud, 3);
                error = 0;
                hints = 2;
                selectedDigit = 0;
                draftMode = 0;
                botUsed = 0;
                currentState = PLAYING;
            }
        }
        else if (currentState == PLAYING) {
            for (int i = 1; i <= 10; i++) {
                if (x >= 45 + (i - 1) * 55 && x <= 90 + (i - 1) * 55 && my >= 60 && my <= 105)
                    selectedDigit = i;
            }
            if (x >= 75 && x <= 525 && my >= 150 && my <= 600) {
                int c = (x - 75) / 50;
                int r = 8 - (int)((my - 150) / 50);
                if (sud[r][c] == 0) {
                    if (hitMode == 1) {
                        sud[r][c] = resSud[r][c];
                        draftSud[r][c] = 0;
                        hints--;
                        hitMode = 0;
                    }
                    else if (selectedDigit == 10) {
                        draftSud[r][c] = 0;
                    }
                    else if (selectedDigit >= 1 && selectedDigit <= 9) {
                        if (draftMode == 1) {
                            draftSud[r][c] = selectedDigit;
                        }
                        else {
                            if (selectedDigit == resSud[r][c]) {
                                sud[r][c] = selectedDigit;
                                draftSud[r][c] = 0;
                            }
                            else error++;
                        }
                    }
                }
            }
            if (x >= 50 && x <= 170 && my >= 640 && my <= 680) currentState = MENU;
            if (x >= 440 && x <= 490 && my >= 640 && my <= 680 && hints > 0) hitMode = !hitMode;
            if (x >= 330 && x <= 420 && my >= 640 && my <= 680) draftMode = !draftMode;
            if (x >= 230 && x <= 310 && my >= 640 && my <= 680) {
                int newMistakes = 0;for (int i = 0; i < 9; i++) {
                    for (int j = 0; j < 9; j++) {
                        if (sud[i][j] == 0 && draftSud[i][j] != 0) {
                            int val = draftSud[i][j];
                            if (val < 0) val = -val;
                            else if (val > 10) val = val - 10;

                            if (val == resSud[i][j]) {
                                sud[i][j] = val;
                                draftSud[i][j] = '0';
                            }
                            else {
                                if (draftSud[i][j] > 0 && draftSud[i][j] <= 9) {
                                    newMistakes++;
                                }
                                draftSud[i][j] = -val;
                            }
                        }
                    }
                }
                error += newMistakes;
            }

            if (x >= 500 && x <= 560 && my >= 640 && my <= 680) {
                for (int i = 0; i < 9; i++) {
                    for (int j = 0; j < 9; j++) {
                        if (sud[i][j] != 0 && sud[i][j] != resSud[i][j]) {
                            sud[i][j] = 0;
                        }
                        draftSud[i][j] = 0;
                    }
                }
                solveSudokuBot(sud);
                botUsed = 1;
            }

            if (error >= 3) currentState = GAMEOVER;
            int filled = 0;
            for (int i = 0; i < 9; i++) {
                for (int j = 0; j < 9; j++) {
                    if (sud[i][j] != 0) filled++;
                }
            }
            if (filled == 81) currentState = WIN;
        }
        else if (currentState == GAMEOVER || currentState == WIN) currentState = MENU;
    }
    glutPostRedisplay();
}

int main(int argc, char** argv) {
    srand((unsigned int)time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(600, 800);
    glutCreateWindow("Sudoku OpenGL");
    glClearColor(0.95, 0.95, 0.97, 1);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, 600, 0, 800);
    glutDisplayFunc(display);
    glutMouseFunc(mouse);
    glutMainLoop();
    return 0;
}
