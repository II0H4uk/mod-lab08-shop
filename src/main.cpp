#include "../include/task.h"

int main()
{
    setlocale(LC_ALL, "russian");
    Shop shop(2, 10, 1, 1, 100, 1000);
    shop.Start();
    return 0;
}
