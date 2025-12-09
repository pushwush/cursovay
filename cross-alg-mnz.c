#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_WORDS 10000
#define MAX_DICT_WORDS 10000
#define MAX_WORD_LEN 100

#define ALPHABET_SIZE 26
#define MAX_LETTER_POS 20000
typedef enum { HORIZONTAL, VERTICAL } Direction;
typedef struct {
    int x, y;
    Direction dir;
} LetterPos;

LetterPos letter_positions[ALPHABET_SIZE][MAX_LETTER_POS];
int letter_pos_count[ALPHABET_SIZE];

void reset_letter_index(){
    for (int i = 0; i < ALPHABET_SIZE; i++){
        letter_pos_count[i]=0;
    }
}

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



typedef struct {
    char word[MAX_WORD_LEN];
    int x, y;
    Direction dir;
} PlacedWord;

int grid_size;
char **grid = NULL;
char word_list[MAX_WORDS][MAX_WORD_LEN];


int word_count = 0;
PlacedWord placed[MAX_WORDS];
int placed_count = 0;

void index_word_letters(const char *word, int x, int y, Direction dir) {
    int len = strlen(word);
    for (int i = 0; i < len; i++) {
        int nx = x + (dir == HORIZONTAL ? i : 0);
        int ny = y + (dir == VERTICAL   ? i : 0);

        char ch = word[i];
        if (ch < 'A' || ch > 'Z') continue; // на всякий случай

        int idx = ch - 'A';

        int pos_index = letter_pos_count[idx];
        if (pos_index >= MAX_LETTER_POS) {
            // переполнение индекса — для курсовой можно просто предупредить
            // или ничего не делать
            continue;
        }

        letter_positions[idx][pos_index].x   = nx;
        letter_positions[idx][pos_index].y   = ny;
        letter_positions[idx][pos_index].dir = dir;

        letter_pos_count[idx]++;
    }
}




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

void delete_word_from_list(const char *word) {
    for (int i = 0; i < word_count; i++) {
        if (strcmp(word_list[i], word) == 0) {
            for (int j = i; j < word_count - 1; j++) {
                strcpy(word_list[j], word_list[j + 1]);
            }
            word_count--;
            return;
        }
    }
    printf("Слово '%s' не найдено в списке.\n", word);
}

void print_grid() {
    printf("   ");
    for (int j = 0; j < grid_size; j++) {
        printf("%2d ", j + 1); // Выводим номера столбцов
    }
    printf("\n");
    
    for (int i = 0; i < grid_size; i++) {
        printf("%2d ", i + 1); // Выводим номера строк
        for (int j = 0; j < grid_size; j++) {
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

// Вспомогательная функция: принадлежит ли клетка (cx, cy) новому слову
int is_new_word_cell(int cx, int cy, int x, int y, Direction dir, int len) {
    if (dir == HORIZONTAL) {
        // горизонтальное слово идёт по строке y от x до x+len-1
        if (cy == y && cx >= x && cx < x + len) return 1;
    } else {
        // вертикальное слово идёт по столбцу x от y до y+len-1
        if (cx == x && cy >= y && cy < y + len) return 1;
    }
    return 0;
}

// Проверка: появится ли где-то полностью заполненный квадрат 2x2
// после размещения ВСЕГО слова word в позиции (x, y) с направлением dir
int creates_filled_square(int x, int y, Direction dir, const char *word) {
    int len = strlen(word);

    int dx = (dir == HORIZONTAL) ? 1 : 0;   
    int dy = (dir == VERTICAL)   ? 1 : 0;

    for (int i = 0; i < len; i++) {
        int nx = x + dx * i;          
        int ny = y + dy * i;

        // Для клетки (nx, ny) проверяем все квадраты 2x2, куда она может входить
        for (int ddy = -1; ddy <= 0; ddy++) {
            for (int ddx = -1; ddx <= 0; ddx++) {
                int sx = nx + ddx; // левый верхний угол квадрата 2x2
                int sy = ny + ddy;
                int filled = 0;

                for (int yy = 0; yy <= 1; yy++) {
                    for (int xx = 0; xx <= 1; xx++) {
                        int cx = sx + xx;
                        int cy = sy + yy;

                        if (cx < 0 || cx >= grid_size || cy < 0 || cy >= grid_size)
                            continue;

                        if (is_new_word_cell(cx, cy, x, y, dir, len)) {
                            filled++;
                        } else if (grid[cy][cx] != ' ') {
                            filled++;
                        }
                    }
                }

                if (filled == 4) {
                    return 1;
                }
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

    return 1;
}

void place_word(const char *word, int x, int y, Direction dir) {
    int len = strlen(word);
    for (int i = 0; i < len; i++) {
        int nx = x + (dir == HORIZONTAL ? i : 0);
        int ny = y + (dir == VERTICAL   ? i : 0);
        grid[ny][nx] = word[i];
    }

    PlacedWord pw;
    strncpy(pw.word, word, MAX_WORD_LEN);
    pw.x = x;
    pw.y = y;
    pw.dir = dir;
    placed[placed_count++] = pw;

    // Новое: добавляем все буквы этого слова в индекс
    index_word_letters(word, x, y, dir);
}


   

int try_place_word(const char *word) {
    int best_score = -1;
    int best_x = 0, best_y = 0;
    Direction best_dir = HORIZONTAL;
    int len = strlen(word);

    if (placed_count == 0) {
        return 0; // первое слово ставится отдельно в auto_place_words_greedy
    }

    // Перебираем буквы нового слова
    for (int i = 0; i < len; i++) {
        char ch = word[i];
        if (ch < 'A' || ch > 'Z') continue;
        int idx = ch - 'A';

        int count = letter_pos_count[idx];
        // Перебираем все клетки, где эта буква уже стоит в сетке
        for (int p = 0; p < count; p++) {
            LetterPos lp = letter_positions[idx][p];

            // Новое слово должно идти перпендикулярно существующему
            Direction new_dir = (lp.dir == HORIZONTAL ? VERTICAL : HORIZONTAL);

            int x, y;
            if (new_dir == HORIZONTAL) {
                // новое слово горизонтально, буква word[i] в точке (lp.x, lp.y)
                x = lp.x - i;
                y = lp.y;
            } else {
                // новое слово вертикально
                x = lp.x;
                y = lp.y - i;
            }

            if (!can_place(word, x, y, new_dir)) continue;

            // Оцениваем "качество" варианта: сколько букв пересекается
            int score = 0;
            for (int k = 0; k < len; k++) {
                int nx = x + (new_dir == HORIZONTAL ? k : 0);
                int ny = y + (new_dir == VERTICAL   ? k : 0);
                if (nx < 0 || ny < 0 || nx >= grid_size || ny >= grid_size) continue;
                if (grid[ny][nx] == word[k]) score++;
            }

            if (score > best_score) {
                best_score = score;
                best_x = x;
                best_y = y;
                best_dir = new_dir;
            }
        }
    }

    if (best_score > 0) {
        place_word(word, best_x, best_y, best_dir);
        return 1;
    }

    return 0;
}


void auto_place_words_greedy() {
    clear_grid();
    placed_count = 0;
    reset_letter_index();

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
        printf("6. Remove a word and regenerate the crossword\n");
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
                if (idx >= 0 && idx < word_count) {
                    delete_word_from_list(word_list[idx]);
                } else {
                    printf("Неверный индекс.\n");
                }
                break;
            case 6:
                printf("Enter the word to remove: ");
                fgets(input, MAX_WORD_LEN, stdin);
                input[strcspn(input, "\n")] = 0;
                delete_word_from_list(input);
                auto_place_words_greedy();
                print_grid();
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



    // Буфер для чтения слов
    char buffer[MAX_WORD_LEN];
    word_count = 0;

    // Сначала прочитать все слова в массив dictionary
    dict_size = 0;
    while (fgets(buffer, MAX_WORD_LEN, f) && dict_size < MAX_DICT_WORDS) {
        buffer[strcspn(buffer, "\n")] = '\0';
        dictionary[dict_size] = strdup(buffer);
        dict_size++;
    }
    fclose(f);

    // Проверить, достаточно ли слов в словаре
    if (count > dict_size) {
        count = dict_size;
        printf("В словаре только %d слов. Будет загружено %d слов.\n", dict_size, count);
    }

    // Выбрать случайные слова без повторений
    int used[MAX_DICT_WORDS] = {0};
    while (word_count < count) {
        int index = rand() % dict_size;
        if (!used[index]) {
            strncpy(word_list[word_count], dictionary[index], MAX_WORD_LEN);
            // Конвертировать в верхний регистр
            for (int i = 0; word_list[word_count][i]; i++) {
                word_list[word_count][i] = toupper(word_list[word_count][i]);
            }
            used[index] = 1;
            word_count++;
        }
    }

    // Очистить временный словарь
    for (int i = 0; i < dict_size; i++) {
        free(dictionary[i]);
    }
    dict_size = 0;
}

int main() {
    int size, count;

    srand((unsigned int)time(NULL));

    printf("Enter the grid size: ");
    scanf("%d", &size);
    getchar();

    printf("How many words to use from dictionary.txt? ");
    scanf("%d", &count);
    getchar();

    init_grid(size);
    load_words_from_file("dictionary.txt", count);

    // замеряем только генерацию кроссворда
    clock_t start = clock();
    auto_place_words_greedy();
    clock_t end = clock();

    double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Generation time: %.6f seconds\n", seconds);

    // print_grid();  // на время исследований можно отключить

    menu(); 
    return 0;
}


