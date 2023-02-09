#pragma once

#include <atomic>

namespace light::rhi
{
    class Spin
    {
    public:
        Spin() = default;

        Spin(const Spin&) = delete;

        Spin& operator= (const Spin&) = delete;

        void lock()
        {
            while (flag_.test_and_set())
            {

            }
        }

        void unlock()
        {
            flag_.clear();
        }
    private:
        std::atomic_flag flag_;
    };
}
