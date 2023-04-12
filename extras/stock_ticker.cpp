// Given a stream of stock prices, design a data structure to suppor
// * initialization of the ticker
// * add or update the price of a stock to the data structure
// * return the top k price stoks and their current prices
class StockTiker{
    public:
        StockTicker(int k){
            k = k;
        }
        // O(log n)
        void addOrUpdate(String stock, double price){
            if (mp.find(stock) != mp.end()){
                st.erase(st.find(
                            {mp[stock],  stock}
                            )
                        )
            }
            mp[stock] = price;
            st.insert({price, stock});
        }
        // O(k)
        vector<pair<String, double> > topK(){
            vector<pair<String, double> > ans;
            int cnt = 0;

            set<pair<double, String>, greater<pair<double, String> > >::iterator itr;

            for (itr = st.begin(); itr != st.end() && cnt < k, itr++){
                ans.push_back(
                        {itr -> second, itr -> first});
                cout << itr -> second << " " << itr -> first << " ";
                cnt++;
            }
            cout << endl;
            return ans;
        }
    private:
        set<pair<double, string>, greater<pair<double, string> > > st;
        unordered_map<string, double> mp;
        int k;
};
