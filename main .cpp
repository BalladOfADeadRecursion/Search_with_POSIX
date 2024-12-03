#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>
#include <cstdlib>
#include <sstream>
#include <cstdio>
#include <pthread.h>

using namespace std;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Структура для передачи аргументов в поток
struct ThreadArgs {
    int threadNum; // Номер потока
    unordered_map<string, vector<string>>* fragmentMap;
    vector<string> filenames;
    const string* fragment;
};

// Функция для генерации хэша для строки
size_t generateHash(const string& str) {
    size_t hash = 0;
    for (char ch : str) {
        hash = hash * 31 + ch;
    }
    return hash;
}

// Функция для обработки одного файла в отдельном потоке
void* processFile(void* args) {
    ThreadArgs* tArgs = (ThreadArgs*)args;
    int threadNum = tArgs->threadNum;
    cout << "Поток " << threadNum << " начал работу" << endl;

    unordered_map<string, vector<string>> localFragmentMap;

    for (const string& filename : tArgs->filenames) {
        ifstream file(filename);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                size_t pos = line.find(*(tArgs->fragment));
                if (pos != string::npos) {
                    cout << "Фрагмент найден в " << filename << " - " << threadNum << " поток" << endl;
                    pthread_mutex_lock(&mutex);
                    localFragmentMap[line.substr(pos, (*(tArgs->fragment)).size())].push_back(filename);
                    pthread_mutex_unlock(&mutex);
                }
            }
            file.close();
        } else {
            cerr << "Не удалось открыть файл: " << filename << endl;
        }
    }

    pthread_mutex_lock(&mutex);
    for (auto& entry : localFragmentMap) {
        (*(tArgs->fragmentMap))[entry.first].insert(end((*(tArgs->fragmentMap))[entry.first]), begin(entry.second), end(entry.second));
    }
    pthread_mutex_unlock(&mutex);

    cout << "Поток " << threadNum << " завершил работу" << endl;
    pthread_exit(NULL);
}
// Функция для создания и заполнения хэш-таблицы с использованием многопоточности
void createAndPopulateHashTable(unordered_map<string, vector<string>>& fragmentMap, const vector<string>& filenames, const string& fragment, int numThreads) {
    pthread_t threads[numThreads];

    vector<ThreadArgs> tArgs(numThreads);
    for (int i = 0; i < numThreads; ++i) {
        tArgs[i].threadNum = i + 1;
        tArgs[i].fragmentMap = &fragmentMap;
        tArgs[i].filenames = vector<string>(filenames.begin() + i * (filenames.size() / numThreads), filenames.begin() + (i + 1) * (filenames.size() / numThreads));
        tArgs[i].fragment = &fragment;
        pthread_create(&threads[i], NULL, processFile, (void*)&tArgs[i]);
    }

    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], NULL);
    }
}

// Функция для поиска фрагмента в хэш-таблице
void searchFragmentInHashTable(const unordered_map<string, vector<string>>& fragmentMap, const string& fragment, const vector<string>& filenames) {
    auto start = chrono::high_resolution_clock::now();

    if (fragmentMap.find(fragment) != fragmentMap.end()) {
        cout << "Фрагмент найден в следующих файлах:" << endl;
        for (const string& filename : fragmentMap.at(fragment)) {
            cout << filename << endl;
        }
    }
    else {
        cout << "Фрагмент не найден в указанных файлах." << endl;
    }

    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    double seconds = duration.count() / 1000.0;
    cout << "Время затраченное на поиск: " << seconds << " секунд" << endl;
}

// Функция для получения информации о затратах памяти
string getMemoryUsage() {
    FILE* file = popen("ps -o rss= -p $$", "r");
    if (!file) return "Ошибка при выполнении команды ps.";

    char buffer[128];
    string result = "";
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        result += buffer;
    }
    pclose(file);

    stringstream ss(result);
    int memory;
    ss >> memory;
    if (memory == 0) return "Не удалось получить информацию о затратах памяти.";

    return to_string(memory / -1024) + " КБ";
}

int main() {
    setlocale(LC_ALL, "Russian");
    vector<string> filenames = { "source1.txt", "source2.txt", "source3.txt", "source4.txt", "source5.txt", "source6.txt", "source7.txt", "source8.txt", "source9.txt", "source10.txt" };
    string fragment;
    int numThreads;

    cout << "Введите фрагмент для поиска: ";
    getline(cin, fragment);

    cout << "Введите количество потоков (от 2 до 10): ";
    cin >> numThreads;
    if (numThreads < 2 || numThreads > 10) {
        cerr << "Некорректное количество потоков. Установлено значение по умолчанию: 2" << endl;
        numThreads = 2;
    }

    unordered_map<string, vector<string>> fragmentMap;
    auto start = chrono::steady_clock::now();

    createAndPopulateHashTable(fragmentMap, filenames, fragment, numThreads);

    // Поиск фрагмента в файлах с использованием хэш-таблицы
    searchFragmentInHashTable(fragmentMap, fragment, filenames);

    auto end = chrono::steady_clock::now();
    auto diff = end - start;
    cout << "Time: " << chrono::duration_cast<chrono::seconds>(diff).count() << " seconds" << endl;

    // Получение и вывод информации о затратах памяти
    cout << "Затраты памяти на создание хэш-таблицы: " << getMemoryUsage() << endl;

    return 0;
}