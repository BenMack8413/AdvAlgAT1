#ifndef FILE_READER_HPP
#define FILE_READER_HPP

#include <string>
#include <fstream>
#include <cstddef>

class FileReader {
public:
    explicit FileReader(const std::string& filepath);
    ~FileReader();

    bool open();
    bool get_next_line(std::string& line, size_t& line_number);
    void close();

private:
    std::string filepath_;
    std::ifstream file_stream_;
    size_t current_line_num_ = 0;
};

#endif // FILE_READER_HPP