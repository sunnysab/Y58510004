//
// Created by sunnysab on 3/21/24.
//

#ifndef PARALLEL_FILEMAPPER_H
#define PARALLEL_FILEMAPPER_H


#include <stdexcept>
#include <cstddef>
#include <cerrno>
#include <cstring>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "exception.h"

class FileMapper {
private:
    /// File descriptor.
    int fd = 0;

    /// Start address of file content loaded in memory.
    uint8_t *start = nullptr;
    /// File size.
    size_t size = 0;

    /// Raw file name.
    const char *filename = nullptr;

public:
    FileMapper(const char *filename) : filename(filename) {}

    ~FileMapper() { close(); }

    uint8_t *get_start() const {
        return start;
    }

    size_t get_size() const {
        return size;
    }

    void load() {
        // Open file and get file descriptor.
        this->fd = open(this->filename, O_RDONLY);
        if (this->fd == -1) {
            auto error_message = strerror(errno);
            auto message = "failed to open file " + std::string(this->filename) + ": " + error_message;
            throw Exception(message);
        }

        // Get file size.
        struct stat file_stat{};
        if (fstat(this->fd, &file_stat) == -1) {
            // Close file descriptor, clean the environment.
            ::close(this->fd);
            this->fd = 0;

            auto error_message = strerror(errno);
            auto message = "failed to get file stat of " + std::string(this->filename) + ": " + error_message;
            throw Exception(message);
        }
        this->size = file_stat.st_size;

        // Map file content to memory.
        auto addr = mmap(nullptr, 0, PROT_READ, MAP_PRIVATE, fd, 0);
        this->start = reinterpret_cast<uint8_t *>(addr);
        if (this->start == MAP_FAILED) {
            // Close file descriptor, clean the environment.
            ::close(this->fd);
            this->start = nullptr;
            this->size = 0;

            auto error_message = strerror(errno);
            auto message = "failed to map file " + std::string(this->filename) + ": " + error_message;
            throw Exception(message);
        }

    }

    void close() {
        if (nullptr != this->start) {
            munmap(this->start, 0);
            ::close(this->fd);

            this->start = nullptr;
            this->fd = 0;
            this->size = 0;
        }
    }
};


#endif //PARALLEL_FILEMAPPER_H
