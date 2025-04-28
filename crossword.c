#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_WORDS 100
#define MAX_LEN 20
#define MAX_SIZE 20

// Структура для хранения информации о слове
typedef struct {
    char word[MAX_LEN]; // Слово
    int x, y;          // Координаты размещения
    int direction;     // Направление (0 - горизонтально, 1 - вертикально)
    int placed;        // Флаг, указывающий, размещено ли слово
} Word;

Word words[MAX_WORDS]; // Массив слов
int word_count = 0;    // Количество добавленных слов

char grid[MAX_SIZE][MAX_SIZE]; // Сетка кроссворда
int width = 15;                // Ширина сетки
int height = 15;               // Высота сетки

// Функция для очистки сетки
void clean_grid() {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            grid[i][j] = '$'; // Заполняем сетку символом '$'
        }
    }
}

// Функция для отображения текущей сетки кроссворда
void print_grid() {
    printf("\nCurrent crossword (%dx%d):\n", width, height);
    printf("   ");
    for (int j = 0; j < width; j++) {
        printf("%2d ", j + 1); // Выводим номера столбцов
    }
    printf("\n");
    
    for (int i = 0; i < height; i++) {
        printf("%2d ", i + 1); // Выводим номера строк
        for (int j = 0; j < width; j++) {
            printf(" %c ", grid[i][j]); // Выводим символы сетки
        }
        printf("\n");
    }
}

// Функция для проверки возможности размещения слова
int can_place_word(Word w) {
    // Проверяем границы сетки
    if (w.direction == 0) { // Горизонтальное размещение
        if (w.x < 0 || w.x + strlen(w.word) > width || w.y < 0 || w.y >= height) {
            return 0; // Не помещается
        }
    } else { // Вертикальное размещение
        if (w.y < 0 || w.y + strlen(w.word) > height || w.x < 0 || w.x >= width) {
            return 0; // Не помещается
        }
    }

    // Проверяем, можно ли разместить слово в указанной позиции
    for (int i = 0; i < strlen(w.word); i++) {
        char grid_char;
        if (w.direction == 0) {
            grid_char = grid[w.y][w.x + i]; // Проверяем горизонтально
        } else {
            grid_char = grid[w.y + i][w.x]; // Проверяем вертикально
        }
        
        if (grid_char != '$' && grid_char != w.word[i]) {
            return 0; // Находится конфликт
        }
    }
    
    return 1; // Можно разместить
}

// Функция для размещения слова в сетке
void place_word(Word w) {
    if (w.direction == 0) { // Горизонтально
        for (int i = 0; i < strlen(w.word); i++) {
            grid[w.y][w.x + i] = w.word[i]; // Размещаем слово
        }
    } else { // Вертикально
        for (int i = 0; i < strlen(w.word); i++) {
            grid[w.y + i][w.x] = w.word[i]; // Размещаем слово
        }
    }
}

// Функция для добавления нового слова
void add_word() {
    if (word_count >= MAX_WORDS) {
        printf("Reached the maximum number of words!\n");
        return;
    }
    
    printf("Enter a word: ");
    scanf("%19s", words[word_count].word);
    
    // Преобразуем слово в верхний регистр
    for (int i = 0; i < strlen(words[word_count].word); i++) {
        words[word_count].word[i] = toupper(words[word_count].word[i]);
    }
    
    words[word_count].placed = 0; // Устанавливаем флаг размещения
    word_count++; // Увеличиваем счетчик слов
    printf("Word added. Total words: %d\n", word_count);
}

// Функция для удаления слова
void delete_word() {
    if (word_count == 0) {
        printf("No words to delete!\n");
        return;
    }
    
    printf("Enter the number of the word to delete (1-%d): ", word_count);
    int word_num;
    scanf("%d", &word_num);
    
    if (word_num < 1 || word_num > word_count) {
        printf("Invalid word number!\n");
        return;
    }
    
    // Сдвигаем слова в массиве
    for (int i = word_num - 1; i < word_count - 1; i++) {
        words[i] = words[i + 1];
    }
    
    word_count--; // Уменьшаем счетчик слов
    printf("Word deleted. Total words: %d\n", word_count);
}

// Функция для проверки возможности размещения слова
void test_place_word() {
    if (word_count == 0) {
        printf("First add some words!\n");
        return;
    }
    
    printf("Enter the number of the word (1-%d): ", word_count);
    int word_num;
    scanf("%d", &word_num);
    
    if (word_num < 1 || word_num > word_count) {
        printf("Invalid word number!\n");
        return;
    }
    
    Word test_word = words[word_num - 1]; // Получаем слово для тестирования
    printf("Enter X Y and direction (0-horizontal, 1-vertical): ");
    scanf("%d %d %d", &test_word.x, &test_word.y, &test_word.direction);
    
    test_word.x--; // Корректируем координаты
    test_word.y--;
    
    // Проверяем возможность размещения
    if (can_place_word(test_word)) {
        printf("The word \"%s\" can be placed at this position!\n", test_word.word);
    } else {
        printf("Cannot place the word here.\n");
    }
}

// Функция для подтверждения размещения слова
void confirm_place_word() {
    if (word_count == 0) {
        printf("First add some words!\n");
        return;
    }
    
    printf("Enter the number of the word (1-%d): ", word_count);
    int word_num;
    scanf("%d", &word_num);
    
    if (word_num < 1 || word_num > word_count) {
        printf("Invalid word number!\n");
        return;
    }
    
    Word *current_word = &words[word_num - 1]; // Получаем текущее слово
    printf("Enter X Y and direction (0-horizontal, 1-vertical): ");
    scanf("%d %d %d", &current_word->x, &current_word->y, &current_word->direction);
    
    current_word->x--; // Корректируем координаты
    current_word->y--;
    
    // Проверяем возможность размещения и размещаем слово
    if (can_place_word(*current_word)) {
        place_word(*current_word);
        current_word->placed = 1; // Устанавливаем флаг размещения
        printf("The word \"%s\" has been successfully placed!\n", current_word->word);
    } else {
        printf("Cannot place the word at this position!\n");
    }
}

// Функция для сохранения списка слов в файл
void save_word_list(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error saving the word list.\n");
        return;
    }
    fprintf(file, "%d\n", word_count); // Сохраняем количество слов
    for (int i = 0; i < word_count; i++) {
        fprintf(file, "%s\n", words[i].word); // Сохраняем каждое слово
    }
    fclose(file);
    printf("Word list saved.\n");
}

// Функция для загрузки списка слов из файла
void load_word_list(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error loading the word list.\n");
        return;
    }
    fscanf(file, "%d\n", &word_count); // Считываем количество слов
    for (int i = 0; i < word_count; i++) {
        fgets(words[i].word, MAX_LEN, file);
        words[i].word[strcspn(words[i].word, "\n")] = 0; // Убираем символ новой строки
        words[i].placed = 0; // Устанавливаем флаг размещения
    }
    fclose(file);
    printf("Word list loaded.\n");
}

// Функция для сохранения текущей сетки кроссворда в файл
void save_grid(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error saving the grid.\n");
        return;
    }
    fprintf(file, "%d %d\n", width, height); // Сохраняем размеры сетки
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fputc(grid[i][j], file); // Сохраняем символы сетки
        }
        fputc('\n', file); // Перевод строки для следующей строки сетки
    }
    fclose(file);
    printf("Grid saved.\n");
}

// Функция для загрузки сетки из файла
void load_grid(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error loading the grid.\n");
        return;
    }
    fscanf(file, "%d %d\n", &width, &height); // Считываем размеры сетки
    clean_grid(); // Очищаем сетку перед загрузкой
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            grid[i][j] = fgetc(file); // Считываем символы сетки
        }
        fgetc(file); // Пропускаем перевод строки
    }
    fclose(file);
    printf("Grid loaded.\n");
}

// Функция для генерации случайного кроссворда
void generate_crossword() {
    clean_grid(); // Очищаем сетку перед генерацией
    for (int i = 0; i < word_count; i++) {
        Word w = words[i];
        w.x = rand() % width; // Случайная X координата
        w.y = rand() % height; // Случайная Y координата
        w.direction = rand() % 2; // Случайное направление (0 или 1)
        if (can_place_word(w)) {
            place_word(w); // Размещаем слово, если это возможно
            w.placed = 1; // Устанавливаем флаг размещения
            printf("The word \"%s\" has been placed.\n", w.word);
        } else {
            printf("The word \"%s\" could not be placed.\n", w.word);
        }
    }
}

// Функция для поиска слова в сетке
void search_word() {
    if (word_count == 0) {
        printf("First add some words!\n");
        return;
    }

    printf("Enter a word to search: ");
    char search_word[MAX_LEN];
    scanf("%19s", search_word);
    
    // Ищем слово в списке
    for (int i = 0; i < word_count; i++) {
        if (strcasecmp(words[i].word, search_word) == 0) {
            printf("The word \"%s\" found at position (%d, %d) in %s.\n", 
                   words[i].word, words[i].x + 1, words[i].y + 1, 
                   (words[i].direction == 0) ? "horizontal" : "vertical");
            return;
        }
    }
    printf("The word \"%s\" not found.\n", search_word); // Если слово не найдено
}

// Функция для отображения меню
void print_menu() {
    printf("\nMenu:\n");
    printf("1. Clear grid\n");
    printf("2. Add word\n");
    printf("3. Delete word\n");
    printf("4. Test word placement\n");
    printf("5. Place word\n");
    printf("6. Show grid\n");
    printf("7. Save word list\n");
    printf("8. Load word list\n");
    printf("9. Save grid\n");
    printf("10. Load grid\n");
    printf("11. Generate crossword\n");
    printf("12. Search word\n");
    printf("13. Exit\n");
    printf("Choose an action: ");
}

// Функция для очистки буфера ввода
void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

// Главная функция
int main() {
    srand(time(NULL)); // Инициализация генератора случайных чисел
    clean_grid(); // Очищаем сетку перед началом
    
    int choice = 0; // Переменная для выбора действия
    do {
        print_menu(); // Отображаем меню
        scanf("%d", &choice); // Считываем выбор пользователя
        clear_input_buffer(); // Очищаем буфер ввода
        
        // Обработка выбора пользователя
        switch(choice) {
            case 1:
                clean_grid();
                printf("Grid cleared.\n");
                break;
            case 2:
                add_word();
                break;
            case 3:
                delete_word();
                break;
            case 4:
                test_place_word();
                break;
            case 5:
                confirm_place_word();
                break;
            case 6:
                print_grid();
                break;
            case 7:
                save_word_list("words.txt");
                break;
            case 8:
                load_word_list("words.txt");
                break;
            case 9:
                save_grid("grid.txt");
                break;
            case 10:
                load_grid("grid.txt");
                break;
            case 11:
                generate_crossword();
                break;
            case 12:
                search_word();
                break;
            case 13:
                printf("Exiting...\n");
                break;
            default:
                printf("Enter a valid number!\n");
        }
        
    } while(choice != 13); // Продолжаем до тех пор, пока не выберем выход
    
    return 0; // Завершаем программу
}
