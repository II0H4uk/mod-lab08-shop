#include "task.h"

int sum_client_time;
int sum_checkout_wait_time;
int sum_checkout_work_time;

Shop::Shop(int checkouts_num, int avg_processing_time, int avgProdCount, int queue_length, const int customer_count, int lambda) :
    checkouts_num(checkouts_num),
    avg_processing_time(avg_processing_time),
    avgProdCount(avgProdCount),
    queue_length(queue_length),
    customer_count(customer_count),
    requestCount(0),
    processedCount(0),
    rejectedCount(0),
    lambda(lambda) {}

void Shop::Start() {
    int mu = 1000 / avg_processing_time * avgProdCount;
    int queue_length_count = 0;

    vector<Checkout> checkouts(checkouts_num);
    queue<Customer> queue;

    random_device rd;
    mt19937 generator(rd());
    uniform_int_distribution<> distrib(1, 2 * avgProdCount);

    chrono::time_point<chrono::high_resolution_clock> total_time_start = chrono::high_resolution_clock::now();
    chrono::time_point<chrono::high_resolution_clock>* queue_time_start = new chrono::high_resolution_clock::time_point[customer_count];
    chrono::time_point<chrono::high_resolution_clock>* queue_time_end = new chrono::high_resolution_clock::time_point[customer_count];
    chrono::milliseconds* queue_time_dif = new chrono::milliseconds[customer_count];

    for (int i = 0; i < customer_count; ++i) {
        requestCount++;
        if (queue.size() < queue_length) {
            queue_time_start[i] = chrono::high_resolution_clock::now();
            queue.push(Customer(i, distrib(generator), avg_processing_time));
            processedCount++;
        }
        else {
            rejectedCount++;
        }

        for (Checkout& checkout : checkouts) {
            unique_lock<mutex> lock(checkout.mtx);

            if (!checkout.currentCustomer.created && !queue.empty()) {
                checkout.currentCustomer = queue.front();
                queue_time_end[queue.front().id] = chrono::high_resolution_clock::now();
                queue.pop();
                checkout.cond.notify_all();
                break;
            }
        }

        queue_length_count += queue.size();

        this_thread::sleep_for(chrono::milliseconds(1000 / lambda));
    }

    while (!queue.empty()) {
        for (auto& checkout : checkouts) {
            std::unique_lock<std::mutex> lock(checkout.mtx);

            if (!checkout.currentCustomer.created) {
                checkout.currentCustomer = queue.front();
                queue_time_end[queue.front().id] = chrono::high_resolution_clock::now();
                queue.pop();
                checkout.cond.notify_all();
            }
        }
    }

    for (auto& checkout : checkouts) {
        checkout.close();
    }

    for (int i = 0; i < customer_count; i++) {
        sum_client_time += chrono::duration_cast<chrono::milliseconds>(queue_time_end[i] - queue_time_start[i]).count();
    }

    chrono::time_point<chrono::high_resolution_clock> total_time_end = chrono::high_resolution_clock::now();
    chrono::milliseconds total_duration = chrono::duration_cast<chrono::milliseconds>(total_time_end - total_time_start);
    std::cout << "Ïðîøëî " << total_duration.count() << " ìñ." << endl;

    std::cout << "requestCount: " << requestCount << endl;
    std::cout << "processedCount: " << processedCount << endl;
    std::cout << "rejectedCount: " << rejectedCount << endl;

    double ro = (double)lambda / mu;
    double average_queue_length = (double)queue_length_count / customer_count;
    double avg_customer_time = sum_client_time / customer_count;

    double P0;
    double P0_buf = 1;
    for (int i = 1; i <= checkouts_num; i++) {
        P0_buf += pow(ro, i) / fact(i);
    }
    double checkouts_fact = fact(checkouts_num);
    for (int i = checkouts_num + 1; i <= checkouts_num + customer_count; i++) {
        P0_buf += pow(ro, i) / (checkouts_fact * pow(checkouts_num, i - checkouts_num));
    }
    P0 = pow(P0_buf, -1);

    double Prej = pow(ro, checkouts_num + customer_count) / (pow(checkouts_num, customer_count) * checkouts_fact) * P0;
    cout << Prej << endl;
    double Q = 1 - Prej;
    double A = lambda * Q;
}

double fact(int n) {
    if (n == 0 || n == 1) {
        return 1;
    }
    else {
        return n * fact(n - 1);
    }
}

Checkout::Checkout() : open(true) {
    thread = std::thread([this] {
        while (true) {
            unique_lock<mutex> lock(mtx);

            chrono::time_point<chrono::high_resolution_clock> checkout_wait_start = chrono::high_resolution_clock::now();

            while (!currentCustomer.created && open) {
                cond.wait(lock);
            }

            chrono::time_point<chrono::high_resolution_clock> checkout_wait_end = chrono::high_resolution_clock::now();

            if (!currentCustomer.created) {
                return;
            }

            chrono::time_point<chrono::high_resolution_clock> checkout_work_start = chrono::high_resolution_clock::now();
            chrono::time_point<chrono::high_resolution_clock> client_time_start = chrono::high_resolution_clock::now();

            int products = currentCustomer.products;
            int avg_processing_time = currentCustomer.avg_processing_time;
            int id = currentCustomer.id;
            currentCustomer = Customer();

            lock.unlock();
            this_thread::sleep_for(chrono::milliseconds(products * avg_processing_time));
            cout << "Customer " << id << " is serviced." << endl;
            chrono::time_point<chrono::high_resolution_clock> checkout_work_end = chrono::high_resolution_clock::now();
            chrono::time_point<chrono::high_resolution_clock> client_time_end = chrono::high_resolution_clock::now();

            lock_guard<std::mutex> lock_time(time_mtx);
            sum_client_time += chrono::duration_cast<chrono::milliseconds>(client_time_end - client_time_start).count();
            sum_checkout_wait_time += chrono::duration_cast<chrono::milliseconds>(checkout_wait_end - checkout_wait_start).count();
            sum_checkout_work_time += chrono::duration_cast<chrono::milliseconds>(checkout_work_end - checkout_work_start).count();
        }
    });
}

Customer::Customer() {
    id = -1;
    products = -1;
    avg_processing_time = -1;
    created = false;
}

void Checkout::close() {
    open = false;
    cond.notify_all();
    if (thread.joinable()) {
        thread.join();
    }
}
