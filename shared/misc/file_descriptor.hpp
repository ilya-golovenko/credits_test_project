#ifndef _misc_file_descriptor_hpp
#define _misc_file_descriptor_hpp

#pragma once


class file_descriptor
{
public:
    file_descriptor(int fd);

    file_descriptor();
    ~file_descriptor();

    file_descriptor(file_descriptor&& other);
    file_descriptor& operator=(file_descriptor&& other);

    file_descriptor(file_descriptor const&) = delete;
    file_descriptor& operator=(file_descriptor const&) = delete;

    void swap(file_descriptor& other) noexcept;

    bool valid() const;

    void close();

    int get() const;

    operator int() const;

private:
    int fd_;
};

#endif
