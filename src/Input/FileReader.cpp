#include "FileReader.hpp"

FileReader::FileReader(const std::string& filepath) : filepath_(filepath) {}

FileReader::~FileReader() {
    close();
}

bool FileReader::open() {
    file_stream_.open(filepath_);
    return file_stream_.is_open();
}

bool FileReader::get_next_line(std::string& line, size_t& line_number) {
    if (std::getline(file_stream_, line)) {
        current_line_num_++;
        line_number = current_line_num_;
        return true;
    }
    return false;
}

void FileReader::close() {
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
}