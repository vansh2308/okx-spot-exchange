#include "models/regression_model.h"
#include "core/logger.h"
#include "core/utils.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>

namespace models {

RegressionModel::RegressionModel(ModelType type)
    : modelType_(type),
      polynomialDegree_(2),
      quantile_(0.5) {
    // Constructor
}

RegressionModel::~RegressionModel() {
    // Destructor
}

void RegressionModel::setModelType(ModelType type) {
    modelType_ = type;
}

void RegressionModel::setPolynomialDegree(int degree) {
    if (degree < 1) {
        core::Logger::getInstance().warn("Invalid polynomial degree: {}, must be >= 1", degree);
        return;
    }
    
    polynomialDegree_ = degree;
}

void RegressionModel::setQuantile(double quantile) {
    if (quantile < 0.0 || quantile > 1.0) {
        core::Logger::getInstance().warn("Invalid quantile: {}, must be between 0 and 1", quantile);
        return;
    }
    
    quantile_ = quantile;
}

void RegressionModel::addTrainingPoint(double x, double y) {
    xData_.push_back(x);
    yData_.push_back(y);
}

void RegressionModel::setTrainingData(const std::vector<double>& x, const std::vector<double>& y) {
    if (x.size() != y.size()) {
        core::Logger::getInstance().error("X and Y data must have the same size");
        return;
    }
    
    xData_ = x;
    yData_ = y;
}

bool RegressionModel::train() {
    if (xData_.empty() || yData_.empty()) {
        core::Logger::getInstance().warn("Cannot train model with empty data");
        return false;
    }
    
    if (xData_.size() != yData_.size()) {
        core::Logger::getInstance().error("X and Y data must have the same size");
        return false;
    }
    
    bool success = false;
    
    switch (modelType_) {
        case ModelType::LINEAR:
            success = trainLinear();
            break;
        case ModelType::POLYNOMIAL:
            success = trainPolynomial();
            break;
        case ModelType::EXPONENTIAL:
            success = trainExponential();
            break;
        case ModelType::LOGISTIC:
            success = trainLogistic();
            break;
        case ModelType::QUANTILE:
            success = trainQuantile();
            break;
    }
    
    if (success) {
        core::Logger::getInstance().info("Regression model trained successfully with {} data points",
            xData_.size());
    } else {
        core::Logger::getInstance().error("Failed to train regression model");
    }
    
    return success;
}

void RegressionModel::clearTrainingData() {
    xData_.clear();
    yData_.clear();
}

double RegressionModel::predict(double x) const {
    switch (modelType_) {
        case ModelType::LINEAR:
            return predictLinear(x);
        case ModelType::POLYNOMIAL:
            return predictPolynomial(x);
        case ModelType::EXPONENTIAL:
            return predictExponential(x);
        case ModelType::LOGISTIC:
            return predictLogistic(x);
        case ModelType::QUANTILE:
            return predictQuantile(x);
        default:
            return 0.0;
    }
}

std::vector<double> RegressionModel::predictBatch(const std::vector<double>& x) const {
    std::vector<double> predictions;
    predictions.reserve(x.size());
    
    for (double value : x) {
        predictions.push_back(predict(value));
    }
    
    return predictions;
}

double RegressionModel::calculateRSquared() const {
    if (xData_.empty() || yData_.empty() || coefficients_.empty()) {
        return 0.0;
    }
    
    double meanY = std::accumulate(yData_.begin(), yData_.end(), 0.0) / yData_.size();
    
    double totalSumOfSquares = 0.0;
    double residualSumOfSquares = 0.0;
    
    for (size_t i = 0; i < xData_.size(); ++i) {
        double prediction = predict(xData_[i]);
        residualSumOfSquares += std::pow(yData_[i] - prediction, 2);
        totalSumOfSquares += std::pow(yData_[i] - meanY, 2);
    }
    
    if (totalSumOfSquares == 0.0) {
        return 0.0;
    }
    
    return 1.0 - (residualSumOfSquares / totalSumOfSquares);
}

double RegressionModel::calculateMSE() const {
    if (xData_.empty() || yData_.empty() || coefficients_.empty()) {
        return 0.0;
    }
    
    double sumSquaredError = 0.0;
    
    for (size_t i = 0; i < xData_.size(); ++i) {
        double prediction = predict(xData_[i]);
        sumSquaredError += std::pow(yData_[i] - prediction, 2);
    }
    
    return sumSquaredError / xData_.size();
}

double RegressionModel::calculateMAE() const {
    if (xData_.empty() || yData_.empty() || coefficients_.empty()) {
        return 0.0;
    }
    
    double sumAbsError = 0.0;
    
    for (size_t i = 0; i < xData_.size(); ++i) {
        double prediction = predict(xData_[i]);
        sumAbsError += std::abs(yData_[i] - prediction);
    }
    
    return sumAbsError / xData_.size();
}

std::vector<double> RegressionModel::getCoefficients() const {
    return coefficients_;
}

RegressionModel::ModelType RegressionModel::getModelType() const {
    return modelType_;
}

bool RegressionModel::saveModel(const std::string& filepath) const {
    try {
        // Create JSON object
        nlohmann::json modelJson;
        
        // Add model type
        modelJson["model_type"] = static_cast<int>(modelType_);
        modelJson["polynomial_degree"] = polynomialDegree_;
        modelJson["quantile"] = quantile_;
        
        // Add coefficients
        modelJson["coefficients"] = coefficients_;
        
        // Write to file
        std::ofstream file(filepath);
        if (!file.is_open()) {
            core::Logger::getInstance().error("Failed to open file for writing: {}", filepath);
            return false;
        }
        
        file << modelJson.dump(4);
        return true;
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Error saving model: {}", e.what());
        return false;
    }
}

bool RegressionModel::loadModel(const std::string& filepath) {
    try {
        // Read from file
        std::ifstream file(filepath);
        if (!file.is_open()) {
            core::Logger::getInstance().error("Failed to open file for reading: {}", filepath);
            return false;
        }
        
        // Parse JSON
        nlohmann::json modelJson;
        file >> modelJson;
        
        // Load model type
        if (modelJson.contains("model_type")) {
            modelType_ = static_cast<ModelType>(modelJson["model_type"].get<int>());
        }
        
        // Load polynomial degree
        if (modelJson.contains("polynomial_degree")) {
            polynomialDegree_ = modelJson["polynomial_degree"].get<int>();
        }
        
        // Load quantile
        if (modelJson.contains("quantile")) {
            quantile_ = modelJson["quantile"].get<double>();
        }
        
        // Load coefficients
        if (modelJson.contains("coefficients")) {
            coefficients_ = modelJson["coefficients"].get<std::vector<double>>();
        }
        
        return true;
    } catch (const std::exception& e) {
        core::Logger::getInstance().error("Error loading model: {}", e.what());
        return false;
    }
}

std::string RegressionModel::getModelEquation() const {
    std::stringstream equation;
    
    switch (modelType_) {
        case ModelType::LINEAR:
            if (coefficients_.size() >= 2) {
                equation << "y = " << coefficients_[0] << " + " << coefficients_[1] << "x";
            }
            break;
            
        case ModelType::POLYNOMIAL:
            equation << "y = ";
            for (size_t i = 0; i < coefficients_.size(); ++i) {
                if (i > 0) {
                    equation << " + ";
                }
                equation << coefficients_[i];
                if (i > 0) {
                    equation << "x";
                    if (i > 1) {
                        equation << "^" << i;
                    }
                }
            }
            break;
            
        case ModelType::EXPONENTIAL:
            if (coefficients_.size() >= 2) {
                equation << "y = " << coefficients_[0] << " * e^(" << coefficients_[1] << "x)";
            }
            break;
            
        case ModelType::LOGISTIC:
            if (coefficients_.size() >= 3) {
                equation << "y = " << coefficients_[0] << " / (1 + e^(-" << coefficients_[1] << "*(x-" << coefficients_[2] << ")))";
            }
            break;
            
        case ModelType::QUANTILE:
            equation << "Quantile regression (q=" << quantile_ << ")";
            break;
    }
    
    return equation.str();
}

bool RegressionModel::trainLinear() {
    // Use the linear regression utility from core::utils
    auto result = core::utils::linearRegression(xData_, yData_);
    
    // Set coefficients [intercept, slope]
    coefficients_ = {result.intercept, result.slope};
    
    return true;
}

bool RegressionModel::trainPolynomial() {
    size_t n = xData_.size();
    size_t degree = polynomialDegree_;
    
    // Prepare design matrix (Vandermonde matrix)
    std::vector<std::vector<double>> X(n, std::vector<double>(degree + 1, 0.0));
    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j <= degree; ++j) {
            X[i][j] = std::pow(xData_[i], j);
        }
    }
    
    // Normal equation: (X^T X)^(-1) X^T y
    // First compute X^T X
    std::vector<std::vector<double>> XtX(degree + 1, std::vector<double>(degree + 1, 0.0));
    for (size_t i = 0; i <= degree; ++i) {
        for (size_t j = 0; j <= degree; ++j) {
            for (size_t k = 0; k < n; ++k) {
                XtX[i][j] += X[k][i] * X[k][j];
            }
        }
    }
    
    // Compute X^T y
    std::vector<double> Xty(degree + 1, 0.0);
    for (size_t i = 0; i <= degree; ++i) {
        for (size_t k = 0; k < n; ++k) {
            Xty[i] += X[k][i] * yData_[k];
        }
    }
    
    // Simplified approach for polynomial regression using basic linear algebra
    // This is not the most numerically stable approach, but works for our purposes
    
    // Solve the system (XtX) * coefficients = Xty using Gaussian elimination
    std::vector<std::vector<double>> augmented(degree + 1, std::vector<double>(degree + 2, 0.0));
    for (size_t i = 0; i <= degree; ++i) {
        for (size_t j = 0; j <= degree; ++j) {
            augmented[i][j] = XtX[i][j];
        }
        augmented[i][degree + 1] = Xty[i];
    }
    
    // Forward elimination
    for (size_t i = 0; i <= degree; ++i) {
        // Find pivot
        size_t maxRow = i;
        double maxVal = std::abs(augmented[i][i]);
        for (size_t k = i + 1; k <= degree; ++k) {
            if (std::abs(augmented[k][i]) > maxVal) {
                maxVal = std::abs(augmented[k][i]);
                maxRow = k;
            }
        }
        
        // Swap rows if needed
        if (maxRow != i) {
            std::swap(augmented[i], augmented[maxRow]);
        }
        
        // Eliminate below
        for (size_t k = i + 1; k <= degree; ++k) {
            double factor = augmented[k][i] / augmented[i][i];
            for (size_t j = i; j <= degree + 1; ++j) {
                augmented[k][j] -= factor * augmented[i][j];
            }
        }
    }
    
    // Back substitution
    coefficients_.resize(degree + 1, 0.0);
    for (int i = degree; i >= 0; --i) {
        double sum = 0.0;
        for (size_t j = i + 1; j <= degree; ++j) {
            sum += augmented[i][j] * coefficients_[j];
        }
        coefficients_[i] = (augmented[i][degree + 1] - sum) / augmented[i][i];
    }
    
    return true;
}

bool RegressionModel::trainExponential() {
    // For exponential regression, we transform to linear regression
    // y = a * e^(bx) => ln(y) = ln(a) + bx
    
    // Check for valid data (no negative y values)
    for (double y : yData_) {
        if (y <= 0.0) {
            core::Logger::getInstance().error("Exponential regression requires positive y values");
            return false;
        }
    }
    
    // Transform y data
    std::vector<double> lnY;
    lnY.reserve(yData_.size());
    for (double y : yData_) {
        lnY.push_back(std::log(y));
    }
    
    // Linear regression on transformed data
    auto result = core::utils::linearRegression(xData_, lnY);
    
    // Set coefficients [a, b] where y = a * e^(bx)
    coefficients_ = {std::exp(result.intercept), result.slope};
    
    return true;
}

bool RegressionModel::trainLogistic() {
    // Logistic regression using simplified gradient descent
    // y = a / (1 + e^(-b*(x-c)))
    
    // Initialize coefficients [a, b, c]
    // a: upper asymptote
    // b: growth rate
    // c: midpoint
    double maxY = *std::max_element(yData_.begin(), yData_.end());
    double minY = *std::min_element(yData_.begin(), yData_.end());
    double meanX = std::accumulate(xData_.begin(), xData_.end(), 0.0) / xData_.size();
    
    coefficients_ = {maxY - minY, 1.0, meanX};
    
    // Gradient descent parameters
    double learningRate = 0.01;
    int maxIterations = 1000;
    double convergenceThreshold = 0.0001;
    
    for (int iteration = 0; iteration < maxIterations; ++iteration) {
        double prevCost = 0.0;
        for (size_t i = 0; i < xData_.size(); ++i) {
            double prediction = predictLogistic(xData_[i]);
            prevCost += std::pow(yData_[i] - prediction, 2);
        }
        prevCost /= xData_.size();
        
        // Compute gradients
        double gradA = 0.0, gradB = 0.0, gradC = 0.0;
        for (size_t i = 0; i < xData_.size(); ++i) {
            double x = xData_[i];
            double y = yData_[i];
            
            double a = coefficients_[0];
            double b = coefficients_[1];
            double c = coefficients_[2];
            
            double expTerm = std::exp(-b * (x - c));
            double denom = 1.0 + expTerm;
            double prediction = a / denom;
            
            double error = prediction - y;
            
            // Gradients
            gradA += error / denom;
            gradB += error * a * expTerm * (x - c) / (denom * denom);
            gradC += -error * a * expTerm * b / (denom * denom);
        }
        
        gradA /= xData_.size();
        gradB /= xData_.size();
        gradC /= xData_.size();
        
        // Update coefficients
        coefficients_[0] -= learningRate * gradA;
        coefficients_[1] -= learningRate * gradB;
        coefficients_[2] -= learningRate * gradC;
        
        // Check for convergence
        double cost = 0.0;
        for (size_t i = 0; i < xData_.size(); ++i) {
            double prediction = predictLogistic(xData_[i]);
            cost += std::pow(yData_[i] - prediction, 2);
        }
        cost /= xData_.size();
        
        if (std::abs(prevCost - cost) < convergenceThreshold) {
            break;
        }
    }
    
    return true;
}

bool RegressionModel::trainQuantile() {
    // Simplified quantile regression
    // We'll use a linear model for the quantile
    
    // Sort data pairs by x-value
    std::vector<std::pair<double, double>> data;
    for (size_t i = 0; i < xData_.size(); ++i) {
        data.emplace_back(xData_[i], yData_[i]);
    }
    std::sort(data.begin(), data.end());
    
    // Group data into bins
    int numBins = std::min(20, static_cast<int>(data.size() / 5));
    if (numBins < 2) numBins = 2;
    
    std::vector<double> binX;
    std::vector<double> binY;
    
    for (int i = 0; i < numBins; ++i) {
        size_t startIdx = i * data.size() / numBins;
        size_t endIdx = (i + 1) * data.size() / numBins;
        if (i == numBins - 1) endIdx = data.size();
        
        std::vector<double> yValues;
        double sumX = 0.0;
        
        for (size_t j = startIdx; j < endIdx; ++j) {
            sumX += data[j].first;
            yValues.push_back(data[j].second);
        }
        
        double avgX = sumX / (endIdx - startIdx);
        double quantileY = core::utils::percentile(yValues, quantile_);
        
        binX.push_back(avgX);
        binY.push_back(quantileY);
    }
    
    // Linear regression on bin representatives
    auto result = core::utils::linearRegression(binX, binY);
    
    // Set coefficients [intercept, slope]
    coefficients_ = {result.intercept, result.slope};
    
    return true;
}

double RegressionModel::predictLinear(double x) const {
    if (coefficients_.size() < 2) {
        return 0.0;
    }
    
    return coefficients_[0] + coefficients_[1] * x;
}

double RegressionModel::predictPolynomial(double x) const {
    if (coefficients_.empty()) {
        return 0.0;
    }
    
    double result = 0.0;
    for (size_t i = 0; i < coefficients_.size(); ++i) {
        result += coefficients_[i] * std::pow(x, i);
    }
    
    return result;
}

double RegressionModel::predictExponential(double x) const {
    if (coefficients_.size() < 2) {
        return 0.0;
    }
    
    return coefficients_[0] * std::exp(coefficients_[1] * x);
}

double RegressionModel::predictLogistic(double x) const {
    if (coefficients_.size() < 3) {
        return 0.0;
    }
    
    double a = coefficients_[0];
    double b = coefficients_[1];
    double c = coefficients_[2];
    
    return a / (1.0 + std::exp(-b * (x - c)));
}

double RegressionModel::predictQuantile(double x) const {
    // For our simplified implementation, quantile prediction is just linear
    return predictLinear(x);
}

std::vector<double> RegressionModel::generateFeatures(double x) const {
    std::vector<double> features;
    features.reserve(polynomialDegree_ + 1);
    
    for (int i = 0; i <= polynomialDegree_; ++i) {
        features.push_back(std::pow(x, i));
    }
    
    return features;
}

} // namespace models 