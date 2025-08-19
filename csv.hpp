/**
 * @brief simple csv (de)serializer lib for c++
 * @author kylin
 * @date 2025-08-19
 * @version 0.0.3
 * @license MIT
 */

#pragma once
#include <ostream>
#include <string>
#include <string_view>
#include <vector>
#include <map>
#include <fstream>


#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <sys/mman.h>


namespace csv {

struct CsvItem {
    std::string_view value;
    CsvItem(const CsvItem& other) : value(other.value) {}
    CsvItem(std::string_view sv)  : value(sv) {}
    CsvItem& operator=(CsvItem&& other) {
        value = std::move(other.value);
        return *this;
    }
    CsvItem& operator=(std::string&& other) {
        value = std::move(other);
        return *this;
    }
    CsvItem& operator=(const std::string& other) {
        value = other;
        return *this;
    }
    operator std::string() { return value.data(); }
    operator int() const {
        return std::stoi(value.data());
    }
    operator double() const {
        return std::stod(value.data());
    }
    operator int64_t() const {
        return std::stoll(value.data());
    }
    operator size_t() const {
        return std::stoull(value.data());
    }
};

std::ostream& operator<<(std::ostream& os, const CsvItem& item) {
    os << item.value;
    return os;
}

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
    std::string_view content;
    size_t index;
    char* _data;

    CsvReader(std::string& content)  : content(content), index(0), _data(nullptr) { }
    CsvReader(const std::string_view& content)  : content(content), index(0), _data(nullptr) { }
    CsvReader(const std::string_view& content, char* data)  : content(content), index(0), _data(data) { }
    ~CsvReader() {
        if (_data) {
            munmap(_data, content.size());
        }
    }
    static CsvReader open_file(const std::string& file, bool throw_error = true) {
        int fd = open(file.c_str(), O_RDONLY);
        if (fd == -1 && throw_error) {
            throw std::runtime_error("cannot open file: " + file);
        }
        struct stat sb;
        fstat(fd, &sb);
        size_t length = sb.st_size;
        char* data = (char*)mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);
        CsvReader rd(std::string_view(data, length), data);
        return rd;
    }

    /**
        @brief skip next line
        @return void
    */
    void skip() {
        size_t offset = index;
        while(offset < content.size() && content[offset] != line_end) {
            offset++;
        }
        index = offset + 1;
    }

    /**
        @param row the row to fill with data
        @brief read next row, fill the row with data
        @note &row will be cleared
        @return true if read successfully, false if no more rows
    */
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
                    first_c = !first_c?head=cursor,true:first_c;
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
        index[row[i]] = i;
    }
    return index;
}



struct skip_t {};
struct end_of_line_t {};

constexpr skip_t skip{};
constexpr end_of_line_t eol{};

template<typename ostream_t, char delimiter = ',', char line_end = '\n'>
struct CsvWriter {
    CsvWriter(ostream_t& os) : os(os), is_first(true) {}
    ~CsvWriter() {
        os.flush();
    }
    ostream_t& os;
    bool is_first = true;

private:
    // override of endline_t 
    template <typename T>
    typename std::enable_if<
        std::is_same<typename std::decay<T>::type, end_of_line_t>::value,
        CsvWriter &>::type
    write_impl(T &&) {
        os << line_end;
        is_first = true;
        os.flush();
        return *this;
    }

    // override of skip_t 
    template<typename T>
    typename std::enable_if<std::is_same<typename std::decay<T>::type, skip_t>::value, CsvWriter&>::type
    write_impl(T&&) {
        if (!is_first) {
            os << delimiter;
        }
        is_first = false;
        return *this;
    }
    
    // override of other
    template <typename T>
    typename std::enable_if<
        !std::is_same<typename std::decay<T>::type, skip_t>::value &&
            !std::is_same<typename std::decay<T>::type, end_of_line_t>::value,
        CsvWriter &>::type
    write_impl(T &&data) {
      if (!is_first) {
        os << delimiter;
      }
      os << data;
      is_first = false;
      return *this;
    }

public:
    template<typename T>
    CsvWriter& operator<<(T&& data) {
        return write_impl(std::forward<T>(data));
    }
};


} // end of namespace `csv`