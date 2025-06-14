/**
 * @brief simple csv deserializer lib for c++
 * @author kylin
 * @date 2025-06-14
 * @version 0.0.1
 * @license MIT
 */

#pragma once
#include <string>
#include <vector>
#include <map>
#include <fstream>

struct CsvItem {
    std::string value;
    CsvItem(const CsvItem& other) : value(other.value) {}
    CsvItem(std::string&& value)  : value(std::move(value)) {}
    CsvItem& operator=(CsvItem&& other) {
        value = std::move(other.value);
        return *this;
    }
    CsvItem& operator=(std::string&& other) {
        value = std::move(other);
        return *this;
    }
    // 添加 const string& 的赋值运算符
    CsvItem& operator=(const std::string& other) {
        value = other;
        return *this;
    }
    operator std::string&() { return value; }
    std::string& str() { return value; }
    operator int() const {
        return std::stoi(value);
    }
    operator double() const {
        return std::stod(value);
    }
    operator int64_t() const {
        return std::stoll(value);
    }
    operator size_t() const {
        return std::stoull(value);
    }
};

using CsvIndexDict = std::map<std::string, size_t>;

struct CsvRow: std::vector<CsvItem> {
    CsvIndexDict index;
    CsvRow() {}
    CsvRow(CsvIndexDict index) : index(index) {}
    CsvItem& operator[](size_t index) {
        return std::vector<CsvItem>::operator[](index);
    }
    CsvItem& operator[](std::string key) {
        size_t idx = index[key];
        return std::vector<CsvItem>::operator[](idx);
    }
};



template<char delimiter = ',', char line_end = '\n'>
struct CsvReader {
    std::string content;
    size_t index;

    CsvReader(std::string& content)  : content(content), index(0) { }
    CsvReader(std::string&& content) : content(std::move(content)), index(0) { }
    CsvReader(std::string_view content)  : content(content.data(), content.size()), index(0) { }
    static CsvReader open_file(const std::string& file, bool throw_error = true) {
        std::ifstream f(file);
        if (!f.is_open() && throw_error) {
            throw std::runtime_error("\ncannot open file: " + file);
        }
        std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        return CsvReader(content);
    }
    void skip() {
        size_t offset = index;
        while(offset < content.size() && content[offset] != line_end) {
            offset++;
        }
        index = offset + 1;
    }
    bool next(CsvRow& row) {
        if( index >= content.size() ) {
            return false;
        }
        row.clear();

        size_t cursor = index;
        size_t head   = index;
        size_t tail   = index;
        bool   first_c =  false;
        while(true) {
            switch (content[cursor]) {
                case line_end:
                    row.push_back(CsvItem(content.substr(head, tail - head + 1)));
                    first_c = false;    
                    head = cursor + 1;
                    tail = cursor + 1;
                    goto ROW_END;
                case delimiter: {
                    row.push_back(CsvItem(content.substr(head, tail - head + 1)));
                    first_c = false;
                    head = cursor + 1;
                    tail = cursor + 1;
                    break;
                }
                case ' ':
                case '\r':
                    break;
                default: {
                    if(!first_c) { 
                        first_c = true;
                        head = cursor;
                    }
                    tail = cursor;
                    break;
                }
            }
            cursor++;
            if(cursor >= content.size()) {
                break;
            }
        }
        ROW_END:
            index = cursor + 1;
        return true;
    }
};

template<char delimiter = ',', char line_end = '\n'>
inline CsvIndexDict make_header_dict(std::string_view header) {
    CsvReader<delimiter, line_end> reader(header);
    CsvRow row;
    reader.next(row);
    CsvIndexDict index;
    for(size_t i = 0; i < row.size(); i++) {
        index[row[i].str()] = i;
    }
    return index;
}