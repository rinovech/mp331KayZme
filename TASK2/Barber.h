#pragma once

#include<iostream>
#include<vector>
#include<mutex>
#include<semaphore>
#include<thread>
#include<queue>
#include<cassert>
#include<chrono>

using std::mutex, std::unique_lock, std::cout, std::queue, std::condition_variable, std::endl, std::binary_semaphore;

const int WAITING_PLACES = 5;

class Customer
{
public:
	void getServed(); // ����� ��� ������������ �������
	void arrive(); // ����� ��� �������� �������
	int getId() const; // ������ id �������
	int getTimeToServe() const; // ������ ��� ��������� ������� ������������ �������
	int getPlace() const; // ������ ����� ������� � �������

	Customer() = delete; // ������ ������������� ������������ �� ���������
	Customer(int id, int timeToServe) : id(id), timeToServe(timeToServe), place(-1) {};
	Customer(int id, int timeToServe, int place) : id(id), timeToServe(timeToServe), place(place) {};
	Customer(const Customer& customer) : id(customer.getId()), timeToServe(customer.getTimeToServe()), place(customer.getPlace()) {}; // ����������� �����������

private:
	int id;
	int timeToServe;
	int place;
};

class CustomerLine
{
	// ��� ��� ������� ������ ����, ���������� ������� "��������"
	// �������� �������
	// ����� ����, ��� ��� ��������� ������� ����� ����� ����������� � ��������
	// ��� ������ ���� �����������������
public:
	static CustomerLine& getCustomerLine(); // ����� ������������ ������������ ��������� ������
	bool empty(); // �����, �����������, ����� �� �������
	bool tryPushCustomer(Customer customer); // �����, ���������� �������� ���������� � �������
	Customer popCustomer(); // �����, ����������� ������� ���������� �� �������

private:
	queue<Customer> customers; // ������� ����������� 
	queue<int> waitingPlaces;  // � ������� ��������� ���� � �������
	mutex lineMutex; // ������� ��� ������������� ������� � �������
	condition_variable cv; // �������� ����������, �������������� ��� �������� ���������� ����� � �������

	CustomerLine(); // ����������� ������ - �������� private, ����� ����� ���� ������� ������ ���� ��������� ������
	CustomerLine(const CustomerLine& customerLine) = delete;               // ����������� ����������� � �������� ������������ - ���������, 
	CustomerLine& operator=(const CustomerLine& customerLine) = delete;    // ����� ������������� ����������� �������� ������
};

class Barber
{
	// ��� ��� ���������� ������ ����, ��� ��� ���������� �������� ������� "��������"
	// ������������ �������� �������
public:
	static Barber& getBarber();
	void awaken(); // �����, ������������ �����������
	void goToSleep(); // �����, ������������ ����������� �����
	bool isSleeping(); // ������ ��� ��������� ��������� �����������
	void checkLine(); // �����, ������� ���������, ���� �� � ������� ������� ������� ���� ��������� 
	bool canServe(); // �����, ������������ ���� �� � ����������� ����������� ��������� �������� (���� �� ����)
	void serve(const Customer& customer); // ����� ��������� ������� � �������� ��������� � ����������� ��� 

private:
	bool sleeping;
	binary_semaphore canWork;

	Barber() : canWork(1), sleeping(true) {}; // ����������� ������ - ���������, ��� ��� ������ ��������� ���������� ������ ��������
	Barber(const Barber& barber) = delete;            // ��������� ����������� � ������������ ���� �������,
	Barber& operator=(const Barber& barber) = delete; // ����� ������������� �������� ����� �������
};

inline void Customer::getServed()
{
	return Barber::getBarber().serve(*this);
}

inline void Customer::arrive()
{
	cout << "The customer " << id << " arrives\n";
	if (Barber::getBarber().canServe())
		getServed();
	else if (!CustomerLine::getCustomerLine().tryPushCustomer(*this))
		cout << "The customer " << id << " leaves\n";
}

inline int Customer::getId() const
{
	return id;
}

inline int Customer::getTimeToServe() const
{
	return timeToServe;
}

inline int Customer::getPlace() const
{
	return place;
}

inline CustomerLine& CustomerLine::getCustomerLine()
{
	static CustomerLine instance;
	return instance;
}

inline bool CustomerLine::empty()
{
	return customers.empty();
}

inline bool CustomerLine::tryPushCustomer(Customer customer)
{
	// ��� ��� ����� ��������� ����������� ����� ����� � �������, ��� ������ ���� ����������������
	unique_lock<mutex> lock(lineMutex);
	if (!waitingPlaces.empty())
	{
		int place = waitingPlaces.front();
		waitingPlaces.pop();
		customers.push(Customer(customer.getId(), customer.getTimeToServe(), place));
		cout << "The customer " << customer.getId() << " takes the place " << place << "\n";
		lock.unlock();
		cv.notify_one();
		return true;
	}
	else
	{
		lock.unlock();
		cv.notify_one();
		return false;
	}
}

inline Customer CustomerLine::popCustomer()
{
	// ��� ��� ���������� ����� ���������� ����-�� ��������� � ������, ����� ���-�� ������� � �������
	// �� ���� ������� ����������������
	unique_lock<mutex> lock(lineMutex);
	Customer customer = customers.front();
	customers.pop();
	waitingPlaces.push(customer.getPlace());
	cout << "The place " << customer.getPlace() << " is now free\n";
	lock.unlock();
	cv.notify_one();
	return customer;
}

inline CustomerLine::CustomerLine()
{
	for (int i = 0; i < WAITING_PLACES; i++)
	{
		waitingPlaces.push(i + 1);
	}
}

inline Barber& Barber::getBarber()
{
	static Barber instance;
	return instance;
}

inline void Barber::awaken()
{
	cout << "The barber has awoken\n";
	cout.flush();
	sleeping = false;
}

inline void Barber::goToSleep()
{
	canWork.release();
	cout << "The barber has begun sleeping\n";
	sleeping = true;
}

// ������ ��� ��������� ��������� �����������

inline bool Barber::isSleeping()
{
	return sleeping;
}

inline void Barber::checkLine()
{
	cout << "The barber checks the line\n";
	if (CustomerLine::getCustomerLine().empty())
		goToSleep();
	else
	{
		// ��� ��� �� �����, ��� ������� �������, � ������ �� ����� �� ��� ���-�� �������
		// �� ����� ������� ������ ������ �������
		Customer customer = CustomerLine::getCustomerLine().popCustomer();
		serve(customer);
	}
}

inline bool Barber::canServe()
{
	return canWork.try_acquire();
}

inline void Barber::serve(const Customer& customer)
{
	if (isSleeping())
		awaken();
	cout << "The customer " << customer.getId() << " is being served\n";
	std::this_thread::sleep_for(std::chrono::milliseconds(customer.getTimeToServe()));
	cout << "The customer " << customer.getId() << " has been served\n";
	checkLine();
}