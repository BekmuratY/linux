#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <string>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <getopt.h>

using namespace std;

string get_current_time() {
    auto now = chrono::system_clock::now();
    auto in_time_t = chrono::system_clock::to_time_t(now);
    stringstream ss;
    ss << put_time(localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

string exec(const char* cmd) {
    char buffer[128];
    string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw runtime_error("popen() failed!");
    try {
        while (fgets(buffer, sizeof buffer, pipe) != nullptr) {
            result += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

int fibonacci(int n) {
    if (n <= 1) return n;
    int a = 0, b = 1;
    for (int i = 2; i <= n; ++i) {
        int temp = b;
        b += a;
        a = temp;
    }
    return b;
}

int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int sum_first_n_numbers(int n) {
    return n * (n + 1) / 2;
}

void print_table_row(const vector<string>& row_data) {
    for (const auto& cell : row_data) {
        cout << cell << "\t";
    }
    cout << endl;
}

int main(int argc, char* argv[]) {
    vector<string> headers = {"Time", "System", "Release", "Version", "Machine", "Processor",
                              "CPU Cores", "CPU Threads", "Memory (GB)", "Disk Space (GB)",
                              "GPU", "Network Interfaces", "Peripheral Devices",
                              "Fibonacci_10", "Factorial_10", "Sum_First_100_Numbers"};

    string output_file = "system_info.csv";
    int log_frequency = 1;
    vector<string> tracked_indicators = {"Fibonacci_10", "Factorial_10", "Sum_First_100_Numbers"};

    int opt;
    while ((opt = getopt(argc, argv, "o:l:t:")) != -1) {
        switch (opt) {
            case 'o':
                output_file = optarg;
                break;
            case 'l':
                log_frequency = atoi(optarg);
                break;
            case 't':
                tracked_indicators.push_back(optarg);
                break;
            default:
                cerr << "Usage: " << argv[0] << " [-o output_file] [-l log_frequency] [-t tracked_indicator]" << endl;
                exit(EXIT_FAILURE);
        }
    }

    string current_time = get_current_time();
    string system = exec("uname -s");
    string release = exec("uname -r");
    string version = exec("uname -v");
    string machine = exec("uname -m");
    
    char cpu_brand[256];
    size_t cpu_brand_len = sizeof(cpu_brand);
    sysctlbyname("machdep.cpu.brand_string", &cpu_brand, &cpu_brand_len, nullptr, 0);
    string processor = cpu_brand;
    
    int cpu_cores;
    size_t cpu_cores_len = sizeof(cpu_cores);
    sysctlbyname("hw.physicalcpu", &cpu_cores, &cpu_cores_len, nullptr, 0);
    
    int cpu_threads;
    size_t cpu_threads_len = sizeof(cpu_threads);
    sysctlbyname("hw.logicalcpu", &cpu_threads, &cpu_threads_len, nullptr, 0);
    
    int64_t memory;
    size_t memory_len = sizeof(memory);
    sysctlbyname("hw.memsize", &memory, &memory_len, nullptr, 0);
    memory = memory / (1024 * 1024 * 1024); // GB
    
    struct statvfs stat;
    statvfs("/", &stat);
    int64_t disk_space = (stat.f_blocks * stat.f_frsize) / (1024 * 1024 * 1024); // GB
    
    string gpu_info = exec("system_profiler SPDisplaysDataType | grep Chip");
    string network_info = exec("ifconfig -a | grep 'flags\\|inet '");
    string peripheral_info = exec("system_profiler SPUSBDataType SPThunderboltDataType SPSerialATADataType | grep -e 'Device Speed' -e 'Vendor ID'");

    int fib_10 = fibonacci(10);
    int fact_10 = factorial(10);
    int sum_100 = sum_first_n_numbers(100);

    vector<string> row_data = {
        current_time, system, release, version, machine, processor,
        to_string(cpu_cores), to_string(cpu_threads), to_string(memory), to_string(disk_space),
        gpu_info, network_info, peripheral_info,
        to_string(fib_10), to_string(fact_10), to_string(sum_100)
    };

    // Вывод заголовков таблицы
    print_table_row(headers);
    // Вывод данных таблицы
    print_table_row(row_data);

    return 0;
}
