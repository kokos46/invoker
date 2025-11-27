#include <iostream>
#include <string>
#include <cstring>   // Для strcmp, strncmp
#include <algorithm> // Для std::swap

using namespace std;

// =========================================================================
// A. Заглушки для компиляции и вывода
// =========================================================================

/**
 * Имитирует ввод из консоли, возвращает std::string.
 */
string prompt(const string& message) {
    cout << message << ": ";
    string input;
    // Используем getline для ввода, содержащего пробелы (FIO)
    getline(cin, input);
    return input;
}

// =========================================================================
// B. Структура данных и константы
// =========================================================================

const int N = 4000;

// 🟢 ИСПРАВЛЕННАЯ СТРУКТУРА: СОДЕРЖИТ ТОЛЬКО ФИО, ОТДЕЛ, ДОЛЖНОСТЬ, ДЕНЬ РОЖДЕНИЯ
struct Record {
    char fio[30];          // ФИО (ключ для DBD-дерева по первым 3 буквам)
    int department_num;    // Номер отдела (ключ для QuickSort)
    char job_title[22];    // Должность (вторичный ключ для QuickSort)
    char birth_day[10];    // День рождения
};

// 🟢 ИСПРАВЛЕННЫЙ ВЫВОД ЗАГОЛОВКА
void print_head() {
    cout << "\n-----------------------------------------------------------------------\n";
    cout << " | # | FIO                             | Dept | Job Title            | Date       |\n";
    cout << "-----------------------------------------------------------------------\n";
}

// 🟢 ИСПРАВЛЕННЫЙ ВЫВОД ЗАПИСИ
void print_record(const struct Record* rec, int index) {
    cout.width(3); cout << index << " | ";
    cout.width(30); cout.setf(ios::left); cout << rec->fio << " | ";
    cout.width(4); cout << rec->department_num << " | ";
    cout.width(20); cout.setf(ios::left); cout << rec->job_title << " | ";
    cout.width(10); cout.setf(ios::left); cout << rec->birth_day << " |\n";
    cout.setf(ios::right); // Возвращаем по умолчанию
}

// =========================================================================
// D. Функции для Сортировки (QuickSort)
// (Логика diff оставлена прежней, так как использует существующие поля)
// =========================================================================

/**
 * Функция для сравнения двух записей для QuickSort.
 * Сначала по полю department_num (по возрастанию),
 * затем по полю job_title (лексикографически).
 */
int diff(const Record &a, const Record &b) {
    // 1. Сортировка по department_num (по возрастанию)
    if (a.department_num != b.department_num) {
        return a.department_num - b.department_num;
    }
    // 2. Сортировка по job_title (лексикографически)
    return strcmp(a.job_title, b.job_title);
}

void qSort(Record *array[], int L, int R) {
    if (L >= R) return;

    Record *x = array[(L + R) / 2];
    int i = L, j = R;

    while (i <= j) {
        while (diff(*array[i], *x) < 0) {
            ++i;
        }
        while (diff(*array[j], *x) > 0) {
            --j;
        }

        if (i <= j) {
            swap(array[i], array[j]);
            ++i;
            --j;
        }
    }

    qSort(array, L, j);
    qSort(array, i, R);
}

void quickSort(Record *array[], const int N) {
    qSort(array, 0, N - 1);
}

// =========================================================================
// E. Функции для Дерева поиска (DBD-дерево)
// (Логика оставлена прежней, так как использует существующее поле FIO)
// =========================================================================

struct Vertex {
    Record *data;
    Vertex *left;
    Vertex *right;
    int balance;
};

int compare_fio_prefix(const char *fio1, const char *fio2) {
    return strncmp(fio1, fio2, 3);
}

void dbd_add(Record *data, Vertex *&p, int& vr, int& hr) {
    if (!p) {
        p = new Vertex{data, NULL, NULL, 0};
        vr = 1;
        hr = 1;
        return;
    }

    if (compare_fio_prefix(data->fio, p->data->fio) < 0) {
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
    } else {
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

void search_in_tree(Vertex *root, const char key[], int &i) {
    if (root) {
        int cmp_result = compare_fio_prefix(key, root->data->fio);

        if (cmp_result < 0) {
            search_in_tree(root->left, key, i);
        } else if (cmp_result > 0) {
            search_in_tree(root->right, key, i);
        } else {
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
    int vr, hr;

    for (int i = 0; i < n; ++i) {
        vr = 0;
        hr = 0;
        dbd_add(arr[i], root, vr, hr);
    }

    cout << "\n--- Вывод всех записей в порядке ключа (Первые 3 буквы FIO) ---\n";
    print_head();
    int i = 1;
    Print_tree(root, i);

    string key_str;
    do {
        key_str = prompt("Input search key (Первые 3 буквы FIO), 0 - exit");

        if (key_str == "0") break;
        if (key_str.empty()) continue;

        const char* search_key = key_str.c_str();

        cout << "\n--- Результаты поиска по ключу (Первые 3 буквы FIO) = " << key_str << " ---\n";
        print_head();
        i = 1;
        search_in_tree(root, search_key, i);

        if (i == 1) {
            cout << "Записи не найдены.\n";
        }

    } while (true);

    rmtree(root);
}


// -------------------------------------------------------------------------
// E. Пример использования (main)
// -------------------------------------------------------------------------
int main() {
    FILE* file = fopen("testBase2.dat", "rb");
    if (!file) {
        perror("fopen error");
        return 1;
    }

    Record* recs = new Record[4000];
    int i = 0;
    while (i < 4000 && fread(&recs[i], sizeof(Record), 1, file) == 1) {
        i++;
    }

    Record* rec_ptrs[5];
    for(int i = 0; i < 5; ++i) {
        rec_ptrs[i] = &recs[i];
    }

    // --- Тестирование сортировки ---
    cout << "--- Записи до сортировки ---\n";
    print_head();
    for(int i = 0; i < 5; ++i) print_record(rec_ptrs[i], i+1);

    quickSort(rec_ptrs, 5);

    cout << "\n--- Записи после сортировки (по Dept возр., затем по Job Title возр.) ---\n";
    print_head();
    for(int i = 0; i < 5; ++i) print_record(rec_ptrs[i], i+1);

    // --- Тестирование дерева ---
    tree(rec_ptrs, 5);

    return 0;
}