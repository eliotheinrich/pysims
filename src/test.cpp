#include "MinCutSimulator.h"
#include "Graph.h"
#include <iostream>
#include <assert.h>
#include <DataFrame.hpp>
#include <ctpl.h>

//#include <boost/test/unit_test.hpp>

int test_compute(int id, uint i) {
    return i*i;
}

bool test_future() {
    std::vector<std::future<int>> futures(10);
    ctpl::thread_pool threads(4);

    for (uint i = 0; i < futures.size(); i++) {
        futures[i] = threads.push(test_compute, i);
    }

    for (uint i = 0; i < futures.size(); i++) {
        int k = futures[i].get();
        std::cout << "k = " << k << std::endl;
    }

}


int main() {
    test_future();
}





