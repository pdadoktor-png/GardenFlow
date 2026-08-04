#pragma once

#include <Arduino.h>

class WeatherManager;
class SmartControlManager;

enum class AdvisorAction : uint8_t
{
    NoData,
    Pause,
    Reduce,
    Normal,
    Increase
};

struct AdvisorFactor
{
    String name;
    String value;
    int16_t contributionPercent = 0;
};

struct AdvisorRecommendation
{
    static constexpr uint8_t MAX_REASONS = 5;
    static constexpr uint8_t MAX_FACTORS = 5;

    bool valid = false;
    AdvisorAction action = AdvisorAction::NoData;
    int16_t adjustmentPercent = 0;
    uint8_t confidencePercent = 0;

    String headline;
    String summary;
    String narrative;

    String reasons[MAX_REASONS];
    uint8_t reasonCount = 0;

    AdvisorFactor factors[MAX_FACTORS];
    uint8_t factorCount = 0;
};

class AdvisorEngine
{
public:
    void begin(
        WeatherManager& weatherManager,
        SmartControlManager& smartControlManager
    );

    void update();

    const AdvisorRecommendation& recommendation() const;

private:
    WeatherManager* weatherManager_ = nullptr;
    SmartControlManager* smartControlManager_ = nullptr;

    AdvisorRecommendation recommendation_;
    uint32_t lastEvaluationMs_ = 0;

    void evaluate();
    void clearRecommendation();
    void addReason(const String& reason);
    void addFactor(
        const String& name,
        const String& value,
        int16_t contributionPercent
    );

    static int16_t clampAdjustment(int16_t value);
};
