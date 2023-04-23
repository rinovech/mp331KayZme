#pragma once

#include"squicksort.hpp"
#include<thread>

const int LIMIT = 150000;

template <class RandomIt>
void pquicksort(RandomIt first, RandomIt last, RandomIt(*partition)(RandomIt, RandomIt))
{
	size_t n = last - first;
	if (n <= 1) return;
	if (n <= LIMIT)
	{
		squicksort(first, last, partition);
		return;
	}
	RandomIt it = partition(first, last);
	auto t1 = std::thread([first, it, partition] { pquicksort(first, it, partition); });
	pquicksort(it + 1, last, partition);
	t1.join();
}

template <class RandomIt, class DataType>
void pquicksort(RandomIt first, RandomIt last, bool(*comp)(DataType, DataType), RandomIt(*partition)(RandomIt, RandomIt, bool(*)(DataType, DataType)))
{
	size_t n = last - first;
	if (n <= 1) return;
	if (n <= LIMIT)
	{
		squicksort(first, last, comp, partition);
		return;
	}
	RandomIt it = partition(first, last, comp);
	auto t1 = std::thread([first, it, comp, partition] { pquicksort(first, it, comp, partition); });
	pquicksort(it + 1, last, comp, partition);
}
