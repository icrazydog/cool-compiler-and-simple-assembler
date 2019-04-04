#include <unordered_map>
#include <map>
#include <vector>
#include <algorithm>
#include <string>
#include <iostream>
using namespace std;

struct CmpByValue {
  bool operator()(const pair<string,int >& lhs, const pair<string,int>& rhs) {
    return lhs.second < rhs.second;
  }
};


int main(){
    unordered_map<string, int> hmap;
    hmap["test"]=1;
    hmap["test-2"]=2;
    if(hmap.find("test") != hmap.end()){
        printf("%s:%d\n","test",hmap["test"]);
    }
    cout<<endl;



    map<string, int> omap;
    omap["a"]=80;
     omap["b"]=100;
     omap["c"]=50;
     omap.insert(make_pair("d",110));

    vector<pair<string,int > > vec;
    for (map<string, int>::iterator iter = omap.begin();
        iter != omap.end();
        ++iter) {
        cout << iter->first<<":"<<iter->second << endl;
        vec.push_back(make_pair(iter->first, iter->second));
    }

    sort(vec.begin(), vec.end(),CmpByValue());

    cout<<endl<<"ordered"<<endl;
    for (vector<pair<string,int >>::iterator iter = vec.begin();
        iter != vec.end();
        ++iter) {
        cout << iter->first<<":"<<iter->second << endl;
    }

    return 0;
}