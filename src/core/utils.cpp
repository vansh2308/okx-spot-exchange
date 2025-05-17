
#include "core/utils.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <regex>
#include <numeric>

namespace core {
namespace utils {

// String utilities
std::string trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), [](int c) {
        return std::isspace(c);
    });
    
    auto end = std::find_if_not(str.rbegin(), str.rend(), [](int c) {
        return std::isspace(c);
    }).base();
    
    return (start < end) ? std::string(start, end) : std::string();
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

bool caseInsensitiveCompare(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        return false;
    }
    
    return std::equal(a.begin(), a.end(), b.begin(), b.end(),
        [](char a, char b) {
            return std::tolower(a) == std::tolower(b);
        });
}

// Numeric utilities
double round(double value, int decimals) {
    const double multiplier = std::pow(10.0, decimals);
    return std::round(value * multiplier) / multiplier;
}

double parseDouble(const std::string& str, double defaultValue) {
    try {
        return std::stod(str);
    } catch (const std::exception& e) {
        return defaultValue;
    }
}

int parseInt(const std::string& str, int defaultValue) {
    try {
        return std::stoi(str);
    } catch (const std::exception& e) {
        return defaultValue;
    }
}

// Time utilities
std::chrono::system_clock::time_point parseISOTimestamp(const std::string& timestamp) {
    // Parse ISO 8601 format: YYYY-MM-DDThh:mm:ssZ
    std::tm tm = {};
    std::istringstream ss(timestamp);
    
    // Try to parse with millisecond precision
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    
    if (ss.fail()) {
        // Try alternative format
        std::istringstream ss2(timestamp);
        ss2 >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        
        if (ss2.fail()) {
            return std::chrono::system_clock::now(); // Return current time if parsing fails
        }
    }
    
    // Convert tm to time_point
    auto time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    
    // Check if we have milliseconds
    std::regex regex("\\.[0-9]+");
    std::smatch match;
    if (std::regex_search(timestamp, match, regex)) {
        std::string ms_str = match.str().substr(1); // Remove the leading dot
        while (ms_str.length() < 3) ms_str += "0"; // Ensure we have 3 digits
        if (ms_str.length() > 3) ms_str = ms_str.substr(0, 3); // Truncate to milliseconds
        
        auto ms = std::chrono::milliseconds(std::stoi(ms_str));
        time += ms;
    }
    
    return time;
}

std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp) {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()).count() % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%dT%H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms;
    ss << 'Z';
    
    return ss.str();
}

std::chrono::system_clock::time_point currentTime() {
    return std::chrono::system_clock::now();
}

double getElapsedMilliseconds(const std::chrono::system_clock::time_point& start) {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}

double getElapsedMicroseconds(const std::chrono::high_resolution_clock::time_point& start) {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(now - start).count();
}

// Market data utilities
double calculateVWAP(const std::map<double, double>& levels) {
    double totalVolume = 0.0;
    double totalValue = 0.0;
    
    for (const auto& [price, quantity] : levels) {
        totalVolume += quantity;
        totalValue += price * quantity;
    }
    
    if (totalVolume <= 0.0) {
        return 0.0;
    }
    
    return totalValue / totalVolume;
}

double calculateMarketImpact(const std::map<double, double>& levels, double quantity, bool isBuy) {
    if (quantity <= 0.0 || levels.empty()) {
        return 0.0;
    }
    
    double totalCost = 0.0;
    double remainingQuantity = quantity;
    double referencePrice = isBuy ? levels.begin()->first : levels.rbegin()->first;
    
    for (const auto& [price, available] : levels) {
        double take = std::min(remainingQuantity, available);
        totalCost += price * take;
        remainingQuantity -= take;
        
        if (remainingQuantity <= 0.0) {
            break;
        }
    }
    
    if (remainingQuantity > 0.0) {
        // Not enough liquidity in the orderbook
        double lastPrice = isBuy ? levels.rbegin()->first : levels.begin()->first;
        totalCost += lastPrice * remainingQuantity;
    }
    
    double avgPrice = totalCost / quantity;
    return std::abs(avgPrice - referencePrice);
}

std::vector<std::pair<double, double>> calculateCumulativeVolume(const std::map<double, double>& levels) {
    std::vector<std::pair<double, double>> result;
    double cumulativeVolume = 0.0;
    
    for (const auto& [price, quantity] : levels) {
        cumulativeVolume += quantity;
        result.emplace_back(price, cumulativeVolume);
    }
    
    return result;
}

// Statistical utilities
double mean(const std::vector<double>& values) {
    if (values.empty()) {
        return 0.0;
    }
    
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / values.size();
}

double median(const std::vector<double>& values) {
    if (values.empty()) {
        return 0.0;
    }
    
    std::vector<double> sortedValues = values;
    std::sort(sortedValues.begin(), sortedValues.end());
    
    size_t size = sortedValues.size();
    if (size % 2 == 0) {
        return (sortedValues[size / 2 - 1] + sortedValues[size / 2]) / 2.0;
    } else {
        return sortedValues[size / 2];
    }
}

double standardDeviation(const std::vector<double>& values) {
    if (values.size() < 2) {
        return 0.0;
    }
    
    double avg = mean(values);
    double sum = 0.0;
    
    for (double value : values) {
        sum += (value - avg) * (value - avg);
    }
    
    return std::sqrt(sum / (values.size() - 1));
}

double percentile(const std::vector<double>& values, double percentileRank) {
    if (values.empty()) {
        return 0.0;
    }
    
    std::vector<double> sortedValues = values;
    std::sort(sortedValues.begin(), sortedValues.end());
    
    double index = percentileRank * (sortedValues.size() - 1);
    size_t lowerIndex = static_cast<size_t>(std::floor(index));
    size_t upperIndex = static_cast<size_t>(std::ceil(index));
    
    if (lowerIndex == upperIndex) {
        return sortedValues[lowerIndex];
    }
    
    double weight = index - lowerIndex;
    return sortedValues[lowerIndex] * (1.0 - weight) + sortedValues[upperIndex] * weight;
}

double skewness(const std::vector<double>& values) {
    if (values.size() < 3) {
        return 0.0;
    }
    
    double avg = mean(values);
    double stdDev = standardDeviation(values);
    
    if (stdDev == 0.0) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (double value : values) {
        double deviation = (value - avg) / stdDev;
        sum += deviation * deviation * deviation;
    }
    
    return sum / values.size();
}

double kurtosis(const std::vector<double>& values) {
    if (values.size() < 4) {
        return 0.0;
    }
    
    double avg = mean(values);
    double stdDev = standardDeviation(values);
    
    if (stdDev == 0.0) {
        return 0.0;
    }
    
    double sum = 0.0;
    for (double value : values) {
        double deviation = (value - avg) / stdDev;
        sum += deviation * deviation * deviation * deviation;
    }
    
    return sum / values.size() - 3.0; // Excess kurtosis (normal distribution has kurtosis of 3)
}

// Linear regression
RegressionResult linearRegression(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size() || x.empty()) {
        return {0.0, 0.0, 0.0}; // Invalid input
    }
    
    double sumX = std::accumulate(x.begin(), x.end(), 0.0);
    double sumY = std::accumulate(y.begin(), y.end(), 0.0);
    double sumXY = 0.0;
    double sumX2 = 0.0;
    
    size_t n = x.size();
    
    for (size_t i = 0; i < n; ++i) {
        sumXY += x[i] * y[i];
        sumX2 += x[i] * x[i];
    }
    
    double xMean = sumX / n;
    double yMean = sumY / n;
    
    // Calculate slope
    double numerator = sumXY - sumX * sumY / n;
    double denominator = sumX2 - sumX * sumX / n;
    
    if (denominator == 0.0) {
        return {0.0, yMean, 0.0}; // Vertical line, undefined slope
    }
    
    double slope = numerator / denominator;
    double intercept = yMean - slope * xMean;
    
    // Calculate R-squared
    double totalSumOfSquares = 0.0;
    double residualSumOfSquares = 0.0;
    
    for (size_t i = 0; i < n; ++i) {
        double predicted = slope * x[i] + intercept;
        residualSumOfSquares += (y[i] - predicted) * (y[i] - predicted);
        totalSumOfSquares += (y[i] - yMean) * (y[i] - yMean);
    }
    
    double rSquared = 1.0;
    if (totalSumOfSquares > 0.0) {
        rSquared = 1.0 - residualSumOfSquares / totalSumOfSquares;
    }
    
    return {slope, intercept, rSquared};
}

double predict(const RegressionResult& regression, double x) {
    return regression.slope * x + regression.intercept;
}

} // namespace utils
} // namespace core 