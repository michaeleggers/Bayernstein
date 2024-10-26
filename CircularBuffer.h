
#ifndef CIRCULARBUFFER_H
#define CIRCULARBUFFER_H

#include <string>
#include <vector>


/** Circular string buffer. */
class CircularBuffer {
private:
    std::vector<std::string> m_buffer;
    int m_maxSize;
    int m_pos;
    bool m_isFull;

public:
    CircularBuffer(uint maxSize);

    /** Add the given string to the buffer. */
    void Push(std::string str);
    /**
     * Get the value at the given offset. The most recent value is at `0`, the oldest is at `size - 1`.
     * @returns `false` if the offset is out of range, `true` otherwise.
     */
    bool Get(uint offset, std::string* str);
    bool Empty();
    int Size();
    int MaxSize();
};

#endif // CIRCULARBUFFER_H
