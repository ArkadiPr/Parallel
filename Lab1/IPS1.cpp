#include <chrono>
#include <vector>

#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>

using namespace std::chrono;

/// Функция ReducerMinTest() определяет максимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMinTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
	{
		minimum->calc_min(i, mass_pointer[i]);
	}
	printf("Minimal element = %d has index = %d\n\n",
		minimum->get_reference(), minimum->get_index_reference());
}

/// Функция ReducerMaxTest() определяет максимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMaxTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	printf("Maximal element = %d has index = %d\n\n",
		maximum->get_reference(), maximum->get_index_reference());
}

/// Функция ParallelSort() сортирует массив в порядке возрастания
/// begin - указатель на первый элемент исходного массива
/// end - указатель на последний элемент исходного массива
void ParallelSort(int *begin, int *end)
{
	if (begin != end)
	{
		--end;
		int *middle = std::partition(begin, end, std::bind2nd(std::less<int>(), *end));
		std::swap(*end, *middle);
		cilk_spawn ParallelSort(begin, middle);
		ParallelSort(++middle, ++end);
		cilk_sync;
	}
}

void CompareForAndCilk_For(size_t sz)
{
	std::vector<int> vec;
	auto t0 = high_resolution_clock::now();
	for (size_t i = 0; i < sz; ++i)
		vec.push_back(rand() % 20000 + 1);

	auto t1 = high_resolution_clock::now();
	const duration<double> duration_vec = t1 - t0;

	cilk::reducer<cilk::op_vector<int>> red_vec;
	t0 = high_resolution_clock::now();
	cilk_for(size_t i = 0; i < sz; ++i)
		red_vec->push_back(rand() % 20000 + 1);

	t1 = high_resolution_clock::now();
	const duration<double> duration_red_vec = t1 - t0;

	printf("Size of array: %d\n", sz);
	printf("std::vector time: %f seconds\n", duration_vec.count());
	printf("cilk::reducer time: %f seconds\n", duration_red_vec.count());
}

int main()
{
	srand((unsigned)time(0));

	// устанавливаем количество работающих потоков = 4
	__cilkrts_set_param("nworkers", "4");

	long i;
	const long mass_size = 50000;
	int *mass_begin, *mass_end;
	int *mass = new int[mass_size];

	for (i = 0; i < mass_size; ++i)
	{
		mass[i] = (rand() % 25000) + 1;
	}

	mass_begin = mass;
	mass_end = mass_begin + mass_size;
	
	printf("Unsorted:\n");
	ReducerMinTest(mass, mass_size);
	ReducerMaxTest(mass, mass_size);

	auto t0 = high_resolution_clock::now();
	ParallelSort(mass_begin, mass_end);
	auto t1 = high_resolution_clock::now();
	duration<double> duration = t1 - t0;
	printf("Size of array: %d\n", mass_size);
	printf("Duration is %f seconds\n", duration.count());

	printf("Sorted:\n");
	ReducerMinTest(mass, mass_size);
	ReducerMaxTest(mass, mass_size);

	delete[]mass;

	auto sizes = {1000000, 100000, 10000, 1000, 500, 100, 50, 10};
	for (auto sz : sizes)
	{
		CompareForAndCilk_For(sz);
		printf("\n");
	}

	return 0;
}
