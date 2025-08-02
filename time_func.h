
#ifndef _TIME_FUNC_H
#define _TIME_FUNC_H

#include <thread>
#include <chrono>

using time_val_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

inline time_val_t xfn_time()
{
    return std::chrono::high_resolution_clock::now();
}

inline double xfn_difftime(time_val_t start, time_val_t end)
{
    return (double)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
}

inline double xfn_nano_difftime(time_val_t start, time_val_t end)
{
	return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

inline void xfn_delay(int ms)
{
	//std::this_thread::sleep_for(std::chrono_literals::operator""ms(ms));
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

inline void xfn_mcsec_delay(int mcsec)
{
	std::this_thread::sleep_for(std::chrono::microseconds(mcsec));
}

inline uint64_t xfn_time_point()
{
	auto now = std::chrono::high_resolution_clock::now();
	return (uint64_t)std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
}

//#include <iostream>
//#include <chrono>
//#include <iomanip>

// Get the current time point
inline uint64_t xfn_current_time(std::time_t *current_time)
{
    auto now = std::chrono::system_clock::now();

    // Convert the time point to a time_t
    *current_time = std::chrono::system_clock::to_time_t(now);
    //std::time_t current_time = std::chrono::system_clock::to_time_t(now);
    //std::localtime(&current_time)
    //std::tm* currentTimeStruct = std::gmtime(&current_time);

    // Get the milliseconds part
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch() % std::chrono::seconds(1));
    return milliseconds.count();

    // Print the current time with milliseconds
    //char buf[64];
    //if (strftime(buf, sizeof(buf), "%H:%M:%S\n", std::localtime(&current_time)))

    //std::cout << "Current time with milliseconds: " << std::put_time(std::localtime(&currentTimeT), "%Y-%m-%d %H:%M:%S") << "." << milliseconds.count() << std::endl;

    //return 0;
}
#endif // _TIME_FUNC_H
