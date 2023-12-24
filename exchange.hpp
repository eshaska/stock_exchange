#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>
#include <string>
#include <sstream>
#include <set>


#include "useraccount.hpp"
#include "utility.hpp"
using std::string;
using std::map;


class Exchange {
 private:
 string username;
 string asset;
 int amount;
 public:
map<string, std::vector<std::pair<int, string> > > portfolio;
map<string, std::vector<Order>> open_orders; //makers, string is username
map<string, std::vector<Order>> filled_orders; //takers, string is username
std::vector<Trade> all_trades;
Exchange() {}
Exchange(const string &username, const string &asset, int amount);
 void MakeDeposit(const string &username, const string &asset,
                  int amount);
 void PrintUserPortfolios(std::ostream &os);
 bool MakeWithdrawal(const string &username, const string &asset,
                     int amount);
 bool sufficient_assets(const Order &taker_order);
 int find_my_open_order(const Order& order);
 Order GetRemainderTakerOrder(const Order &taker_order, const Trade &trade);
 bool AddOrder(Order taker_order);
 void AddToFilledOrders(const Order &this_order, int settled_shares, int settled_price);
 Trade MakeATrade(const Order &taker_order, const Order &maker_order);
 void TakerOrderBecomesMakerOrder(const Order &taker_order);
 void ProcessTrade(const Trade &trade, const Order &taker_order, const Order &maker_order);
 void HandleRemainingBitsOfOrder(const Order &orig_order, int settled_num_shares, int settled_price, bool taker_bool);
 bool PriceWorks(const Order &taker_order, const Order &maker_order);
 bool AmountWorks(const Order &taker_order, const Order &maker_order);
 std::vector<Order> GetListOfPossibleMatchesForTakerOrder(const Order &taker_order);
 Order FindBestMatch(const Order &taker_order, std::vector<Order> possible_orders);
 void PrintUsersOrders(std::ostream &os);
 void PrintTradeHistory(std::ostream &os) const;
 int FindHighestOpenBuy(const string &asset) const;
 int FindLowestOpenSell(const string &asset) const;
 void PrintBidAskSpread(std::ostream &os) const;
};
