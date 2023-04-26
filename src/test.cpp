#include <iostream>
#include <BS_thread_pool.hpp>


uint test(std::unique_ptr<int> i) { return 2*(*i); }

int main() {
    BS::thread_pool pool(3);
    std::vector<std::future<uint>> results(3);

    for (uint i = 0; i < 3; i++) {
        std::unique_ptr<int> k = std::make_unique<int>(i);
        results[i] = pool.submit(test, std::move(k));
        std::cout << results[i].get() << std::endl;
    }

}