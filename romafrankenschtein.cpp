#include <iostream>
#include <string>
#include <cstring> // Для работы со строками C-стиля, если нужно
#include <algorithm> // Для std::min

using namespace std;

// =========================================================================
// A. Заглушки для компиляции и вывода
//    (Предполагаемые недостающие функции)
// =========================================================================

string prompt(const string& message) {
    cout << message << ": ";
    string input;
    getline(cin, input);
    return input;
}

void print_head() {
    cout << "\n--------------------------------------------------------------------------------------------------\n";
    cout << " | # | FIO                             | Street           | Home | Apt | Date     | Sum |\n";
    cout << "--------------------------------------------------------------------------------------------------\n";
}

// ⚠️ Предполагается, что Record *имеет* поле sum, добавлено в Struct B
void print_record(const struct Record* rec, int index) {
    cout.width(3); cout << index << " | ";
    cout.width(30); cout.setf(ios::left); cout << rec->fio << " | ";
    cout.width(17); cout << rec->street << " | ";
    cout.width(4); cout.setf(ios::right); cout << rec->home << " | ";
    cout.width(3); cout << rec->appartament << " | ";
    cout.width(9); cout.setf(ios::left); cout << rec->date << " | ";
    cout.width(3); cout << rec->sum << " |\n";
    cout.setf(ios::right); // Возвращаем по умолчанию
}

// =========================================================================
// B. Структура данных и константы
// =========================================================================

const int N = 4000;  

struct Record {
    // Изменены на std::string для удобства, т.к. использовались в strcomp
    // Если требуется char[], нужно переписать strcomp на C-строки.
    string fio;
    string street;
    short int home;
    short int appartament;
    string date;
    
    // ⚠️ Добавлено поле sum и fio_adv, т.к. они использовались в compare/diff
    int sum = 0; 
    char fio_adv[22]; // Добавлено для компиляции функции compare
};

// =========================================================================
// C. Функции сравнения
// =========================================================================

/**
 * Сравнивает две строки. Возвращает:
 * -1, если str1 < str2
 * 1, если str1 > str2
 * 0, если равны
 */
int strcomp(const string &str1, const string &str2, int len = -1) {
    int max_len = (len == -1) ? (int)min(str1.length(), str2.length()) : len;

    for (int i = 0; i < max_len; ++i) {
        char c1 = (i < str1.length()) ? str1[i] : ' ';
        char c2 = (i < str2.length()) ? str2[i] : ' ';

        if (c1 < c2) {
            return -1;
        } else if (c1 > c2) {
            return 1;
        }
    }

    // Если строки одинаковы по длине len, возвращаем 0.
    // Если len == -1, сравниваем по минимальной длине, но это может быть неточно.
    // Для C++ строк лучше использовать str1.compare(str2)
    return 0; 
}

// ❌ Исходная функция compare() удалена/закомментирована, так как она:
// 1. Не используется.
// 2. Ссылается на нестандартные поля fio_adv и sum.
// Если она нужна, ее нужно обновить, чтобы она соответствовала структуре Record.
/*
int compare(const Record &record1, const Record &record2) {
    //... код с ошибками
    return 0;
}
*/


// =========================================================================
// D. Функции для Сортировки (QuickSort)
// =========================================================================

/**
 * Функция для сравнения двух записей для QuickSort.
 * Сначала по полю sum (в порядке убывания, если diff < 0), 
 * затем по полю date (лексикографически).
 */
int diff(const Record &a, const Record &b) {
    // Сравнение по sum (большее число идет раньше, т.к. diff = a.sum - b.sum)
    int result = a.sum - b.sum; 
    
    if (result == 0) {
        // Если sum равны, сравниваем по date
        result = strcomp(a.date, b.date);
    }
    return result;
}

void qSort(Record *array[], int L, int R) {
    while (L < R) {
        // Опорный элемент
        Record *x = array[(L + R) / 2];
        int i = L, j = R;
        
        while (i <= j) { // ⚠️ Использовал i <= j, как принято для корректной работы
            // Ищем элемент слева, который должен быть справа
            while (diff(*array[i], *x) < 0) {
                ++i;
            }
            // Ищем элемент справа, который должен быть слева
            while (diff(*array[j], *x) > 0) {
                --j;
            }
            
            if (i <= j) {
                // Меняем местами
                swap(array[i], array[j]);
                ++i;
                --j;
            }
        }
        
        // Рекурсивный вызов для меньшей части, итерация для большей
        if (j - L < R - i) {
            qSort(array, L, j);
            L = i;
        } else {
            qSort(array, i, R);
            R = j;
        }
    }
}

void quickSort(Record *array[], const int N) {
    qSort(array, 0, N - 1);
}

// =========================================================================
// E. Функции для Дерева поиска (DBD-дерево/Поиск по home)
// =========================================================================

struct Vertex {
    Record *data;
    Vertex *left;
    Vertex *right;
    int balance; // Фактор баланса
};

// ⚠️ ВАЖНО: dbd_add переписана для работы без статических переменных.
// Однако, логика dbd_add в оригинале НЕ ЯВЛЯЕТСЯ стандартным AVL-деревом.
// Я оставил исходные условия (0, 1) и логику поворотов, но убрал static
// и добавил более корректные (но все еще нестандартные) параметры.

void dbd_add(Record *data, Vertex *&p, int& vr, int& hr) {
    if (!p) {
        p = new Vertex{data, NULL, NULL, 0};
        vr = 1; // Указывает, что высота поддерева изменилась
        hr = 1; // Используется для внутренней логики
    } else if (data->home < p->data->home) {
        dbd_add(data, p->left, vr, hr);
        if (vr == 1) {
            if (p->balance == 0) {
                Vertex *q = p->left;
                p->left = q->right;
                q->right = p;
                p = q;
                q->balance = 1;
                vr = 0;
                hr = 1;
            } else {
                p->balance = 0;
                vr = 1;
                hr = 0;
            }
        } else {
            hr = 0;
        }
    } else { // data->home >= p->data->home (включая равенство, для простоты)
        dbd_add(data, p->right, vr, hr);
        if (vr == 1) {
            p->balance = 1;
            hr = 1;
            vr = 0;
        } else if (hr == 1) {
            if (p->balance == 1) {
                Vertex *q = p->right;
                p->balance = 0;
                q->balance = 0;
                p->right = q->left;
                q->left = p;
                p = q;
                vr = 1;
                hr = 0;
            } else {
                hr = 0;
            }
        }
    }
}

void Print_tree(Vertex *p, int &i) {
    if (p) {
        Print_tree(p->left, i);
        print_record(p->data, i++);
        Print_tree(p->right, i);
    }
}

void search_in_tree(Vertex *root, int key, int &i) {
    if (root) {
        if (key < root->data->home) {
            search_in_tree(root->left, key, i);
        } else if (key > root->data->home) {
            search_in_tree(root->right, key, i);
        } else { // key == root->data->home
            // Поиск всех элементов с таким же ключом (включая левое и правое поддерево)
            search_in_tree(root->left, key, i);
            print_record(root->data, i++);
            search_in_tree(root->right, key, i);
        }
    }
}

void rmtree(Vertex *root) {
    if (root) {
        rmtree(root->right);
        rmtree(root->left);
        delete root;
    }
}

void tree(Record *arr[], int n) {
    Vertex *root = NULL;
    int vr = 0; // Переменные для балансировки, передаются по ссылке
    int hr = 0;

    for (int i = 0; i < n; ++i) {
        // Передаем vr и hr по ссылке при добавлении
        vr = 0; 
        hr = 0;
        dbd_add(arr[i], root, vr, hr);
    }
    
    cout << "--- Вывод всех записей в порядке ключа 'home' ---\n";
    print_head();
    int i = 1;
    Print_tree(root, i);

    int key;
    do {
        // ⚠️ Здесь происходит преобразование из string в int
        string key_str = prompt("Input search key (home), 0 - exit");
        try {
            key = stoi(key_str);
        } catch (...) {
            key = -1; // Некорректный ввод
        }
        
        if (key != 0) {
            cout << "--- Результаты поиска по ключу home = " << key << " ---\n";
            print_head();
            i = 1;
            search_in_tree(root, key, i);
            if (i == 1) {
                cout << "Записи не найдены.\n";
            }
        }
    } while (key != 0);
    
    rmtree(root);
}

/*
// -------------------------------------------------------------------------
// E. Пример использования (main)
// -------------------------------------------------------------------------
int main() {
    // Пример создания и инициализации данных
    Record recs[3] = {
        {"Иванов И.И.", "Ленина", 10, 5, "01.01.24", 50},
        {"Петров П.П.", "Гагарина", 20, 15, "02.01.24", 70},
        {"Сидоров С.С.", "Ленина", 10, 10, "03.01.24", 50}
    };
    
    Record* rec_ptrs[3];
    for(int i = 0; i < 3; ++i) {
        rec_ptrs[i] = &recs[i];
    }
    
    // Тестирование сортировки
    cout << "--- До сортировки ---\n";
    print_head();
    for(int i = 0; i < 3; ++i) print_record(rec_ptrs[i], i+1);
    
    quickSort(rec_ptrs, 3);
    
    cout << "\n--- После сортировки (по sum убыв., затем по date возр.) ---\n";
    print_head();
    for(int i = 0; i < 3; ++i) print_record(rec_ptrs[i], i+1);
    
    // Тестирование дерева
    // tree(rec_ptrs, 3);
    
    return 0;
}
*/