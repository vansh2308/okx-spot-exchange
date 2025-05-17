#pragma once

#include <vector>
#include <map>
#include <string>
#include <functional>

namespace models {

class RegressionModel {
public:
    enum class ModelType {
        LINEAR,
        POLYNOMIAL,
        EXPONENTIAL,
        LOGISTIC,
        QUANTILE
    };

    RegressionModel(ModelType type = ModelType::LINEAR);
    ~RegressionModel();

    // Set model parameters
    void setModelType(ModelType type);
    void setPolynomialDegree(int degree); // For polynomial regression
    void setQuantile(double quantile); // For quantile regression (0.0-1.0)
    
    // Training methods
    void addTrainingPoint(double x, double y);
    void setTrainingData(const std::vector<double>& x, const std::vector<double>& y);
    bool train();
    void clearTrainingData();
    
    // Prediction
    double predict(double x) const;
    std::vector<double> predictBatch(const std::vector<double>& x) const;
    
    // Model evaluation
    double calculateRSquared() const;
    double calculateMSE() const;
    double calculateMAE() const;
    
    // Model parameters
    std::vector<double> getCoefficients() const;
    ModelType getModelType() const;
    
    // Save/load model
    bool saveModel(const std::string& filepath) const;
    bool loadModel(const std::string& filepath);
    
    // String representation
    std::string getModelEquation() const;

private:
    // Model type and parameters
    ModelType modelType_;
    int polynomialDegree_;
    double quantile_;
    
    // Model coefficients
    std::vector<double> coefficients_;
    
    // Training data
    std::vector<double> xData_;
    std::vector<double> yData_;
    
    // Training methods for specific model types
    bool trainLinear();
    bool trainPolynomial();
    bool trainExponential();
    bool trainLogistic();
    bool trainQuantile();
    
    // Prediction methods for specific model types
    double predictLinear(double x) const;
    double predictPolynomial(double x) const;
    double predictExponential(double x) const;
    double predictLogistic(double x) const;
    double predictQuantile(double x) const;
    
    // Helper functions
    std::vector<double> generateFeatures(double x) const;
};

} // namespace models 