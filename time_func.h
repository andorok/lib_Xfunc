
#ifndef _TIME_FUNC_H
#define _TIME_FUNC_H

#include <thread>
#include <chrono>

using time_val_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

inline time_val_t get_time()
{
    return std::chrono::high_resolution_clock::now();
}

inline double get_difftime(time_val_t start, time_val_t end)
{
    return (double)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

inline double get_nano_difftime(time_val_t start, time_val_t end)
{
	return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

inline void make_delay(int ms)
{
	//std::this_thread::sleep_for(std::chrono_literals::operator""ms(ms));
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void make_mcsec_delay(int mcsec)
{
	std::this_thread::sleep_for(std::chrono::microseconds(mcsec));
}

#endif // _TIME_FUNC_H
