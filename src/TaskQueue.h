#pragma once

#include <atomic>
#include <cassert>
#include <vector>

// one-reader-one-writer lock-free ring buffer
template <typename T> class TaskQueue
{
    struct UsageLocation
    {
        unsigned head;
        unsigned len;
    };

  public:
    TaskQueue(unsigned capacity) : m_content(capacity + 1)
    {
    }
    ~TaskQueue() = default;

    unsigned getNumFilled() const
    {
        unsigned curr_begin = m_begin;
        unsigned curr_end = m_end;
        // unwrap index
        if (curr_end < curr_begin)
            curr_end += m_content.size();
        return curr_end - curr_begin;
    }

    unsigned getCapacity() const
    {
        return (unsigned)m_content.size() - 1;
    }

    bool add(const T &value)
    {
        unsigned curr_begin = m_begin;
        unsigned curr_end = m_end;
        // unwrap index
        unsigned logic_end = curr_end;
        if (logic_end < curr_begin)
            logic_end += m_content.size();

        unsigned curr_size = logic_end - curr_begin;
        if (curr_size >= getCapacity())
            return false;

        // set value
        m_content[curr_end] = value;
        // update index
        curr_end += 1;
        if (curr_end >= m_content.size())
            curr_end -= m_content.size();
        m_end = curr_end;
        return true;
    }

    bool fetch(T &re)
    {
        unsigned curr_begin = m_begin;
        unsigned curr_end = m_end;
        // unwrap index
        if (curr_end < curr_begin)
            curr_end += m_content.size();

        unsigned curr_size = curr_end - curr_begin;
        if (curr_size == 0)
            return false;

        // get value
        re = std::move(m_content[curr_begin]);

        // update index
        curr_begin += 1;
        if (curr_begin >= m_content.size())
            curr_begin -= m_content.size();
        m_begin = curr_begin;
        return true;
    }

  private:
    std::vector<T> m_content;
    std::atomic<unsigned> m_begin{0};
    std::atomic<unsigned> m_end{0};
};
