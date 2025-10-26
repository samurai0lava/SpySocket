#include <iostream>
#include <sys/stat.h>
#include <sstream>

bool isLeapYear(int year)
{
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

void epochToUTC(long long epoch, int& year, int& month, int& day, int& hour, int& minute, int& second)
{
    int monthDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    second = epoch % 60;
    epoch /= 60;
    minute = epoch % 60;
    epoch /= 60;
    hour = epoch % 24;
    epoch /= 24;
    year = 1970;

    while (true)
    {
        int daysInYear = isLeapYear(year) ? 366 : 365;
        if (epoch >= daysInYear) {
            epoch -= daysInYear;
            year++;
        }
        else {
            break;
        }
    }
    if (isLeapYear(year)) monthDays[1] = 29;
    month = 0;
    while (epoch >= monthDays[month])
    {
        epoch -= monthDays[month];
        month++;
    }
    day = epoch + 1;
    month += 1;
}


void print_time()
{
    struct stat file_stat;
    int stat_return = stat("/proc/self", &file_stat);
    if (stat_return != 0)
    {
        std::cerr << "Stat Failed" << std::endl;
        return;
    }

    long long epoch_time = file_stat.st_mtime;

    std::cout << epoch_time << std::endl;
    int year, month, day, hour, minute, second;
    epochToUTC(epoch_time, year, month, day, hour, minute, second);
    std::cout << "Date & Time (UTC): "
        << year << "-"
        << (month < 10 ? "0" : "") << month << "-"
        << (day < 10 ? "0" : "") << day << " "
        << (hour < 10 ? "0" : "") << hour << ":"
        << (minute < 10 ? "0" : "") << minute << ":"
        << (second < 10 ? "0" : "") << second
        << std::endl;
}

std::string ft_time_format()
{
    struct stat file_stat;
    long long epoch_time;
    int year, month, day, hour, minute, second;
    std::ostringstream oss;

    if (stat("/proc/self", &file_stat) == -1)
    {
        std::cerr << "Stat Failed" << std::endl;
        return "";
    }

    epoch_time = file_stat.st_mtime;

    // Calculate day of week (0 = Thursday, Jan 1, 1970)
    int dayOfWeek = (epoch_time / 86400 + 4) % 7;
    const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    epochToUTC(epoch_time, year, month, day, hour, minute, second);

    // HTTP/1.1 IMF-fixdate format: Sun, 06 Nov 1994 08:49:37 GMT
    oss << days[dayOfWeek] << ", "
        << (day < 10 ? "0" : "") << day << " "
        << months[month - 1] << " "
        << year << " "
        << (hour < 10 ? "0" : "") << hour << ":"
        << (minute < 10 ? "0" : "") << minute << ":"
        << (second < 10 ? "0" : "") << second << " GMT";

    return oss.str();
}
