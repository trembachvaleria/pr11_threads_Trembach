// pr11_Trembach.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <conio.h>
using namespace std;

//список состояний
enum State { work, pause, finish };

//робот
struct Robot {
    int id;
    int energy = 60;
    int score = 0;
    State state = work;
};

int* tests = nullptr;
int testsCount = 0;

//препятствие
void test1_Obstacle(Robot* robot) {
    if (robot->energy < 10)
    {
        cout << "Робот №" << robot->id << " пропустил препятствие (мало энергии)" << endl;
        return;
    }
    Sleep(3000);
    robot->energy -= 10;
    robot->score += 15;
    cout << "Робот №" << robot->id << " прошел препятствие. Энергия: " << robot->energy << ", Очки: " << robot->score << endl;
}

//спринт
void test2_Sprint(Robot* robot) {
    if (robot->energy < 20)
    {
        cout << "Робот №" << robot->id << " пропустил спринт (мало энергии)" << endl;
        return;
    }
    Sleep(4000);
    robot->energy -= 20;
    robot->score += 30;
    cout << "Робот №" << robot->id << " прошел спринт. Энергия: " << robot->energy << ", Очки: " << robot->score << endl;
}

//подзарядка
void test3_Recharge(Robot* robot) {
    srand(robot->id);
    int time = rand() % 5 + 1;
    Sleep(time * 1000);
    robot->energy += time * 10;
    if (robot->energy > 100) robot->energy = 100;
    cout << "Робот №" << robot->id << " подзарядился. (" << time << "c.) " << "Энергия: " << robot->energy << endl;
}

//марафон
void test4_Marathon(Robot* robot) {
    if (robot->energy < 60)
    {
        cout << "Робот №" << robot->id << " пропустил марафон (мало энергии)" << endl;
        return;
    }
    Sleep(10000);
    robot->energy -= 60;
    robot->score += 55;
    cout << "Робот №" << robot->id << " прошел марафон. Энергия: " << robot->energy << ", Очки: " << robot->score << endl;
}

//массив указателей для функ
void (*testFuncs[])(Robot*) = { test1_Obstacle, test2_Sprint, test3_Recharge, test4_Marathon };

//поток
DWORD WINAPI RobotThread(LPVOID param) {
    Robot* robot = (Robot*)param;
    cout << "Робот №" << robot->id << " запущен на арене. Энергия: " << robot->energy << ", Очки : " << robot->score << endl;

    for (int i = 0; i < testsCount; i++)
    {
        if (robot->state == State::pause)
        {
            Sleep(100);
        }

        int testCode = tests[i];

        if (testCode >= 1 && testCode <= 4)
        {
            testFuncs[testCode - 1](robot);
        }

        srand(robot->id);
        Sleep((rand() % 3 + 1) * 1000);
    }

    robot->state = finish;
    cout << "! Робот №" << robot->id << " финишировал. Итог: " << robot->score << " очков." << endl;
    return 0;
}

int main() {
    setlocale(NULL, "Russian");
    srand(time(NULL));

    ifstream file("arena.dat", ios::binary);
    if (!file.is_open())
    {
        cout << "Ошибка: файл arena.dat не найден!" << endl;
        return 1;
    }

    file.seekg(0, ios::end);
    int fileSize = file.tellg();
    testsCount = fileSize / sizeof(int);
    file.seekg(0, ios::beg);

    tests = new int[testsCount];
    file.read((char*)tests, fileSize);
    file.close();

    cout << "Загружено испытаний: " << testsCount << endl;

    int robotCount;
    string input;
    do {
        cout << "Введите количество роботов (3-10): ";
        cin >> input;
        try
        {
            robotCount = stoi(input);
        }
        catch (exception) {
            throw invalid_argument("Ошибка ввода.");
        }
    } while (robotCount < 3 || robotCount > 10);

    Robot* robots = new Robot[robotCount];
    HANDLE* threads = new HANDLE[robotCount]; //дескриптор (идентификатор потока)

    cout << "\nСТАРТ" << endl;
    cout << "\nДля приостановления робота нажмите 'p', а для возобновления нажмите 'r'\n" << endl;

    for (int i = 0; i < robotCount; i++)
    {
        robots[i].id = i + 1;
        threads[i] = CreateThread(NULL, 0, RobotThread, &robots[i], 0, NULL);//создаем
        Sleep(100);
    }

    bool allDone = false;
    while (!allDone)
    {
        allDone = true;
        for (int i = 0; i < robotCount; i++)
        {
            if (robots[i].state != finish)
            {
                allDone = false;
                break;
            }
        }
        if (allDone) break;

        if (_kbhit())
        {
            char key = _getch();
            int id;

            if (key == 'p' || key == 'P')
            {
                //замораживаем всех
                for (int i = 0; i < robotCount; i++) {
                    SuspendThread(threads[i]);
                }

                cout << "\nВведите ID робота для паузы: ";
                cin >> id;
                cin.ignore();

                if (id >= 1 && id <= robotCount)
                {
                    robots[id - 1].state = pause;

                    //размораживаем всех кроме выбр
                    for (int i = 0; i < robotCount; i++) {
                        if (i != id - 1)
                        {
                            ResumeThread(threads[i]);
                        }
                    }
                    cout << "Робот №" << id << " на паузе." << endl << endl;
                }
                else
                {
                    //если неверн то размораж всех
                    for (int i = 0; i < robotCount; i++) {
                        ResumeThread(threads[i]);
                    }
                }
            }
            else if (key == 'r' || key == 'R')
            {
                for (int i = 0; i < robotCount; i++) {
                    SuspendThread(threads[i]);
                }

                cout << "\nВведите ID робота для возобновления: ";
                cin >> id;
                cin.ignore();

                if (id >= 1 && id <= robotCount)
                {
                    if (robots[id - 1].state == pause)
                    {
                        robots[id - 1].state = work;
                        ResumeThread(threads[id - 1]);//размораж его
                        cout << "Робот №" << id << " продолжает." << endl << endl;
                    }
                    else
                    {
                        cout << "Этот робот сейчас не на паузе." << endl;
                    }
                }
                for (int i = 0; i < robotCount; i++) {
                    ResumeThread(threads[i]);
                }
            }
        }
        Sleep(100);
    }

    //пока все не закончат
    WaitForMultipleObjects(robotCount, threads, TRUE, INFINITE);
    for (int i = 0; i < robotCount; i++)
    {
        CloseHandle(threads[i]);
    }

    for (int i = 0; i < robotCount - 1; i++)
    {
        for (int j = 0; j < robotCount - i - 1; j++)
        {
            if (robots[j].score < robots[j + 1].score)
            {
                Robot temp = robots[j];
                robots[j] = robots[j + 1];
                robots[j + 1] = temp;
            }
        }
    }

    cout << "\nРЕЗУЛЬТАТЫ:" << endl;
    for (int i = 0; i < robotCount; i++) {
        cout << i + 1 << ". Робот №" << robots[i].id << " | Очки: " << robots[i].score << endl;
    }

    if (robots[0].score == robots[1].score) {
        cout << "\nИТОГ: ничья!" << endl;
    }
    else {
        cout << "\nИТОГ: победил робот №" << robots[0].id << endl;
    }

    delete[] robots;
    delete[] threads;
    delete[] tests;

    return 0;
}

// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
