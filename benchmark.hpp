/*
 * Benchmarking Tool
 * By:  Matthew Evans
 * File:  benchmark.hpp
 * Version:  012322 (WIP)
 *
 * See LICENSE.md for copyright information.
 *
 * Run a benchmark, recording time elapsed to a log file.
 * Template is used to cast to a duration type for logging.
 * See:  https://en.cppreference.com/w/cpp/chrono/duration
 * 
 * Log file:  \benchmark_log.txt
 * 
 * Example:
 * 
 * benchmark my_bench = benchmark<std::chrono::microseconds>("My Benchmark");
 * my_bench.start();
 *   ~~~ do something ~~~
 * my_bench.stop();
 * 
 */

#ifndef WTF_BENCHMARK_HPP
#define WTF_BENCHMARK_HPP

#include <string>
#include <fstream>
#include <chrono>
#include <mutex>

namespace wtf {

/*!
 * \class benchmark
 * \brief Run a benchmark 
 * \tparam T See https://en.cppreference.com/w/cpp/chrono/duration
 */
template <typename T>
class benchmark {
    public:
        /*!
         * \brief General initialization, see specializations below.
         */
        benchmark(const std::string& label) : benchmark_label(label), time_label("nanoseconds") {};

        benchmark() = delete;    //!<  Delete default constructor.
        ~benchmark() = default;  //!<  Default destructor.

        /*!
         * \brief Start benchmark.
         */
        void start(void) { start_bench = std::chrono::system_clock::now(); };

        /*!
         * \brief Stop benchmark and log to file.
         * Logs all benchmarks to: benchmark\log.txt
         */
        void stop(void) {
            end_bench = std::chrono::system_clock::now();
            std::chrono::system_clock::duration total_time = end_bench - start_bench;
            std::time_t start_time = std::chrono::system_clock::to_time_t(start_bench);
            std::time_t end_time = std::chrono::system_clock::to_time_t(end_bench);

            bench_mtx.lock();  //  Lock so multiple threads don't write at once.
            std::ofstream benchmark_log;
            benchmark_log.open("benchmark/log.txt", std::ios::app);
            benchmark_log << "Benchmark:  " << benchmark_label << std::endl;
            benchmark_log << "Started at:  " << std::ctime(&start_time);
            benchmark_log << "Completed at:  " << std::ctime(&end_time);
            if(total_time == std::chrono::system_clock::duration::zero()) {
                benchmark_log << "Internal clock did not tick during benchmark";
            } else {
                benchmark_log << "Total time:  " <<
                    std::chrono::duration_cast<T>(end_bench - start_bench).count() << 
                    " " << time_label;
            }
            benchmark_log << std::endl << std::endl;
            benchmark_log.close();
            bench_mtx.unlock();
        };

    private:
        void verify(void) {
            static_assert(
                std::is_same_v<T, std::chrono::nanoseconds> ||
                std::is_same_v<T, std::chrono::microseconds> ||
                std::is_same_v<T, std::chrono::milliseconds> ||
                std::is_same_v<T, std::chrono::seconds> ||
                std::is_same_v<T, std::chrono::minutes> ||
                std::is_same_v<T, std::chrono::hours>,
                "Incorrect benchmark type."
            );
        };
        std::mutex bench_mtx;               //  Thread safety for logging
        const std::string benchmark_label;  //  Name of benchmark
        const std::string time_label;       //  Duration type for logging
        //  Start / end points for benchmark:
        std::chrono::system_clock::time_point start_bench, end_bench;
};

/*
 * Constructor specializations for different duration types.
 * Depending on the template used, the correct duration type will be displayed in the log.
 */

/*!
 * \brief Benchmark in nanoseconds.
 */
template <> inline benchmark<std::chrono::nanoseconds>::benchmark(const std::string& label) :
benchmark_label(label), time_label("nanoseconds") {};

/*!
 * \brief Benchmark in microseconds.
 */
template <> inline benchmark<std::chrono::microseconds>::benchmark(const std::string& label) :
benchmark_label(label), time_label("microseconds") {};

/*!
 * \brief Benchmark in milliseconds.
 */
template <> inline benchmark<std::chrono::milliseconds>::benchmark(const std::string& label) :
benchmark_label(label), time_label("milliseconds") {};

/*!
 * \brief Benchmark in seconds.
 */
template <> inline benchmark<std::chrono::seconds>::benchmark(const std::string& label) :
benchmark_label(label), time_label("seconds") {};

/*!
 * \brief Benchmark in minutes.
 */
template <> inline benchmark<std::chrono::minutes>::benchmark(const std::string& label) :
benchmark_label(label), time_label("minutes") {};

/*!
 * \brief Benchmark in hours.
 */
template <> inline benchmark<std::chrono::hours>::benchmark(const std::string& label) :
benchmark_label(label), time_label("hours") {};

}  //  end namespace wtf

#endif
