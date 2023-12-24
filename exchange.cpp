#include "exchange.hpp"
#include "utility.hpp"


 MakeWithdrawal(taker_order.username, taker_order.asset, taker_order.amount);


 std::vector < Order > GetListOfPossibleMatchesForTakerOrder(const Order

 int requested_shares = orig_order.amount; //the shares they originally wanted

 std::vector <Order>> open_orders; //^makers, string is username


Exchange::Exchange(const string &u, const string &a, int am) : username(u), asset(a), amount(am) {}


void Exchange::MakeDeposit(const string &username, const string &asset, int amount) {


 auto asset_exists = std::find_if(portfolio[username].begin(), portfolio[username].end(),[&](const std::pair<int, std::string>& pair) {return pair.second == asset;});
 //source for find_if: https://cplusplus.com/reference/algorithm/find_if/
 if (asset_exists == portfolio[username].end()) { //means that there's no asset
   portfolio[username].push_back(std::make_pair(amount, asset));
 }
 else { //the asset already exists
   for (int i=0; i<static_cast<int>(portfolio[username].size()); i++) {
     if (portfolio[username].at(i).second == asset) {
       portfolio[username].at(i).first += amount; //making deposit
       break;
     }
   }
 }
}


void Exchange::PrintUserPortfolios(std::ostream &os) {
 os << "User Portfolios (in alphabetical order):";
 for (const auto&pair1 : portfolio) {
   os << "\n" << pair1.first << "\'s Portfolio: "; //pair1.first is the user's name
   auto user_vec = pair1.second; //pair1.second is the user's vector of assets and amounts
   std::sort(user_vec.begin(), user_vec.end(),
             [](const std::pair<int, std::string>& a, const std::pair<int, std::string>& b) {
               return a.second < b.second;
             });
   bool bool_continue = true; //variable's purpose is to end loop
   for (const auto &pair2 : user_vec) {
     if (bool_continue == false) {
       os << ", ";
     }
     os << pair2.first << " " << pair2.second;
     bool_continue = false;
   }
   if (pair1.second.size()>0) {
     os << ", ";
   }
 }
 os << "\n";
}


bool Exchange::MakeWithdrawal(const std::string &username, const std::string &asset, int amount) {
     if (portfolio.count(username)>0) { //checks that user has something to withdraw
     for (auto iter = portfolio[username].begin(); iter != portfolio[username].end(); ++iter) {
       if (iter->second == asset) {
         if (amount <= iter->first) {
           iter->first -= amount;
           if (iter->first == 0) {
             portfolio[username].erase(iter);
           }
             return true;
         }
       }
     }
   }
   return false;
 }




std::ostream& operator<<(std::ostream &os, const Order &o) {
 os << o.side << " " << o.amount << " " << o.asset << " at " << o.price << " USD by " << o.username;
 return os;
}


bool operator==(const Order &o1, const Order &o2) {
 if (o1.amount == o2.amount && o1.side == o2.side && o1.asset == o2.asset) {
   return true;
 }
 return false;
}


int Exchange::find_my_open_order(const Order &order) {
 int pos_count = 0;
 for(auto &x : open_orders[order.username]) {
 //x.first is the buyer's name, x.second is their list of open orders
     pos_count++;
   if (x.username == order.username && x.side == order.side && x.asset == order.asset && x.price <= order.price) {
     return pos_count;
   }
 }
 return pos_count;
}


bool Exchange::sufficient_assets(const Order &taker_order) {
 if (taker_order.side == "Buy") {
   for (auto &x : portfolio[taker_order.username]) {
       if (x.second == "USD") { //checks that asset is USD
         if (x.first >= taker_order.price*taker_order.amount) {
         //checks if the user has enough money to buy what they want
           return true;
         }
       }
   }
 }
 else { //taker is a seller
   for (auto &x : portfolio[taker_order.username]) {
       if (x.second == taker_order.asset) {
         if (x.first >= taker_order.amount) {
         //checks that the user has enough of the asset to make the sale
           return true;
         }
       }
   }
 }
 return false;
}


Order Exchange::GetRemainderTakerOrder(const Order &taker_order, const Trade &trade) {
 int unfulfilled_amount = taker_order.amount-trade.amount;
 Order new_taker_order = Order{"empty", "empty", "empty", 0, 0};
 //used to know when there was no unfufilled amount^
 if (unfulfilled_amount>0) {
   new_taker_order = Order{taker_order.username, taker_order.side, taker_order.asset, unfulfilled_amount, taker_order.price};
 }
 return new_taker_order;
}


bool Exchange::AddOrder(Order taker_order) {
 Trade trade;
 if (sufficient_assets(taker_order)) {
   if (taker_order.side == "Buy") {
     MakeWithdrawal(taker_order.username, "USD", taker_order.amount*taker_order.price);
     //if they are a buyer who has enough money, the money is immediately withdrawn
   }
   else { //taker's selling
     MakeWithdrawal(taker_order.username, taker_order.asset, taker_order.amount);
   }
   std::vector<Order> possible_matches = GetListOfPossibleMatchesForTakerOrder(taker_order);
   while (taker_order.username != "empty") {
     if (possible_matches.empty()) {
       TakerOrderBecomesMakerOrder(taker_order);
       taker_order = Order{"empty", "empty", "empty", 0, 0};
     }
     else { //there are some possible matches
       //loop until they got their full request or no matches remain
       Order best_match_order = FindBestMatch(taker_order, possible_matches);
         trade = MakeATrade(taker_order, best_match_order);
         ProcessTrade(trade, taker_order, best_match_order);
         taker_order = GetRemainderTakerOrder(taker_order, trade);
         possible_matches = GetListOfPossibleMatchesForTakerOrder(taker_order);
       }
   }
 HandleRemainingBitsOfOrder(taker_order, trade.amount, trade.price, true);
 //handles any unfufilled parts of the order
 return true;
 }
 else {
   return false;
 }
}


void Exchange::PrintUsersOrders(std::ostream &os) {
 os << "Users Orders (in alphabetical order):";
   for (auto & f : portfolio) { //f.first is the username, f.second is the vector of amounts and assets
     os << "\n" << f.first << "\'s Open Orders (in chronological order):";
     for (auto& x : open_orders[f.first]) {
           os << "\n" << x.side << " " << x.amount << " " << x.asset << " at " << x.price << " USD by " << x.username;
       }
       os << "\n" << f.first << "\'s Filled Orders (in chronological order):";
       for (auto &x : filled_orders[f.first]) {
         os << "\n" << x.side << " " << x.amount << " " << x.asset << " at " << x.price << " USD by " << x.username;
       }
     }
   os << "\n";
}


void Exchange::AddToFilledOrders(const Order &this_order, int settled_shares, int settled_price) {
 Order new_order;
 new_order = this_order;
 new_order.amount = settled_shares;
 new_order.price = settled_price;
 filled_orders[this_order.username].push_back(new_order);
}


Trade Exchange::MakeATrade(const Order &taker_order, const Order &maker_order) {
 int settled_num_shares;
 int settled_price;
 string buyer_name;
 string seller_name;
 settled_num_shares = std::min(taker_order.amount, maker_order.amount);
 //^the minimum number of shares between the taker and maker is the one they use
 if (taker_order.side == "Buy") {
   buyer_name = taker_order.username;
   seller_name = maker_order.username;
   settled_price = std::max(taker_order.price, maker_order.price);
   //the maximum price is the price that will be used in the trade
   MakeDeposit(taker_order.username, taker_order.asset, settled_num_shares);
   MakeDeposit(maker_order.username, "USD", settled_price*settled_num_shares);
 }
 else { //taker is a seller
   buyer_name = maker_order.username;
   seller_name = taker_order.username;
   settled_price = std::min(taker_order.price, maker_order.price);
   MakeDeposit(taker_order.username, "USD", settled_price*settled_num_shares);
   MakeDeposit(maker_order.username, maker_order.asset, settled_num_shares);
 }
 Trade new_trade = Trade{buyer_name, seller_name, taker_order.asset, settled_num_shares, settled_price};
 all_trades.push_back(new_trade); //all_trades is used for PrintTradeHistory later on
 return new_trade;
}


void Exchange::TakerOrderBecomesMakerOrder(const Order &taker_order) {
 open_orders[taker_order.username].push_back(taker_order);
}


void Exchange::HandleRemainingBitsOfOrder(const Order &orig_order, int settled_num_shares, int settled_price, bool taker_bool) {
 int requested_price = orig_order.price; //the price they originally wanted
 int requested_shares = orig_order.amount; //the shares they originally wanted
 if (taker_bool) { //checks if user is a taker
   if (requested_price != settled_price || requested_shares != settled_num_shares) {
     if (requested_shares - settled_num_shares>0) {
       Order new_order = Order{orig_order.username, orig_order.side, orig_order.asset, requested_shares-settled_num_shares, orig_order.price};
       open_orders[orig_order.username].push_back(new_order);
     }
   }
 }
 else { //user is maker
   int position = find_my_open_order(orig_order)-1;
   if (requested_shares == settled_num_shares) {
     //the shares are now the same, so the open order can be removed
     open_orders[orig_order.username].erase(open_orders[orig_order.username].begin()+position);
   }
   else {
     //the open order is updated with the new amount
     open_orders[orig_order.username].at(position).amount = orig_order.amount - settled_num_shares;
   }
 }
}


void Exchange::ProcessTrade(const Trade &trade, const Order &taker_order, const Order &maker_order) {
 AddToFilledOrders(maker_order, trade.amount, trade.price);
 AddToFilledOrders(taker_order, trade.amount, trade.price);
 HandleRemainingBitsOfOrder(maker_order, trade.amount, trade.price, false);
}


bool Exchange::PriceWorks(const Order &taker_order, const Order &maker_order) {
 if (taker_order.side == "Buy") {
   if (maker_order.price <= taker_order.price) { //taker buyer wants lowest price
     return true;
   }
 }
 else { //taker is a seller
   if (taker_order.price <= maker_order.price) {
     return true;
   }
 }
 return false;
}


bool Exchange::AmountWorks(const Order &taker_order, const Order &maker_order) {
 //taker is concerned with price and will take as much as they can get
 if (taker_order.side == "Buy") {
   return true;
 }
 else if (taker_order.side == "Sell") {
   return true;
 }
 return false;
}


std::vector<Order> Exchange::GetListOfPossibleMatchesForTakerOrder(const Order &taker_order){
 std::vector<Order> possible_matches;
 for (auto &x : open_orders) {
   for (auto &poss_order : x.second) {
   if (poss_order.side != taker_order.side && poss_order.asset == taker_order.asset
   && PriceWorks(taker_order, poss_order) && AmountWorks(taker_order, poss_order)) {
     //a potential match has been found
     possible_matches.push_back(poss_order);
   }
   }
 }
 return possible_matches;
}


Order Exchange::FindBestMatch(const Order &taker_order, std::vector<Order> possible_orders) {
 Order best_order_so_far;
 if (taker_order.side == "Buy") { //buyer has already set the price
  //the buyer wants the lowest price they can get
  int lowest_price_so_far = 2000000; //arbitrary high number used to initialize
  for (auto &this_order : possible_orders) {
    if (this_order.price < lowest_price_so_far) {
      lowest_price_so_far = this_order.price;
      best_order_so_far = this_order;
    }
  }
}
 else {
   //the seller wants the highest price they can get
   int highest_price_so_far = 0;
   for (auto &this_order : possible_orders) {
     if (this_order.price > highest_price_so_far) {
       highest_price_so_far = this_order.price;
       best_order_so_far = this_order;
     }
   }
 }
 return best_order_so_far;
}


void Exchange::PrintTradeHistory(std::ostream &os) const {
 os << "Trade History (in chronological order):";
 for (auto &x : all_trades) { //x is a Trade
   os << "\n" << x.buyer_username << " Bought " << x.amount << " of " << x.asset << " From " << x.seller_username << " for " << x.price << " USD";
 }
 os << "\n";
}


int Exchange::FindHighestOpenBuy(const string &asset) const{
 //used to calculate bid ask spread
 int max_buy_price=0;
 for (auto &p : open_orders) {
   for (auto &this_order : p.second) {
     if (this_order.asset == asset) {
       if (this_order.side == "Buy") { //trying to find highest buy price
         if (this_order.price >= max_buy_price) {
           max_buy_price = this_order.price;
         }
       }
     }
   }
 }
 return max_buy_price;
}


int Exchange::FindLowestOpenSell(const string &asset) const{
 //used to calculate bid ask spread
 int min_sell_price = 200000000; //arbitrary high number
 for (auto &p : open_orders) {
   for (auto &this_order : p.second) {
     if (this_order.asset == asset) {
       if (this_order.side == "Sell") {
         if (this_order.price < min_sell_price) {
           min_sell_price = this_order.price;
         }
       }
     }
   }
 }
 return min_sell_price;
}


void Exchange::PrintBidAskSpread(std::ostream &os) const {
 os << "Asset Bid Ask Spread (in alphabetical order):\n";
 std::set<string> assets_set; //used a set so there won't be duplicates
 for (auto &p : portfolio) {
   for (auto &x : p.second) {
     if (x.second != "USD") { //this function excludes USD
       assets_set.insert(x.second); //creating a set of all the assets
     }
   }
 }
 for (auto asset_var : assets_set) {
 //iterating through each asset to find the bid and ask
   int highest_open_buy = FindHighestOpenBuy(asset_var);
   string HOB_str = std::to_string(highest_open_buy);
   if (highest_open_buy == 0) {
     HOB_str = "NA";
   }
   int lowest_open_sell = FindLowestOpenSell(asset_var);
   string LOS_str = std::to_string(lowest_open_sell);
   if (lowest_open_sell == 200000000) {
     LOS_str = "NA";
   }
   os << asset_var << ": Highest Open Buy = " << HOB_str << " USD and Lowest Open Sell = " << LOS_str << " USD\n";
 }
}
