#include "season/SeasonManager.h"

#include <cmath>

#include "settings/SettingsManager.h"
#include "time/TimeManager.h"

namespace
{
constexpr float PI_F = 3.14159265358979323846f;
constexpr float DEG_TO_RAD_F = PI_F / 180.0f;
}

void SeasonManager::begin(
    TimeManager& timeManager,
    SettingsManager& settingsManager)
{
    timeManager_ = &timeManager;
    settingsManager_ = &settingsManager;
    update();
    Serial.println("SeasonManager initialisiert");
}

void SeasonManager::update()
{
    if (timeManager_ == nullptr ||
        settingsManager_ == nullptr ||
        !timeManager_->isValid())
    {
        valid_ = false;
        explanation_ =
            "Warte auf gültige Uhrzeit und Standortdaten.";
        return;
    }

    struct tm local = {};
    if (!timeManager_->getLocalTime(local))
    {
        valid_ = false;
        explanation_ = "Lokale Zeit ist nicht verfügbar.";
        return;
    }

    const int32_t dayKey =
        (local.tm_year + 1900) * 1000 + local.tm_yday;

    if (dayKey == lastDayKey_ && valid_)
    {
        return;
    }

    calculate(local);
    lastDayKey_ = dayKey;
}

bool SeasonManager::isValid() const
{
    return valid_;
}

uint8_t SeasonManager::seasonPercent() const
{
    return seasonPercent_;
}

float SeasonManager::dayLengthHours() const
{
    return dayLengthHours_;
}

uint16_t SeasonManager::sunriseMinutes() const
{
    return sunriseMinutes_;
}

uint16_t SeasonManager::sunsetMinutes() const
{
    return sunsetMinutes_;
}

const String& SeasonManager::seasonName() const
{
    return seasonName_;
}

const String& SeasonManager::explanation() const
{
    return explanation_;
}

void SeasonManager::calculate(const struct tm& local)
{
    const float latitude = settingsManager_->latitude();
    const float longitude = settingsManager_->longitude();

    if (!std::isfinite(latitude) ||
        !std::isfinite(longitude) ||
        latitude < -90.0f || latitude > 90.0f ||
        longitude < -180.0f || longitude > 180.0f)
    {
        valid_ = false;
        explanation_ = "Ungültige GPS-Koordinaten im Setup.";
        return;
    }

    const int dayOfYear = local.tm_yday + 1;
    const float latitudeRad = latitude * DEG_TO_RAD_F;

    // Näherung der Sonnen-Deklination für den aktuellen Kalendertag.
    const float declination =
        23.44f * DEG_TO_RAD_F *
        std::sin(
            2.0f * PI_F *
            (284.0f + static_cast<float>(dayOfYear)) /
            365.0f
        );

    float cosineHourAngle =
        -std::tan(latitudeRad) * std::tan(declination);

    cosineHourAngle = constrain(
        cosineHourAngle,
        -1.0f,
        1.0f
    );

    const float hourAngle = std::acos(cosineHourAngle);
    dayLengthHours_ = 24.0f * hourAngle / PI_F;

    // Für die Anzeige wird der lokale Mittag als Mittelpunkt verwendet.
    // Der Saisonfaktor selbst hängt nur von der Tageslänge ab.
    const float sunriseHour = 12.0f - dayLengthHours_ / 2.0f;
    const float sunsetHour = 12.0f + dayLengthHours_ / 2.0f;

    sunriseMinutes_ = static_cast<uint16_t>(
        constrain(
            static_cast<int>(std::round(sunriseHour * 60.0f)),
            0,
            1439
        )
    );

    sunsetMinutes_ = static_cast<uint16_t>(
        constrain(
            static_cast<int>(std::round(sunsetHour * 60.0f)),
            0,
            1439
        )
    );

    seasonPercent_ = static_cast<uint8_t>(
        constrain(
            static_cast<int>(std::round(
                interpolateFactor(dayLengthHours_)
            )),
            30,
            120
        )
    );

    const bool firstHalf = dayOfYear <= 172;

    if (dayLengthHours_ < 9.0f)
    {
        seasonName_ = "Winter";
    }
    else if (dayLengthHours_ < 11.0f)
    {
        seasonName_ = firstHalf ? "Vorfrühling" : "Spätherbst";
    }
    else if (dayLengthHours_ < 13.5f)
    {
        seasonName_ = firstHalf ? "Frühling" : "Herbst";
    }
    else if (dayLengthHours_ < 15.0f)
    {
        seasonName_ = firstHalf ? "Spätfrühling" : "Spätsommer";
    }
    else
    {
        seasonName_ = "Sommer";
    }

    explanation_ =
        String("Standort ") +
        String(latitude, 3) + "°, " +
        String(longitude, 3) + "° · " +
        "Tageslänge " +
        String(dayLengthHours_, 1) + " h · " +
        "Sonnenaufgang ca. " +
        formatMinutes(sunriseMinutes_) + " · " +
        "Sonnenuntergang ca. " +
        formatMinutes(sunsetMinutes_);

    valid_ = true;

    Serial.printf(
        "Saisonautomatik: %s, %.1f h Tageslicht, %u %%\n",
        seasonName_.c_str(),
        dayLengthHours_,
        static_cast<unsigned>(seasonPercent_)
    );
}

float SeasonManager::interpolateFactor(float hours)
{
    struct Point
    {
        float hours;
        float percent;
    };

    static constexpr Point points[] =
    {
        {  6.0f, 30.0f },
        {  8.0f, 40.0f },
        { 10.0f, 60.0f },
        { 12.0f, 80.0f },
        { 14.0f, 100.0f },
        { 16.0f, 115.0f },
        { 18.0f, 120.0f }
    };

    if (hours <= points[0].hours)
    {
        return points[0].percent;
    }

    constexpr size_t count =
        sizeof(points) / sizeof(points[0]);

    for (size_t i = 1; i < count; ++i)
    {
        if (hours <= points[i].hours)
        {
            const float span =
                points[i].hours - points[i - 1].hours;

            const float position =
                (hours - points[i - 1].hours) / span;

            return
                points[i - 1].percent +
                position *
                (points[i].percent - points[i - 1].percent);
        }
    }

    return points[count - 1].percent;
}

String SeasonManager::formatMinutes(uint16_t minutes)
{
    /*
     * uint16_t erlaubt theoretisch mehr als 99 Stunden.
     * Der größere Puffer verhindert deshalb zuverlässig eine
     * mögliche Abschneidung und beseitigt -Wformat-truncation.
     */
    char buffer[9] = {};

    std::snprintf(
        buffer,
        sizeof(buffer),
        "%02u:%02u",
        static_cast<unsigned>(minutes / 60U),
        static_cast<unsigned>(minutes % 60U)
    );

    return String(buffer);
}
