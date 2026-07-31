#pragma once
#include <Arduino.h>
#include <lv_conf.h>
#include <esp32_smartdisplay.h>

#include "AppConfig.h"
#include "ValveManager.h"
#include "Scheduler.h"
#include "ProgramsPage.h"
#include "runtime/RuntimeManager.h"
#include "time/TimeManager.h"

class DisplayManager
{
public:
    void begin(
        ValveManager& valveManager,
        Scheduler& scheduler,
        RuntimeManager& runtimeManager,
        TimeManager& timeManager
    );
    void update();

    void refreshValve(uint8_t index);
    void showMessage(const char* message);

private:
    enum class Page : uint8_t
    {
        Manual = 0,
        Programs,
        Status,
        Setup,
        Count
    };

    struct ValveWidgets
    {
        lv_obj_t* card = nullptr;
        lv_obj_t* statusDot = nullptr;
        lv_obj_t* nameLabel = nullptr;
        lv_obj_t* stateLabel = nullptr;
        lv_obj_t* button = nullptr;
        lv_obj_t* buttonLabel = nullptr;
        lv_obj_t* counterLabel = nullptr;
    };


    ValveManager* valveManager_ = nullptr;
    Scheduler* scheduler_ = nullptr;
    RuntimeManager* runtimeManager_ = nullptr;
    TimeManager* timeManager_ = nullptr;

    ValveWidgets widgets_[AppConfig::DISPLAYED_VALVE_COUNT];
    ProgramsPage programsPage_;

    lv_obj_t* pages_[static_cast<uint8_t>(Page::Count)] = {};
    lv_obj_t* navButtons_[static_cast<uint8_t>(Page::Count)] = {};
    lv_obj_t* navLabels_[static_cast<uint8_t>(Page::Count)] = {};

    lv_obj_t* clockLabel_ = nullptr;
    lv_obj_t* modeLabel_ = nullptr;
    lv_obj_t* pageTitleLabel_ = nullptr;
    lv_obj_t* toast_ = nullptr;

    lv_obj_t* runtimeOverlay_ = nullptr;
    lv_obj_t* runtimeProgramLabel_ = nullptr;
    lv_obj_t* runtimeValveLabel_ = nullptr;
    lv_obj_t* runtimeRemainingLabel_ = nullptr;
    lv_obj_t* runtimeProgressBar_ = nullptr;
    lv_obj_t* runtimeStopButton_ = nullptr;

    lv_obj_t* statusUptimeLabel_ = nullptr;
    lv_obj_t* statusValve1Label_ = nullptr;
    lv_obj_t* statusValve2Label_ = nullptr;
    lv_obj_t* statusPulseLabel_ = nullptr;
    lv_obj_t* statusActiveLabel_ = nullptr;
    lv_obj_t* statusNextLabel_ = nullptr;
    lv_obj_t* statusTimeLabel_ = nullptr;

    lv_obj_t* brightnessValueLabel_ = nullptr;
    lv_obj_t* pulseValueLabel_ = nullptr;
    lv_obj_t* brightnessSlider_ = nullptr;
    lv_obj_t* pulseSlider_ = nullptr;

    Page activePage_ = Page::Manual;
    uint32_t toastHideAtMs_ = 0;
    uint32_t lastClockUpdateMs_ = 0;
    uint32_t lastStatusUpdateMs_ = 0;
    uint8_t brightnessPercent_ = 80;
    bool runtimeOverlayVisible_ = false;
    int16_t lastRuntimeProgramIndex_ = -1;

    static DisplayManager* instance_;

    static void valveButtonEvent(lv_event_t* event);
    static void valveStateChanged(uint8_t index);
    static void navigationEvent(lv_event_t* event);
    static void brightnessSliderEvent(lv_event_t* event);
    static void pulseSliderEvent(lv_event_t* event);
    static void runtimeStopEvent(lv_event_t* event);

    void createDashboard();
    void createHeader(lv_obj_t* screen);
    void createPages(lv_obj_t* screen);
    void createManualPage(lv_obj_t* parent);
    void createStatusPage(lv_obj_t* parent);
    void createSetupPage(lv_obj_t* parent);
    void createValveCard(lv_obj_t* parent, uint8_t index, int x);
    void createFooter(lv_obj_t* screen);
    void createRuntimeOverlay(lv_obj_t* screen);

    void showPage(Page page);
    void updateClock();
    void updateStatus();
    void updateToast();
    void updateRuntimeOverlay();
    void applyBrightness(uint8_t percent);
    void applyPulseDuration(uint32_t durationMs);
    
};
