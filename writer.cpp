#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>

#include "csv.hpp"


int main() {
    using namespace csv;
    
    std::ofstream f("wdemo.csv");
    std::stringstream ss;

    CsvWriter writer(f);
    writer << "abc" << "def" << skip << skip << eol;
    
    std::cout << ss.str();
}