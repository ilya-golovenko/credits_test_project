#ifndef _misc_filedesc_hpp
#define _misc_filedesc_hpp

#pragma once

#include <cstddef>


class filedesc
{
public:
    filedesc(int fd);

    filedesc();
    ~filedesc();

    filedesc(filedesc&& other);
    filedesc& operator=(filedesc&& other);

    filedesc(filedesc const&) = delete;
    filedesc& operator=(filedesc const&) = delete;

    void swap(filedesc& other);

    void close();

    operator int() const;

private:
    int fd_;
};

#endif
