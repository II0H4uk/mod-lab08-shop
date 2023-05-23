// Copyright 2021 GHA Test Team
#include <gtest/gtest.h>
#include "task.h"


TEST(shop_tests, test1) {
    Shop shop(1, 500, 5, 1, 10, 20);
    shop.Start();
    EXPECT_EQ(shop.requestCount, 10);
}

TEST(shop_tests, test2) {
    Shop shop(1, 500, 5, 1, 10, 20);
    shop.Start();
    EXPECT_EQ(shop.rejectedCount, 7);
}

TEST(shop_tests, test3) {
    Shop shop(1, 500, 5, 5, 10, 20);
    shop.Start();
    EXPECT_EQ(shop.rejectedCount, 3);
}

TEST(shop_tests, test4) {
    Shop shop(10, 50, 5, 5, 10, 2);
    shop.Start();
    EXPECT_EQ(shop.rejectedCount, 0);
}

TEST(shop_tests, test5) {
    Shop shop(10, 50, 5, 5, 10, 2);
    shop.Start();
    EXPECT_EQ(shop.processedCount, 10);
}

//#include <gtest/gtest.h>
//#include "task1.h"
//#include "task2.h"
//#include "task3.h"
