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
	void getServed(); // метод для обслуживания клиента
	void arrive(); // метод для прибытия клиента
	int getId() const; // геттер id клиента
	int getTimeToServe() const; // геттер для получения времени обслуживания клиента
	int getPlace() const; // геттер места клиента в очереди

	Customer() = delete; // запрет использования контсруктора по умолчанию
	Customer(int id, int timeToServe) : id(id), timeToServe(timeToServe), place(-1) {};
	Customer(int id, int timeToServe, int place) : id(id), timeToServe(timeToServe), place(place) {};
	Customer(const Customer& customer) : id(customer.getId()), timeToServe(customer.getTimeToServe()), place(customer.getPlace()) {}; // конструктор копирования

private:
	int id;
	int timeToServe;
	int place;
};

class CustomerLine
{
	// так как очередь только одна, используем паттерн "одиночка"
	// синглтон Майерса
	// кроме того, так как несколько потоков могут разом оперировать с очередью
	// она должна быть потоконезависимой
public:
	static CustomerLine& getCustomerLine(); // метод возвращающий единственный экземпляр класса
	bool empty(); // метод, проверяющий, пуста ли очередь
	bool tryPushCustomer(Customer customer); // метод, пытающийся добавить покупателя в очередь
	Customer popCustomer(); // метод, извлекающий первого покупателя из очереди

private:
	queue<Customer> customers; // очередь покупателей 
	queue<int> waitingPlaces;  // и очередь свободных мест в очереди
	mutex lineMutex; // мьютекс для синхронизации доступа к очереди
	condition_variable cv; // условная переменная, использующаяся для ожидания свободного места в очереди

	CustomerLine(); // конструктор класса - делается private, чтобы можно было создать только один экземпляр класса
	CustomerLine(const CustomerLine& customerLine) = delete;               // конструктор копирования и оператор присваивания - удаляются, 
	CustomerLine& operator=(const CustomerLine& customerLine) = delete;    // чтобы предотвратить копирование объектов класса
};

class Barber
{
	// так как парикмахер только один, для его реализации подойдет паттерн "одиночка"
	// используется синглтон Майерса
public:
	static Barber& getBarber();
	void awaken(); // метод, пробуждающий парикмахера
	void goToSleep(); // метод, отправляющий парикмахера спать
	bool isSleeping(); // геттер для получения состояния парикмахера
	void checkLine(); // метод, который проверяет, есть ли в очереди клиенты которых надо обслужить 
	bool canServe(); // метод, показывающий есть ли у парикмахера возможность обслужить клиентов (если он спит)
	void serve(const Customer& customer); // метод принимает клиента в качестве параметра и обслуживает его 

private:
	bool sleeping;
	binary_semaphore canWork;

	Barber() : canWork(1), sleeping(true) {}; // Конструктор класса - приватный, так что нельзя создавать экземпляры класса напрямую
	Barber(const Barber& barber) = delete;            // Операторы копирования и присваивания тоже удалены,
	Barber& operator=(const Barber& barber) = delete; // чтобы предотвратить создание копий объекта
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
	// так как сразу несколько посетителей могут войти в очередь, она должна быть потокобезопасной
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
	// так как парикмахер может попытаться кого-то обслужить в момент, когда кто-то заходит в очередь
	// ее надо сделать потокобезопасной
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

// геттер для получения состояния парикмахера

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
		// так как мы знаем, что очередь непуста, и только мы можем из нее что-то извлечь
		// мы можем извлечь оттуда первый элемент
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