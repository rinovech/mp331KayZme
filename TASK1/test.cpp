#include<iostream>
#include<vector>
#include<algorithm>
#include<ctime>
#include<cassert>
#include<random>
#include"pquicksort.hpp"
#include"squicksort.hpp"

using namespace std;

mt19937 rnd(time(NULL));

vector<int>::iterator partition(vector<int>::iterator first, vector<int>::iterator last) {
    auto end = last - 1;
    auto x = *end;
    auto i = first;
    for (auto j = first; j < end; j++) {
        if (*j <= x) {
            swap(*i, *j);
            i++;
        }
    }
    swap(*i, *end);
    return i;
}

vector<int>::iterator partition_compare(vector<int>::iterator first, vector<int>::iterator last, bool(*comp)(int, int)) {
    auto end = last - 1;
    auto x = *end;
    auto i = first;
    for (auto j = first; j < end; j++) {
        if (comp(*j, x)) {
            swap(*i, *j);
            i++;
        }
    }
    swap(*i, *end);
    return i;
}

bool comp(int x, int y)
{
    return x > y;
}

bool comp2(int x, int y)
{
    return abs(x) % 2 < abs(y) % 2;
}

vector<int> generate_random_test(int n)
{
    vector<int> data(n);
    uniform_int_distribution<int> nxt(1, n);
    for (int i = 0; i < n; i++)
        data[i] = nxt(rnd);
    return data;
}

pair<double, double> measure_random_test(int n)
{
    vector<int> v1 = generate_random_test(n);
    vector<int> v2 = v1;
    double t1_start = clock();
    squicksort(v1.begin(), v1.end(), partition);
    double t1_end = clock();
    assert(is_sorted(v1.begin(), v1.end()));
    double t2_start = clock();
    pquicksort(v2.begin(), v2.end(), partition);
    double t2_end = clock();
    assert(is_sorted(v2.begin(), v2.end()));
    return { (t1_end - t1_start) / CLOCKS_PER_SEC, (t2_end - t2_start) / CLOCKS_PER_SEC };
}

vector<int> generate_worst_case(int n, bool need_shuffle = false)
{
    vector<int> data(n);
    for (int i = 0; i < n; i++)
        data[i] = n - i;
    if (need_shuffle)
        shuffle(data.begin(), data.end(), rnd);
    return data;
}

pair<double, double> measure_worst_case(int n, bool need_shuffle = false)
{
    vector<int> v1 = generate_worst_case(n, need_shuffle);
    vector<int> v2 = v1;
    double t1_start = clock();
    squicksort(v1.begin(), v1.end(), partition);
    double t1_end = clock();
    assert(is_sorted(v1.begin(), v1.end()));
    double t2_start = clock();
    pquicksort(v2.begin(), v2.end(), partition);
    double t2_end = clock();
    assert(is_sorted(v2.begin(), v2.end()));
    return { (t1_end - t1_start) / CLOCKS_PER_SEC, (t2_end - t2_start) / CLOCKS_PER_SEC };
}

int main()
{
    pair<double, double> test_res = measure_worst_case(3e4, true);
    cout << test_res.first << " " << test_res.second << endl;
    test_res = measure_worst_case(1e7, true);
    cout << test_res.first << " " << test_res.second << endl;
    test_res = measure_random_test(1e7);
    cout << test_res.first << " " << test_res.second << endl;

    vector<int> a = { 11, 15, 1, 6, 2, 4, 4, 8, 1, 7, 3, 5 };
    pquicksort(a.begin(), a.end(), comp, partition_compare);
    for (auto x : a) cout << x << " ";
    cout << endl;
}
