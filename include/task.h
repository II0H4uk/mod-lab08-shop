#pragma once
#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include <chrono>

using namespace std;

extern int sum_client_time;
extern int sum_checkout_wait_time;
extern int sum_checkout_work_time;

double fact(int n);

class Shop {
 public:
    Shop(int checkouts_num, int avg_processing_time, int avgProdCount, int queue_length, int customer_count, int lambda);
    void Start();
    int requestCount;
    int processedCount;
    int rejectedCount;

 private:
    int checkouts_num;
    int avg_processing_time;
    int avgProdCount;
    int queue_length;
    int customer_count;
    int lambda;
};

class Customer {
public:
    Customer(int id, int products, int avg_processing_time) :
        id(id),
        products(products),
        avg_processing_time(avg_processing_time),
        created(true) {}
    Customer();

    int id;
    int products;
    int avg_processing_time;
    bool created = false;
};

class Checkout {
 public:
     Checkout();

     void close();

     bool open;

     Customer currentCustomer;
     mutex mtx;
     mutex time_mtx;
     condition_variable cond;
     std::thread thread;
};