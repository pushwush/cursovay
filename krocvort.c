#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORDS 100
#define MAX_WORD_LEN 32
#define GRID_SIZE 20

typedef enum { HORIZONTAL, VERTICAL } Direction;

typedef struct {
    char word[MAX_WORD_LEN];
    int x, y;
    Direction dir;
} PlacedWord;

char grid[GRID_SIZE][GRID_SIZE];
char word_list[MAX_WORDS][MAX_WORD_LEN];
int occupied[20][20] = {0}; // Занятость клеток словами

int word_count = 0;
PlacedWord placed[MAX_WORDS];
int placed_count = 0;

void clear_grid() {
    for (int i = 0; i < GRID_SIZE; i++){
        for (int j = 0; j < GRID_SIZE; j++){
            grid[i][j] = ' ';
            occupied[i][j] = 0;
        }
    }
}
void delete_word_from_list(int index) {
    if (index < 0 || index >= word_count) {
        printf("Error index\n");
        return;
    }
    for (int i = index; i < word_count - 1; i++) {
        strcpy(word_list[i], word_list[i + 1]);
    }
    word_count--;
}


void print_grid() {
    printf("\nCurrent crossword (%dx%d):\n");
    printf("   ");
    for (int j = 0; j < GRID_SIZE; j++) {
        printf("%2d ", j + 1); // Выводим номера столбцов
    }
    printf("\n");
    
    for (int i = 0; i < GRID_SIZE; i++) {
        printf("%2d ", i + 1); // Выводим номера строк
        for (int j = 0; j < GRID_SIZE; j++) {
            printf(" %c ", grid[i][j]); // Выводим символы сетки
        }
        printf("\n");
    }
}

void add_word_to_list(const char *word) {
    if (word_count < MAX_WORDS) {
        strncpy(word_list[word_count++], word, MAX_WORD_LEN);
    }
}
// int has_adjacent_crossing(const char *word, int x, int y, Direction dir) {
//     int len = strlen(word);
//     for (int i = 0; i < len; i++) {
//         int nx = x + (dir == HORIZONTAL ? i : 0);
//         int ny = y + (dir == VERTICAL ? i : 0);

//         if (grid[ny][nx] == word[i]) {
//             // Если уже стоит буква — это пересечение, проверим наличие других букв по перпендикуляру
//             if (dir == VERTICAL) {
//                 // Проверка по горизонтали: есть ли буквы слева и справа
//                 int left_filled = (nx > 0 && grid[ny][nx - 1] != ' ');
//                 int right_filled = (nx + 1 < GRID_SIZE && grid[ny][nx + 1] != ' ');
//                 if (left_filled || right_filled)
//                     return 1; // уже есть горизонтальное соединение, новое вертикальное пересечение запрещено
//             } else {
//                 // Проверка по вертикали: есть ли буквы сверху и снизу
//                 int top_filled = (ny > 0 && grid[ny - 1][nx] != ' ');
//                 int bottom_filled = (ny + 1 < GRID_SIZE && grid[ny + 1][nx] != ' ');
//                 if (top_filled || bottom_filled)
//                     return 1;
//             }
//         }
//     }
//     return 0;
// }

int creates_filled_square(int x, int y, Direction dir, const char *word) {
    int len = strlen(word);
    for (int i = 0; i < len; i++) {
        int nx = x + (dir == HORIZONTAL ? i : 0);
        int ny = y + (dir == VERTICAL ? i : 0);

        // Проверка всех 4 возможных квадратов 2x2, куда входит эта клетка
        for (int dy = -1; dy <= 0; dy++) {
            for (int dx = -1; dx <= 0; dx++) {
                int filled = 0;
                for (int yy = 0; yy <= 1; yy++) {
                    for (int xx = 0; xx <= 1; xx++) {
                        int cx = nx + dx + xx;
                        int cy = ny + dy + yy;
                        if (cx < 0 || cx >= GRID_SIZE || cy < 0 || cy >= GRID_SIZE)
                            continue;
                        char c = grid[cy][cx];

                        // учтём текущую вставляемую букву
                        if (cx == nx && cy == ny)
                            filled++;
                        else if (c != ' ')
                            filled++;
                    }
                }
                if (filled == 4)
                    return 1;
            }
        }
    }
    return 0;
}

int can_place(const char *word, int x, int y, Direction dir) {
    int len = strlen(word);
    for (int i = 0; i < len; i++) {
        int nx = x + (dir == HORIZONTAL ? i : 0);
        int ny = y + (dir == VERTICAL ? i : 0);
        if (nx < 0 || ny < 0 || nx >= GRID_SIZE || ny >= GRID_SIZE)
            return 0;
        char cell = grid[ny][nx];
        if (cell != ' ' && cell != word[i])
            return 0;
    }

    // запрет на создание 2x2 квадрата
    if (creates_filled_square(x, y, dir, word)){
        return 0;
    }
        
    // if (has_adjacent_crossing(word, x, y, dir)){
    //     return 0;
    // }

    return 1;
}




void place_word(const char *word, int x, int y, Direction dir) {
    int len = strlen(word);
    for (int i = 0; i < len; i++) {
        int nx = x + (dir == HORIZONTAL ? i : 0);
        int ny = y + (dir == VERTICAL ? i : 0);
        grid[ny][nx] = word[i];
        occupied[ny][nx] = 1;
    }

    PlacedWord pw;
    strncpy(pw.word, word, MAX_WORD_LEN);
    pw.x = x;
    pw.y = y;
    pw.dir = dir;
    placed[placed_count++] = pw;
}


int try_place_word(const char *word) {
    int best_score = -1, best_x = 0, best_y = 0, best_dir = HORIZONTAL;
    int len = strlen(word);

    if (placed_count > 0) {
        for (int pi = 0; pi < placed_count; pi++) {
            const PlacedWord *pw = &placed[pi];
            int plen = strlen(pw->word);

            for (int i = 0; i < len; i++) {
                for (int j = 0; j < plen; j++) {
                    if (word[i] != pw->word[j]) continue;

                    int x = pw->x + (pw->dir == HORIZONTAL ? j : 0) - (pw->dir == VERTICAL ? i : 0);
                    int y = pw->y + (pw->dir == VERTICAL ? j : 0) - (pw->dir == HORIZONTAL ? i : 0);
                    Direction dir = (pw->dir == HORIZONTAL ? VERTICAL : HORIZONTAL);

                    if (!can_place(word, x, y, dir)) continue;

                    int score = 0;
                    for (int k = 0; k < len; k++) {
                        int nx = x + (dir == HORIZONTAL ? k : 0);
                        int ny = y + (dir == VERTICAL ? k : 0);
                        if (grid[ny][nx] == word[k]) score++;
                    }

                    if (score > best_score) {
                        best_score = score;
                        best_x = x;
                        best_y = y;
                        best_dir = dir;
                    }
                }
            }
        }
    }
    if (best_score >= 0) {
        place_word(word, best_x, best_y, best_dir);
        return 1;
    }

    for (int y = 0; y < GRID_SIZE; y++) {
        for (int x = 0; x < GRID_SIZE; x++) {
            for (int d = 0; d < 2; d++) {
                if (can_place(word, x, y, d)) {
                    place_word(word, x, y, d);
                    return 1;
                }
            }
        }
    }

    return 0;
}

void auto_place_words_greedy() {
    clear_grid();
    placed_count = 0;

    if (word_count == 0) return;

    int x = GRID_SIZE / 2 - strlen(word_list[0]) / 2;
    int y = GRID_SIZE / 2;
    place_word(word_list[0], x, y, HORIZONTAL);

    for (int i = 1; i < word_count; i++) {
        if (!try_place_word(word_list[i])) {
            printf("Couldn't place a word: %s\n", word_list[i]);
        }
    }
}

void save_to_file(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) { perror("fopen"); return; }
    fprintf(f, "%d\n", word_count);
    for (int i = 0; i < word_count; i++)
        fprintf(f, "%s\n", word_list[i]);
    fclose(f);
    printf("Saved in  %s\n", filename);
}
void menu() {
    int choice;
    char input[MAX_WORD_LEN];
    while (1) {
        printf("1. Add a word\n");
        printf("2. Generate a crossword puzzle\n");
        printf("3. Show the grid\n");
        printf("4. Save a list of words\n");
        printf("5. Delete a word from the list\n");
        printf("0. Exit\n> ");
        scanf("%d", &choice);
        getchar();
        switch (choice) {
            case 1:
                printf("Enter the word: ");
                fgets(input, MAX_WORD_LEN, stdin);
                input[strcspn(input, "\n")] = 0;
                for (int i = 0; input[i]; i++) input[i] = toupper(input[i]);
                add_word_to_list(input);
                break;
            case 2:
                auto_place_words_greedy();
                break;
            case 3:
                print_grid();
                break;
            case 4:
                save_to_file("words.txt");
                break;
            case 5:
                for (int i = 0; i < word_count; i++) printf("%d. %s\n", i, word_list[i]);
                printf("Enter the word number: ");
                int idx;
                scanf("%d", &idx);
                getchar();
                delete_word_from_list(idx);
                break;
            
            case 0:
                return;
        }
    }
}

int main() {
    menu();
    return 0;
}