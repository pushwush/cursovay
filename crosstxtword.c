#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_WORDS 10000
#define MAX_DICT_WORDS 10000
#define MAX_WORD_LEN 100

char *dictionary[MAX_DICT_WORDS];
int dict_size = 0;

void load_dictionary(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Не удалось открыть словарь");
        exit(1);
    }

    char buffer[MAX_WORD_LEN];
    while (fgets(buffer, sizeof(buffer), file) && dict_size < MAX_DICT_WORDS) {
        buffer[strcspn(buffer, "\n")] = '\0'; // убрать \n
        dictionary[dict_size] = strdup(buffer); // динамически копируем строку
        dict_size++;
    }

    fclose(file);
}
typedef enum { HORIZONTAL, VERTICAL } Direction;

typedef struct {
    char word[MAX_WORD_LEN];
    int x, y;
    Direction dir;
} PlacedWord;
int grid_size;
char **grid = NULL;
char word_list[MAX_WORDS][MAX_WORD_LEN];
int occupied[20][20] = {0}; // Занятость клеток словами

int word_count = 0;
PlacedWord placed[MAX_WORDS];
int placed_count = 0;
int grid_size = 20;
//Иициализация сетки



void init_grid(int size) {
    if (grid) {
        for (int i = 0; i < grid_size; i++)
            free(grid[i]);
        free(grid);
    }

    grid_size = size;
    grid = malloc(size * sizeof(char*));
    for (int i = 0; i < size; i++) {
        grid[i] = malloc(size * sizeof(char));
        for (int j = 0; j < size; j++)
            grid[i][j] = ' ';
    }
}


void clear_grid() {
    for (int i = 0; i < grid_size; i++)
        for (int j = 0; j < grid_size; j++)
            grid[i][j] = ' ';
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
    printf("   ");
    for (int j = 0; j < grid_size; j++) {
        printf("%2d ", j + 1); // Выводим номера столбцов
    }
    printf("\n");
    
    for (int i = 0; i < grid_size; i++) {
        printf("%2d ", i + 1); // Выводим номера строк
        for (int j = 0; j <grid_size; j++) {
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
void add_random_words_from_file(int count) {
    if (count > dict_size) count = dict_size;
    srand((unsigned int)time(NULL));

    int used[MAX_DICT_WORDS] = {0};
    for (int i = 0; i < count; ++i) {
        int index;
        do {
            index = rand() % dict_size;
        } while (used[index]);

        used[index] = 1;
        add_word_to_list(dictionary[index]);
    }
}
 
// int has_adjacent_word_conflict(const char *word, int x, int y, Direction dir) {
//     int len = strlen(word);

//     for (int i = 0; i < len; i++) {
//         int nx = x + (dir == HORIZONTAL ? i : 0);
//         int ny = y + (dir == VERTICAL ? i : 0);

//         // 1. Проверка на диагональные соседи (их вообще не должно быть)
//         for (int dx = -1; dx <= 1; dx++) {
//             for (int dy = -1; dy <= 1; dy++) {
//                 if (dx == 0 && dy == 0) continue; // Пропустить саму клетку
//                 int cx = nx + dx;
//                 int cy = ny + dy;
//                 if (cx >= 0 && cx < grid_size && cy >= 0 && cy < grid_size) {
//                     if (grid[cy][cx] != ' ') {
//                         // Разрешаем только пересечение по нужному направлению
//                         if ((dir == HORIZONTAL && dx == 0) || (dir == VERTICAL && dy == 0)) {
//                             if (grid[cy][cx] != word[i]) {
//                                 return 1;
//                             }
//                         } else {
//                             return 1;
//                         }
//                     }
//                 }
//             }
//         }

//         // 2. Проверка начала слова (буква перед словом)
//         if (i == 0) {
//             int px = nx - (dir == HORIZONTAL ? 1 : 0);
//             int py = ny - (dir == VERTICAL ? 1 : 0);
//             if (px >= 0 && py >= 0 && px < grid_size && py < grid_size) {
//                 if (grid[py][px] != ' ')
//                     return 1;
//             }
//         }

//         // 3. Проверка конца слова (буква после слова)
//         if (i == len - 1) {
//             int sx = nx + (dir == HORIZONTAL ? 1 : 0);
//             int sy = ny + (dir == VERTICAL ? 1 : 0);
//             if (sx >= 0 && sy >= 0 && sx < grid_size && sy < grid_size) {
//                 if (grid[sy][sx] != ' ')
//                     return 1;
//             }
//         }
//     }

//     return 0;
// }

int has_tail_conflict(const char *word, int x, int y, Direction dir) {
    int len = strlen(word);

    // Проверка буквы ПЕРЕД началом слова
    int px = x - (dir == HORIZONTAL ? 1 : 0);
    int py = y - (dir == VERTICAL ? 1 : 0);
    if (px >= 0 && py >= 0 && px < grid_size && py < grid_size) {
        if (grid[py][px] != ' ') return 1;
    }

    // Проверка буквы ПОСЛЕ окончания слова
    int ex = x + (dir == HORIZONTAL ? len : 0);
    int ey = y + (dir == VERTICAL ? len : 0);
    if (ex >= 0 && ey >= 0 && ex < grid_size && ey < grid_size) {
        if (grid[ey][ex] != ' ') return 1;
    }

    return 0;
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
                        if (cx < 0 || cx >= grid_size || cy < 0 || cy >= grid_size)
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
        if (nx < 0 || ny < 0 || nx >= grid_size || ny >= grid_size)
            return 0;
        char cell = grid[ny][nx];
        if (cell != ' ' && cell != word[i])
            return 0;
    }

    // запрет на создание 2x2 квадрата
    if (creates_filled_square(x, y, dir, word)){
        return 0;
    }
    if (has_tail_conflict(word, x, y, dir)) return 0;

 
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

                    if (score > best_score || best_score == -1) {
                        best_score = score;
                        best_x = x;
                        best_y = y;
                        best_dir = dir;
                    }
                }
            }
        }
    }


    for (int y = 0; y < grid_size; y++) {
        for (int x = 0; x < grid_size; x++) {
            for (int d = 0; d < 2; d++) {
                if (can_place(word, x, y, d)) {
                    int score = 0;
                    for (int k = 0; k < len; k++) {
                        int nx = x + (d == HORIZONTAL ? k : 0);
                        int ny = y + (d == VERTICAL ? k : 0);
                        if (grid[ny][nx] == word[k]) score++;
                    }
    
                    if (score > best_score || best_score == -1) {
                        best_score = score;
                        best_x = x;
                        best_y = y;
                        best_dir = d;
                    }
                }
            }
        }
    }

    if (best_score >= 0) {
    place_word(word, best_x, best_y, best_dir);
    return 1;
    }

    return 0;
}

void auto_place_words_greedy() {
    clear_grid();
    placed_count = 0;

    if (word_count == 0) return;

    int x = grid_size / 2 - strlen(word_list[0]) / 2;
    int y = grid_size / 2;
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

void load_words_from_file(const char *filename, int count) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen");
        exit(1);
    }

    word_count = 0;
    while (fgets(word_list[word_count], MAX_WORD_LEN, f) && word_count < count) {
        word_list[word_count][strcspn(word_list[word_count], "\n")] = 0;
        for (int i = 0; word_list[word_count][i]; i++)
            word_list[word_count][i] = toupper(word_list[word_count][i]);
        word_count++;
    }

    fclose(f);
}

int main() {
    int size, count;
    printf("Enter the grid size: ");
    scanf("%d", &size);
    getchar();
    if (size <= 0 || size > 501) {
        printf("Invalid size\n");
        return 1;
    }

    printf("How many words to use from dictionary.txt? ");
    scanf("%d", &count);
    getchar();

    init_grid(size);
    load_words_from_file("dictionary.txt", count);
    auto_place_words_greedy();
    print_grid();
    return 0;
}
