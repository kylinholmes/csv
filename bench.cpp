#include "csv.hpp"
#include <iostream>
#include <chrono>


using namespace std;
using namespace csv;

int main(int argc, char* argv[]) {
    auto f = "data/orders.csv";
    if (argc > 1) {
        f = argv[1];
    }
    cout << "file: " << f << endl;
    auto reader = CsvReader<>::open_file(f, true);

    reader.skip(); // skip one line, header
    CsvRow row;
    auto now = std::chrono::high_resolution_clock::now();
    while(reader.next(row)) {

    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - now);
    cout << "time: " << duration.count() << "ns" << endl;
    cout << "time: " << duration.count() / 1'000 << "us" << endl;
    cout << "time: " << duration.count() / 1'000'000 << "ms" << endl;
    return 0;
}