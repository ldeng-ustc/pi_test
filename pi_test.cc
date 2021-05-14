#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "fmt/format.h"
#include "pi/pi.h"

using namespace std;
using fmt::format;

string BuildKey(int id) {
    return format("key{:010}", id);
}

string BuildSK(int sk) {
    return format("sk{:010}", sk);
}

string BuildValue(int sk, int val) {
    return format("sk{:010}val{:030}", sk, val);
}


int main() {
    const int N = 100;
    const int M = 7;

    PiOptions opt{};
    opt.remove_existing_db = true;
    opt.sketch_width = 300;
    opt.sketch_height = 5;
    opt.index_threshold = 1;
    opt.parser = new Parser(12);
    Pi pi("/tmp/db", opt);

    for(int i=0;i<N;i++) {
        pi.Put(BuildKey(i), BuildValue(i%M, i));
    }
    string val;
    for(int i=0;i<N;i++) {
        pi.Get(BuildKey(i), &val);
        assert(val == BuildValue(i%M, i));
    }

    vector<string> vals;
    for(int i=0;i<M;i++) {
        pi.Get2(BuildSK(i), &vals);
        cout << format("{}: {}\n", i, vals.size());
        assert(vals.size() == N / M + (N%M > i));
        vals.clear();
    }

    for(int i=0;i<M;i++) {
        for(int j=0;j<M/2;j++) {
            pi.Get2(BuildSK(j), &vals);
        }
    }
    vals.clear();

    for(int i=0;i<M;i++) {
        pi.Get2(BuildSK(i), &vals);
        cout << format("{}: {}\n", i, vals.size());
        assert(vals.size() == N / M + (N%M > i));
        vals.clear();
    }
    
    return 0;
}