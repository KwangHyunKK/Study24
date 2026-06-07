#include <iostream>
#include <chrono>

// Chrono/calendar example with comments explaining each step.
void run_chrono_practice() {
    using namespace std::chrono; // bring chrono symbols into scope for convenience
    std::cout << "--- Chrono (calendar) Practice ---\n";

    // Obtain current system_clock time point, then floor to whole days.
    // 'floor<days>' truncates the time_point to the day resolution.
    auto today = floor<days>(system_clock::now());

    // Construct a year_month_day from the day-based time_point (C++20 calendaring)
    year_month_day ymd{today};

    // Print year, month, day. Cast to integral types to format numeric output.
    std::cout << "Today: " << int(ymd.year()) << '-' << unsigned(ymd.month()) << '-' << unsigned(ymd.day()) << '\n';

    // Duration arithmetic: combine minutes and seconds using chrono literals
    auto dur = 90min + 30s; // 90 minutes and 30 seconds
    // Convert duration to minutes for display using duration_cast
    std::cout << "Duration: " << duration_cast<minutes>(dur).count() << " minutes\n";
}
