#include<vector>
#include<algorithm>
#include<numeric>
#include<thread>
#include<random>
#include<chrono>
#include"Barber.h"

using namespace std;

void processCustomer(int id, int timeToServe)
{
	Customer c = Customer(id, timeToServe);
	c.arrive();
}

int main()
{
	CustomerLine& line = CustomerLine::getCustomerLine();

	int nCustomers = 20;
	int minTimeToServe = 1000;
	int maxTimeToServe = 10000;
	int minArrivalDelay = 700;
	int maxArrivalDelay = 1000;

	mt19937 rnd(time(NULL));
	uniform_int_distribution<int> serveTime(minTimeToServe, maxTimeToServe);
	uniform_int_distribution<int> arrivalDelay(minArrivalDelay, maxArrivalDelay);

	vector<thread> customerThreads(nCustomers);

	int fullDelay = 0;

	for (int i = 0; i < nCustomers; i++)
	{
		int id = i + 1;
		int timeToServe = serveTime(rnd);
		int delay = arrivalDelay(rnd);
		fullDelay += delay;

		this_thread::sleep_for(chrono::milliseconds(delay));
		cout << "Current time: " << fullDelay << "ms, customer " << id << " with time to serve " << timeToServe << " arrives\n";
		customerThreads[i] = thread([id, timeToServe]() { processCustomer(id, timeToServe); });
	}

	for (int i = 0; i < nCustomers; i++)
	{
		customerThreads[i].join();
	}
}