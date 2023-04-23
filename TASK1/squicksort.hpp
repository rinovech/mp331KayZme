#pragma once

template <class RandomIt>
void squicksort(RandomIt first, RandomIt last, RandomIt(*partition)(RandomIt, RandomIt))
{
	size_t n = last - first;
	if (n <= 1) return;
	RandomIt it = partition(first, last);
	squicksort(first, it, partition);
	squicksort(it + 1, last, partition);
}

template <class RandomIt, class DataType>
void squicksort(RandomIt first, RandomIt last, bool(*comp)(DataType, DataType), RandomIt(*partition)(RandomIt, RandomIt, bool(*)(DataType, DataType)))
{
	size_t n = last - first;
	if (n <= 1) return;
	RandomIt it = partition(first, last, comp);
	squicksort(first, it, comp, partition);
	squicksort(it + 1, last, comp, partition);
}
