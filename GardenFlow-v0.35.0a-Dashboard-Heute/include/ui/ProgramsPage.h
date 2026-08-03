#pragma once

#include <Arduino.h>
#include <lv_conf.h>
#include <esp32_smartdisplay.h>
#include "Scheduler.h"
#include "runtime/RuntimeManager.h"

class DisplayManager;

class ProgramsPage
{
public:
    void begin(lv_obj_t* parent, Scheduler& scheduler, RuntimeManager& runtimeManager, DisplayManager& displayManager);
    void refresh();
    void show();

private:
    static constexpr uint8_t WEEKDAY_COUNT = 7;

    struct ProgramWidgets
    {
        ProgramsPage* owner = nullptr;
        lv_obj_t* card = nullptr;
        lv_obj_t* details = nullptr;
        lv_obj_t* enableSwitch = nullptr;
        lv_obj_t* editButton = nullptr;
        uint8_t programIndex = 0;
    };

    struct WeekdayButtonContext
    {
        ProgramsPage* owner = nullptr;
        uint8_t weekdayIndex = 0;
    };

    struct AddButtonContext
    {
        ProgramsPage* owner = nullptr;
        uint8_t valveIndex = 0;
    };

    lv_obj_t* parent_ = nullptr;
    lv_obj_t* listContainer_ = nullptr;
    Scheduler* scheduler_ = nullptr;
    RuntimeManager* runtimeManager_ = nullptr;
    DisplayManager* displayManager_ = nullptr;
    ProgramWidgets widgets_[Scheduler::MAX_PROGRAMS];
    AddButtonContext addContexts_[Scheduler::VALVE_COUNT];

    lv_obj_t* editorOverlay_ = nullptr;
    lv_obj_t* editorPanel_ = nullptr;
    lv_obj_t* editorEnabledSwitch_ = nullptr;
    lv_obj_t* hourValueLabel_ = nullptr;
    lv_obj_t* minuteValueLabel_ = nullptr;
    lv_obj_t* durationValueLabel_ = nullptr;
    lv_obj_t* weekdayButtons_[WEEKDAY_COUNT] = {};
    WeekdayButtonContext weekdayContexts_[WEEKDAY_COUNT];

    uint8_t editedProgramIndex_ = 0;
    bool draftEnabled_ = false;
    uint8_t draftHour_ = 0;
    uint8_t draftMinute_ = 0;
    uint16_t draftDurationMinutes_ = 1;
    uint8_t draftWeekdays_ = 0;

    void rebuildProgramList();
    void createProgramCard(uint8_t slotIndex, uint8_t numberInValve, int y);
    void updateProgramCard(uint8_t slotIndex);
    void openEditor(uint8_t slotIndex);
    void closeEditor();
    bool saveEditor();
    bool deleteEditedProgram();
    void refreshEditorValues();
    void refreshWeekdayButtons();

    lv_obj_t* createTextButton(lv_obj_t* parent, const char* text, int x, int y,
                               int width, int height, lv_event_cb_t callback,
                               void* userData = nullptr);
    lv_obj_t* createValueLabel(lv_obj_t* parent, int x, int y, int width);

    static void programSwitchEvent(lv_event_t* event);
    static void programCardEvent(lv_event_t* event);
    static void addProgramEvent(lv_event_t* event);
    static void hourMinusEvent(lv_event_t* event);
    static void hourPlusEvent(lv_event_t* event);
    static void minuteMinusEvent(lv_event_t* event);
    static void minutePlusEvent(lv_event_t* event);
    static void durationMinusEvent(lv_event_t* event);
    static void durationPlusEvent(lv_event_t* event);
    static void weekdayEvent(lv_event_t* event);
    static void editorEnabledEvent(lv_event_t* event);
    static void editorSaveEvent(lv_event_t* event);
    static void editorCancelEvent(lv_event_t* event);
    static void editorDeleteEvent(lv_event_t* event);
    static void editorStartNowEvent(lv_event_t* event);
};
