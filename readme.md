# CsvReader In C++

## Features

- Fast and easy to use
- Single header file , no dependency
- trim space left and right
- code easy and simple to read
- `O(n)` in parsing one row string

## TODO
- [x] Writer, based on std::ostream
- [ ] ~~Serialization and deserialization~~ ⚠ Abandoned: too complex

## Time complexity

`O(n)` in parsing one row string

copy `string` as `CsvItem`, pushback to `std::vector`, 

no more additional spend

## Usage
- Single header file, no dependency
- Copy file `csv.hpp` to your project


```cpp
#include "csv.hpp"

auto f = "data/orders.csv";
auto reader = CsvReader<>::open_file(f); // open a csv file

// or parse a csv fmt string
auto reader =  CsvReader(content); // assume content here is a csv fmt string

// typcially we don't need to handle the header, just skip it
reader.skip(); // skip one line
```

```cpp
// parse next one row, using `bool next(CsvRow&)`
// row is almost same as std::vector<CsvItem>
// but `[]` accept size_t index or std::string key
CsvRow row;
while(reader.next(row)) { // when content is end, next return false
    size_t index = row[0];
    std::string name = row[1].str();
    // std::string address = row[2]; // you can easily skip some useless column
    int32_t age = row[3];
    double salary = row[4];
    // ...
}
```

Using `size_t` as index is faster, `std::string` is human friendly, but also has more negtive effect on performance,

using `std::map<std::string, size_t>` to store the map relation.

```cpp
// using std::string as key, first need to make a header dict
auto header = "tradeAcc,code,clientId,volume,direction,beginTime,endTime,algoType";
auto dict = make_header_dict(header);

// then you can use std::string as key
CsvRow row(dict);
while(reader.next(row)) {
    std::string trade_acc = row["tradeAcc"].str();
    std::string code = row["code"].str();
    // ...
}
```
> I prefer using `size_t` as index, only fast

if no header dict, and using `std::string` as key, this is a Undefined-Behavior, don't do this.

```cpp
auto reader = CsvReader<>::open_file(f);
CsvRow row;
while(reader.next(row)) {
    std::string trade_acc = row["tradeAcc"].str(); // UB, don't do this
    // ...
}
```

## BenchMark

Time for parsing csv fmt string, using `size_t` as index

- CPU: macbook pro m3
- OS: macOS 15.5

|row| column| time |
|--|--|--|
|1k|8|784us|
|10k|8| 5ms |
|1 million|8|243ms|
|100 million|8| 26.656s|

using `a.ipynb` to generate the bench data => `data/orders.csv`


## Writer
example: `writer.cpp`

```cpp
std::ofstream f("wdemo.csv");

CsvWriter writer(f);
writer << "abc" << "def" << skip << skip << eol;
```

Any sub of `std::ostream` is ok,

like `std::stringstream` `std::cout` `std::ofstream`


## End

Any pr is welcome, fork and pr.