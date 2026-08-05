#include "advisor/AdvisorEngine.h"

#include "smart/SmartControlManager.h"
#include "weather/WeatherManager.h"

namespace
{
constexpr uint32_t EVALUATION_INTERVAL_MS = 30000UL;
}

void AdvisorEngine::begin(
    WeatherManager& weatherManager,
    SmartControlManager& smartControlManager)
{
    weatherManager_ = &weatherManager;
    smartControlManager_ = &smartControlManager;

    evaluate();
    Serial.println("AdvisorEngine initialisiert");
}

void AdvisorEngine::update()
{
    if (lastEvaluationMs_ != 0 &&
        static_cast<uint32_t>(millis() - lastEvaluationMs_) <
            EVALUATION_INTERVAL_MS)
    {
        return;
    }

    evaluate();
}

const AdvisorRecommendation&
AdvisorEngine::recommendation() const
{
    return recommendation_;
}

void AdvisorEngine::evaluate()
{
    lastEvaluationMs_ = millis();
    clearRecommendation();

    if (weatherManager_ == nullptr ||
        smartControlManager_ == nullptr)
    {
        recommendation_.headline = "Advisor nicht bereit";
        recommendation_.summary =
            "Die benötigten Systemmodule fehlen.";
        recommendation_.narrative =
            "Sobald Wetter- und Smart-Control-Daten verfügbar sind, "
            "erstelle ich eine Empfehlung.";
        return;
    }

    recommendation_.seasonPercent =
        smartControlManager_->seasonPercent();

    if (!weatherManager_->isConfigured())
    {
        recommendation_.headline = "Wetterzugang fehlt";
        recommendation_.summary =
            "OpenWeather im Setup konfigurieren.";
        recommendation_.narrative =
            "Ohne Wetterdaten kann ich den Wasserbedarf nicht "
            "zuverlässig beurteilen.";
        return;
    }

    if (!weatherManager_->isValid())
    {
        recommendation_.headline = "Wetterdaten fehlen";
        recommendation_.summary =
            weatherManager_->lastError().length() > 0
                ? weatherManager_->lastError()
                : String("Wetter zunächst aktualisieren.");
        recommendation_.narrative =
            "Bitte aktualisiere zuerst die Wetterdaten.";
        return;
    }

    recommendation_.valid = true;

    const float temperature =
        weatherManager_->temperatureC();

    const uint8_t humidity =
        weatherManager_->humidityPercent();

    const float rainMm =
        weatherManager_->rainMmNext24Hours();

    const uint8_t rainProbability =
        weatherManager_->maxRainProbabilityPercent();

    int16_t temperatureContribution = 0;
    int16_t humidityContribution = 0;
    int16_t rainContribution = 0;
    uint8_t confidence = 70;

    if (temperature >= 35.0f)
    {
        temperatureContribution = 30;
        confidence += 8;
        addReason("Sehr hohe Temperatur erhöht den Wasserbedarf.");
    }
    else if (temperature >= 30.0f)
    {
        temperatureContribution = 20;
        confidence += 7;
        addReason("Hohe Temperatur erhöht die Verdunstung.");
    }
    else if (temperature >= 26.0f)
    {
        temperatureContribution = 10;
        confidence += 5;
        addReason("Der warme Tag erhöht den Wasserbedarf leicht.");
    }
    else if (temperature < 15.0f)
    {
        temperatureContribution = -25;
        confidence += 6;
        addReason("Die niedrige Temperatur senkt den Wasserbedarf.");
    }
    else if (temperature < 18.0f)
    {
        temperatureContribution = -10;
        confidence += 4;
        addReason("Der kühle Tag reduziert die Verdunstung.");
    }
    else
    {
        addReason("Die Temperatur liegt im Normalbereich.");
    }

    addFactor(
        "Temperatur",
        String(temperature, 1) + " °C",
        temperatureContribution
    );

    if (humidity < 35)
    {
        humidityContribution = 10;
        confidence += 5;
        addReason("Die niedrige Luftfeuchte begünstigt Austrocknung.");
    }
    else if (humidity > 80)
    {
        humidityContribution = -10;
        confidence += 4;
        addReason("Die hohe Luftfeuchte reduziert die Verdunstung.");
    }

    addFactor(
        "Luftfeuchte",
        String(humidity) + " %",
        humidityContribution
    );

    if (weatherManager_->automaticPauseActive())
    {
        rainContribution = -100;
        confidence = 95;
    }
    else if (rainMm >= 2.0f)
    {
        rainContribution = -25;
        confidence += 7;
        addReason("Erwarteter Regen reduziert den Bewässerungsbedarf.");
    }
    else if (rainProbability >= 60)
    {
        rainContribution = -30;
        confidence += 7;
        addReason("Die hohe Regenwahrscheinlichkeit spricht für weniger Wasser.");
    }
    else if (rainProbability >= 40)
    {
        rainContribution = -15;
        confidence += 5;
        addReason("Möglicher Regen wird vorsichtig berücksichtigt.");
    }
    else
    {
        addReason("Es wird kein bedeutsamer Regen erwartet.");
    }

    addFactor(
        "Regen",
        String(rainMm, 1) + " mm / " +
            String(rainProbability) + " %",
        rainContribution
    );

    addFactor(
        "Saison-Grundwert",
        String(recommendation_.seasonPercent) + " %",
        0
    );

    int16_t weatherAdjustment =
        temperatureContribution +
        humidityContribution +
        rainContribution;

    weatherAdjustment =
        clampAdjustment(weatherAdjustment);

    recommendation_.weatherAdjustmentPercent =
        weatherAdjustment;

    if (weatherManager_->automaticPauseActive() ||
        weatherAdjustment <= -60)
    {
        recommendation_.action = AdvisorAction::Pause;
        recommendation_.weatherAdjustmentPercent = -100;
        recommendation_.adjustmentPercent = -100;
        recommendation_.combinedPercent = 0;
        recommendation_.confidencePercent = 95;
        recommendation_.headline =
            "Bewässerung heute aussetzen";
        recommendation_.summary =
            "Der saisonale Grundwert bleibt erhalten, "
            "aber die Wetterlage empfiehlt heute eine Pause.";
        recommendation_.narrative =
            "Für heute wird ausreichend Niederschlag erwartet. "
            "Ich empfehle deshalb, die Bewässerung auszusetzen.";
        return;
    }

    const int32_t combined =
        (
            static_cast<int32_t>(
                recommendation_.seasonPercent
            ) *
            (100 + weatherAdjustment) +
            50
        ) /
        100;

    recommendation_.combinedPercent =
        static_cast<uint16_t>(
            constrain(
                combined,
                10L,
                250L
            )
        );

    recommendation_.adjustmentPercent =
        static_cast<int16_t>(
            recommendation_.combinedPercent
        ) - 100;

    recommendation_.confidencePercent =
        static_cast<uint8_t>(
            constrain(
                static_cast<int>(confidence),
                0,
                98
            )
        );

    if (weatherAdjustment <= -10)
    {
        recommendation_.action = AdvisorAction::Reduce;
        recommendation_.headline =
            "Wetterbedingt kürzer bewässern";
        recommendation_.summary =
            String("Saison-Grundwert ") +
            String(recommendation_.seasonPercent) +
            " %, danach Wetterkorrektur " +
            String(weatherAdjustment) +
            " %.";
        recommendation_.narrative =
            "Die Saison legt zuerst die Grundlaufzeit fest. "
            "Die aktuelle Wetterlage reduziert diese Grundlaufzeit zusätzlich.";
    }
    else if (weatherAdjustment >= 10)
    {
        recommendation_.action = AdvisorAction::Increase;
        recommendation_.headline =
            "Wetterbedingt länger bewässern";
        recommendation_.summary =
            String("Saison-Grundwert ") +
            String(recommendation_.seasonPercent) +
            " %, danach Wetterkorrektur +" +
            String(weatherAdjustment) +
            " %.";
        recommendation_.narrative =
            "Die Saison legt zuerst die Grundlaufzeit fest. "
            "Die warme oder trockene Wetterlage erhöht diese Grundlaufzeit.";
    }
    else
    {
        recommendation_.action = AdvisorAction::Normal;
        recommendation_.headline =
            "Saisonale Grundlaufzeit verwenden";
        recommendation_.summary =
            String("Saison-Grundwert: ") +
            String(recommendation_.seasonPercent) +
            " %. Keine wesentliche Wetterkorrektur.";
        recommendation_.narrative =
            "Die Wetterlage liegt im normalen Bereich. "
            "Es gilt die saisonal eingestellte Grundlaufzeit.";
    }
}

void AdvisorEngine::clearRecommendation()
{
    recommendation_ = AdvisorRecommendation();
}

void AdvisorEngine::addReason(const String& reason)
{
    if (recommendation_.reasonCount >=
        AdvisorRecommendation::MAX_REASONS)
    {
        return;
    }

    recommendation_.reasons[
        recommendation_.reasonCount
    ] = reason;

    ++recommendation_.reasonCount;
}

void AdvisorEngine::addFactor(
    const String& name,
    const String& value,
    int16_t contributionPercent)
{
    if (recommendation_.factorCount >= AdvisorRecommendation::MAX_FACTORS) return;
    AdvisorFactor& factor = recommendation_.factors[recommendation_.factorCount];
    factor.name = name;
    factor.value = value;
    factor.contributionPercent = contributionPercent;
    ++recommendation_.factorCount;
}

int16_t AdvisorEngine::clampAdjustment(int16_t value)
{
    if (value < -100)
    {
        return -100;
    }

    if (value > 50)
    {
        return 50;
    }

    return value;
}
