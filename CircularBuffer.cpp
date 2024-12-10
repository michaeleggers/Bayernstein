#include "CircularBuffer.h"

#include <stdint.h>

CircularBuffer::CircularBuffer(uint32_t maxSize) {
    m_buffer.resize(maxSize);
    m_maxSize = maxSize;
    m_pos     = 0;
    m_isFull  = false;
}

void CircularBuffer::Push(std::string str) {
    m_buffer[ m_pos ] = str;
    if (
        ++m_pos
        == m_maxSize ) { // more efficient wraparound than with modulo: https://embeddedartistry.com/blog/2017/05/17/creating-a-circular-buffer-in-c-and-c/#comment-21639
        m_pos    = 0;
        m_isFull = true;
    }
}

bool CircularBuffer::Get(uint32_t offset, std::string* str) {
    if ( offset >= Size() || Empty() ) return false;
    int pos = m_pos - 1 - offset;
    if ( pos < 0 ) pos += m_maxSize;
    *str = m_buffer[ pos ];
    return true;
}

bool CircularBuffer::Empty() {
    return m_pos == 0 && !m_isFull;
}

int CircularBuffer::Size() {
    return m_isFull ? m_maxSize : m_pos;
}

int CircularBuffer::MaxSize() {
    return m_maxSize;
}
