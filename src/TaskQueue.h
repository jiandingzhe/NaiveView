#pragma once

#include <atomic>
#include <vector>
#include <cassert>

// one-reader-one-writer lock-free queue
template <typename T> class TaskQueue
{
    struct UsageLocation
    {
        unsigned head;
        unsigned len;
    };

  public:
    TaskQueue(unsigned capacity) : m_content(capacity)
    {
    }
    ~TaskQueue() = default;

    unsigned getCapacity() const
    {
        return (unsigned)m_content.size();
    }

    bool add(T *value)
    {
        assert(value != nullptr);
        UsageLocation curr_usage = m_usage.load();
        if (curr_usage.len == m_content.size())
            return false;

        // set value
        unsigned i_set = (curr_usage.head + curr_usage.len) % getCapacity();
        m_content[i_set] = value;

        // update location
        while (true)
        {
            auto set_usage = curr_usage;
            set_usage.len += 1;
            if (m_usage.compare_exchange_weak(curr_usage, set_usage))
                break;
        }
        return true;
    }

    T *fetch()
    {
        UsageLocation curr_usage = m_usage.load();
        if (curr_usage.len == 0)
            return nullptr;

        // fetch value
        unsigned i_fetch = curr_usage.head % getCapacity();
        T *result = m_content[i_fetch];
        assert(result != nullptr);
#ifndef NDEBUG
        m_content[i_fetch] = nullptr;
#endif
        // update location
        while (true)
        {
            auto set_usage = curr_usage;
            set_usage.head += 1;
            set_usage.head = set_usage.head % getCapacity();
            set_usage.len -= 1;
            if (m_usage.compare_exchange_weak(curr_usage, set_usage))
                break;
        }
        return result;
    }

  private:
    std::vector<T *> m_content;
    std::atomic<UsageLocation> m_usage{{0u, 0u}};
};
