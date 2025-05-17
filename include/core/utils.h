#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <map>

namespace core {
namespace utils {

// String utilities
std::string trim(const std::string& str);
std::vector<std::string> split(const std::string& str, char delimiter);
bool caseInsensitiveCompare(const std::string& a, const std::string& b);

// Numeric utilities
double round(double value, int decimals);
double parseDouble(const std::string& str, double defaultValue = 0.0);
int parseInt(const std::string& str, int defaultValue = 0);

// Time utilities
std::chrono::system_clock::time_point parseISOTimestamp(const std::string& timestamp);
std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp);
std::chrono::system_clock::time_point currentTime();
double getElapsedMilliseconds(const std::chrono::system_clock::time_point& start);
double getElapsedMicroseconds(const std::chrono::high_resolution_clock::time_point& start);

// Market data utilities
double calculateVWAP(const std::map<double, double>& levels);
double calculateMarketImpact(const std::map<double, double>& levels, double quantity, bool isBuy);
std::vector<std::pair<double, double>> calculateCumulativeVolume(const std::map<double, double>& levels);

// Statistical utilities
double mean(const std::vector<double>& values);
double median(const std::vector<double>& values);
double standardDeviation(const std::vector<double>& values);
double percentile(const std::vector<double>& values, double percentileRank);
double skewness(const std::vector<double>& values);
double kurtosis(const std::vector<double>& values);

// Linear regression
struct RegressionResult {
    double slope;
    double intercept;
    double r_squared;
};

RegressionResult linearRegression(const std::vector<double>& x, const std::vector<double>& y);
double predict(const RegressionResult& regression, double x);

} // namespace utils
} // namespace core 