#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>

#include "csv.hpp"


int main() {
    using namespace csv;
    
    std::ofstream f("wdemo.csv");
    std::stringstream ss;

    CsvWriter writer(std::cout);
    writer << "abc" << "def" << skip << skip << eol;
    
}