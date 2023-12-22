//4. Поиск самой длинной последовательности элементов, сумма которых равна 0;

#include <iostream>
#include <omp.h>
#include <thread>
#include <vector>
#include <conio.h>
#include <mutex>
#include <atomic>
#include <Windows.h>

using namespace std;

const int MAX_SIZE = 100000;
const int MAX_M = 1000;
const int MAX_THREADS = 10;

int max_seq_mutex = 0;
atomic<int> max_seq_atomic = ATOMIC_VAR_INIT(0);
int max_seq_critical = 0;

CRITICAL_SECTION cs;
mutex mtx;

#pragma region Headers
void input(int*, int*, int*, int*);
int find(int*, const int, const int, const int);
void find_par(int*, const int, const int, const int, int*);
void find_par_mutex(int*, const int, const int, const int);
void find_par_atomic(int*, const int, const int, const int);
void find_par_critical(int*, const int, const int, const int);
int find_par_omp(int*, const int, const int);

bool find_test();
#pragma endregion

int main()
{
	int N = -1;
	int M = -1;
	int seed = 0;
	int threads_num = 0;

	InitializeCriticalSection(&cs);
	
	setlocale(LC_ALL, "rus");

	while (true)
	{
#pragma region Input
		cout << "Стартуем..." << endl;
		input(&N, &M, &seed, &threads_num);
		cout << "Генерация осуществляется в следующих границах: [" << -M << ", " << M << "]." << endl;
#pragma endregion

#pragma region Generation
		int base[MAX_SIZE];
		srand(seed);
		for (int i = 0; i < N; i++) base[i] = rand() % (2 * M + 1) - M;
#pragma endregion

#pragma region Test
		cout << "Запуск тестов... " << endl;
		if (!find_test())
		{
			cout << "Тесты провалены =(" << endl;
			return 0;
		}
		else cout << "Тесты пройдены успешно =)" << endl;
#pragma endregion


#pragma region Search
		double start = omp_get_wtime();
		int max_seq = find(base, N, 0, 1);
		double end = omp_get_wtime();

		cout << "Время последовательного выполнения алгоритма: " << end - start << endl;
#pragma endregion

#pragma region SearchParallel
		vector<thread> threads;
		int locale_results[MAX_THREADS]{};

		start = omp_get_wtime();
		for (int i = 0; i < threads_num; i++) threads.push_back(thread(find_par, base, N, i, threads_num, &locale_results[i]));

		for (int i = 0; i < threads_num; i++) threads[i].join();

		int par_max_seq = 0;
		for (int i = 0; i < threads_num; i++) if (locale_results[i] > par_max_seq) par_max_seq = locale_results[i];

		end = omp_get_wtime();
		cout << "Время параллельного выполнения алгоритма: " << end - start << endl;
#pragma endregion

#pragma region Mutex
		threads.clear();
			
		start = omp_get_wtime();
		for (int i = 0; i < threads_num; i++) threads.push_back(thread(find_par_mutex, base, N, i, threads_num));

		for (int i = 0; i < threads_num; i++) threads[i].join();
		end = omp_get_wtime();

		cout << "Время параллельного выполнения алгоритма (mutex): " << end - start << endl;
#pragma endregion

#pragma region Atomic
		threads.clear();

		start = omp_get_wtime();
		for (int i = 0; i < threads_num; i++) threads.push_back(thread(find_par_atomic, base, N, i, threads_num));

		for (int i = 0; i < threads_num; i++) threads[i].join();
		end = omp_get_wtime();

		cout << "Время параллельного выполнения алгоритма (atomic): " << end - start << endl;
#pragma endregion

#pragma region CriticalSection
		threads.clear();

		start = omp_get_wtime();
		for (int i = 0; i < threads_num; i++) threads.push_back(thread(find_par_critical, base, N, i, threads_num));

		for (int i = 0; i < threads_num; i++) threads[i].join();
		end = omp_get_wtime();

		cout << "Время параллельного выполнения алгоритма (critical section): " << end - start << endl;
#pragma endregion

#pragma region OMP
		threads.clear();
		start = omp_get_wtime();
		int max_seq_omp = find_par_omp(base, N, threads_num);
		end = omp_get_wtime();

		cout << "Время параллельного выполнения алгоритма (omp): " << end - start << endl;
#pragma endregion

#pragma region ResultsAssert
		if (par_max_seq == max_seq) cout << "Успех! И последовательная версия алгоритма и параллельная выдали результат: " << max_seq << endl;
		else cout << "Чет там не сошлось как-то... Последовательная версия выдает: " << max_seq << ", а параллельная: " << par_max_seq << endl;

		if (max_seq_mutex == max_seq) cout << "Успех! Последовательная версия алгоритма и параллельная (с mutex) выдали идентичный результат!" << endl;
		else cout << "Чет там не сошлось как-то... Последовательная версия выдает: " << max_seq << ", а параллельная (с mutex): " << max_seq_mutex << endl;

		if (max_seq_atomic.load() == max_seq) cout << "Успех! Последовательная версия алгоритма и параллельная (с atomic) выдали идентичный результат!" << endl;
		else cout << "Чет там не сошлось как-то... Последовательная версия выдает: " << max_seq << ", а параллельная (с atomic): " << max_seq_atomic.load() << endl;

		if (max_seq_critical == max_seq) cout << "Успех! Последовательная версия алгоритма и параллельная (с critical section) выдали идентичный результат!" << endl;
		else cout << "Чет там не сошлось как-то... Последовательная версия выдает: " << max_seq << ", а параллельная (с critical section): " << max_seq_critical << endl;

		if (max_seq_omp == max_seq) cout << "Успех! Последовательная версия алгоритма и параллельная (omp) выдали идентичный результат!" << endl;
		else cout << "Чет там не сошлось как-то... Последовательная версия выдает: " << max_seq << ", а параллельная (omp): " << max_seq_omp << endl;
#pragma endregion

		cout << "Завершение вычислений..." << endl << endl;
		cout << "Запустить еще один тест? (нажми Y для запуска нового теста или любую другую клавишу для завершения работы программы)." << endl;
		int cmd = _getch();

		if (cmd == 121) continue;
		else return 0;
	}
}

void input(int* N, int* M, int* seed, int* thread_num)
{
	do
	{
		cout << "Введите количество элементов массива N [1, " << MAX_SIZE << "]: "; cin >> *N;

		if (*N <= 0 || *N > MAX_SIZE) cout << "Неверный ввод." << endl;
	} while (*N <= 0 || *N > MAX_SIZE);

	do
	{
		cout << "Введите положительную границу генерации [" << 1 << ", " << MAX_M << "]: "; cin >> *M;

		if (*M < 1 || *M > MAX_M) cout << "Неверный ввод." << endl;
	} while (*M < 1 || *M > MAX_M);

	cout << "Введите сид: "; cin >> *seed;

	do
	{
		cout << "Введите количество потоков [" << 2 << ", " << MAX_THREADS << "]: "; cin >> *thread_num;

		if (*thread_num < 2 || *thread_num > MAX_THREADS) cout << "Неверный ввод." << endl;
	} while (*thread_num < 2 || *thread_num > MAX_THREADS);
}

int find(int* base, const int N, const int start_index, const int offset)
{
	int max_sequence = 0;
	int sum;
	int locale_seq;

	for (int i = start_index; i < N; i += offset)
	{
		sum = base[i];
		if (sum == 0) locale_seq = 1;
		else locale_seq = 0;

		for (int j = i + 1; j < N; j++)
		{
			sum += base[j];
			if (sum == 0) locale_seq = j - i + 1;
		}

		if (locale_seq > max_sequence) max_sequence = locale_seq;
	}

	return max_sequence;
}

void find_par(int* base, const int N, const int thread_index, const int threads_count, int* locale_result)
{
	*locale_result = find(base, N, thread_index, threads_count);
}

void find_par_mutex(int* base, const int N, const int start_index, const int offset)
{
	int sum;
	int locale_seq;

	for (int i = start_index; i < N; i += offset)
	{
		sum = base[i];
		if (sum == 0) locale_seq = 1;
		else locale_seq = 0;

		for (int j = i + 1; j < N; j++)
		{
			sum += base[j];
			if (sum == 0) locale_seq = j - i + 1;
		}

		if (locale_seq > max_seq_mutex)
		{
			mtx.lock();
			max_seq_mutex = locale_seq;
			mtx.unlock();
		}
	}
}

void find_par_atomic(int* base, const int N, const int start_index, const int offset)
{
	int sum;
	int locale_seq;

	for (int i = start_index; i < N; i += offset)
	{
		sum = base[i];
		if (sum == 0) locale_seq = 1;
		else locale_seq = 0;

		for (int j = i + 1; j < N; j++)
		{
			sum += base[j];
			if (sum == 0) locale_seq = j - i + 1;
		}

		if (locale_seq > max_seq_atomic.load()) max_seq_atomic.store(locale_seq);
	}
}

void find_par_critical(int* base, const int N, const int start_index, const int offset)
{
	int sum;
	int locale_seq;

	for (int i = start_index; i < N; i += offset)
	{
		sum = base[i];
		if (sum == 0) locale_seq = 1;
		else locale_seq = 0;

		for (int j = i + 1; j < N; j++)
		{
			sum += base[j];
			if (sum == 0) locale_seq = j - i + 1;
		}

		if (locale_seq > max_seq_critical)
		{
			EnterCriticalSection(&cs);
			max_seq_critical = locale_seq;
			LeaveCriticalSection(&cs);
		}
	}
}

int find_par_omp(int* base, const int N, const int threads_num)
{
	int max_seq = 0;	
	omp_set_num_threads(threads_num);
	int locale_seq = 0;
	int sum = 0;

	#pragma omp parallel for shared(max_seq) private(sum, locale_seq)
	for (int i = 0; i < N; i++)
	{
		sum = base[i];

		if (sum == 0) locale_seq = 1;
		else locale_seq = 0;

		for (int j = i + 1; j < N; j++)
		{
			sum += base[j];
			if (sum == 0) locale_seq = j - i + 1;
		}

		if (locale_seq > max_seq)
		{
			#pragma omp critical
			{
				max_seq = locale_seq;
			}
		}
	}

	return max_seq;
}

bool find_test()
{
	int N;
	int base[MAX_SIZE];
	int result;

	bool assert = true;

#pragma region AllNullTest
	N = 10;
	for (int i = 0; i < N; i++)
	{
		base[i] = 0;
	}
	result = find(base, N, 0, 1);

	assert = result == 10;
#pragma endregion
#pragma region OneNullTest
	N = 10;
	base[0];
	for (int i = 1; i < N; i++)
	{
		base[i] = 1;
	}
	result = find(base, N, 0, 1);

	assert = result == 1;
#pragma endregion
#pragma region TwoSeqFourResult
	N = 10;
	base[0] = 5;
	base[1] = 0; base[2] = 0; base[3] = 0;
	base[4] = 1;
	base[5] = -3; base[6] = 3; base[7] = -3; base[8] = 3;
	base[9] = 100;
	result = find(base, N, 0, 1);

	assert = result == 4;
#pragma endregion
#pragma region NIsOne
	N = 1;
	base[0] = 10;
	result = find(base, N, 0, 1);
	
	assert = result == 0;
#pragma endregion

	return assert;
}
