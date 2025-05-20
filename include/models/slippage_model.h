#pragma once

#include <memory>
#include <vector>
#include <map>
#include <string>
#include "core/orderbook.h"
#include "core/utils.h"
#include <Eigen/Dense>

namespace models {

class SlippageModel {
public:
    enum class ModelType {
        LINEAR_REGRESSION,
        QUANTILE_REGRESSION,
        ORDERBOOK_BASED
    };

    struct DataPoint {
        double volume;
        double spread;
        double volatility;
        double timeOfDay;
        double slippage;
    };

    SlippageModel(ModelType type = ModelType::ORDERBOOK_BASED);
    ~SlippageModel();

    // Set model parameters
    void setModelType(ModelType type);
    void setDataPoints(const std::vector<double>& quantities, const std::vector<double>& slippages);
    void setDataPoints(const std::vector<DataPoint>& dataPoints);
    
    // Train the model with historical data
    bool train();
    bool trainQuantileRegression();
    Eigen::VectorXd fitQuantileRegression(const Eigen::MatrixXd& X, const Eigen::VectorXd& y, double quantile);
    
    // Predict slippage for a given order
    double predictSlippage(const std::shared_ptr<core::OrderBook>& orderBook, 
                         double quantity, 
                         bool isBuy) const;
    double predictLinearSlippage(double quantity) const;
    double predictQuantileSlippage(double quantity, double quantile = 0.5) const;
    double predictOrderBookSlippage(const std::shared_ptr<core::OrderBook>& orderBook, 
                                 double quantity, 
                                 bool isBuy) const;
    
    // Calculate slippage from actual order book data
    double calculateSlippage(const std::shared_ptr<core::OrderBook>& orderBook,
                           double quantity,
                           bool isBuy) const;
    
    // Calculate slippage for a range of quantities
    std::map<double, double> calculateSlippageProfile(const std::shared_ptr<core::OrderBook>& orderBook,
                                                   double maxQuantity,
                                                   bool isBuy,
                                                   int steps) const;
    
    // Get model parameters
    ModelType getModelType() const;
    core::utils::RegressionResult getRegressionResult() const;
    double getR2() const;

    // For quantile regression feature-based model
    void setCurrentSpread(double spread) { currentSpread_ = spread; }
    void setCurrentVolatility(double volatility) { currentVolatility_ = volatility; }
    double getTimeOfDay() const;

private:
    // Model type and parameters
    ModelType modelType_;
    core::utils::RegressionResult regression_;
    
    // Training data for simple regression
    std::vector<double> quantityData_;
    std::vector<double> slippageData_;
    
    // For quantile regression feature-based model
    std::vector<DataPoint> trainingData_;
    std::vector<double> quantiles_;
    std::vector<Eigen::VectorXd> coefficients_;
    std::map<std::string, double> featureWeights_;
    double currentSpread_ = 0.0;
    double currentVolatility_ = 0.0;

    // Metrics
    void calculateModelMetrics();
    double calculateMAE(const std::vector<double>& predictions, const std::vector<double>& actuals) const;
    double calculateMSE(const std::vector<double>& predictions, const std::vector<double>& actuals) const;
    double calculateR2(const std::vector<double>& predictions, const std::vector<double>& actuals) const;
};

} // namespace models 