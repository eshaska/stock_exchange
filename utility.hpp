#pragma once
#include <iostream>
#include <string>
using std::string;


struct Order {
 std::string username;
 std::string side;  // Can be "Buy" or "Sell"
 std::string asset;
 int amount;
 int price;
 friend std::ostream& operator<<(std::ostream &os, const Order &o);
 friend bool operator==(const Order &o1, const Order &o2);
};


struct Trade {
 std::string buyer_username;
 std::string seller_username;
 std::string asset;
 int amount;
 int price;
};
