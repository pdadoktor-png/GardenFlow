#include "ProgramsPage.h"
#include "DisplayManager.h"
#include "Theme.h"

namespace
{
    constexpr const char* WEEKDAY_NAMES[7] = {"Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};

    void stylePanel(lv_obj_t* object)
    {
        lv_obj_set_style_bg_color(object, Theme::panel(), 0);
        lv_obj_set_style_border_color(object, Theme::border(), 0);
        lv_obj_set_style_border_width(object, 1, 0);
        lv_obj_set_style_radius(object, Theme::CARD_RADIUS, 0);
        lv_obj_set_style_pad_all(object, 8, 0);
    }

    void styleButton(lv_obj_t* button)
    {
        lv_obj_set_style_radius(button, 8, 0);
        lv_obj_set_style_border_width(button, 1, 0);
        lv_obj_set_style_border_color(button, Theme::border(), 0);
        lv_obj_set_style_bg_color(button, Theme::panel(), 0);
        lv_obj_set_style_bg_color(button, lv_palette_main(LV_PALETTE_GREEN), LV_STATE_CHECKED);
    }

    void styleLabel(lv_obj_t* label)
    {
        lv_obj_set_style_text_color(label, Theme::text(), 0);
    }

    void formatWeekdays(uint8_t mask, char* output, size_t outputSize)
    {
        if (output == nullptr || outputSize == 0) return;
        output[0] = '\0';
        if ((mask & 0x7F) == 0x7F)
        {
            snprintf(output, outputSize, "taeglich");
            return;
        }
        size_t used = 0;
        for (uint8_t i = 0; i < 7; ++i)
        {
            if ((mask & static_cast<uint8_t>(1U << i)) == 0) continue;
            const int written = snprintf(output + used, outputSize - used,
                                         used == 0 ? "%s" : ",%s", WEEKDAY_NAMES[i]);
            if (written <= 0 || static_cast<size_t>(written) >= outputSize - used) break;
            used += static_cast<size_t>(written);
        }
        if (used == 0) snprintf(output, outputSize, "keine Tage");
    }
}

void ProgramsPage::begin(lv_obj_t* parent, Scheduler& scheduler, RuntimeManager& runtimeManager, DisplayManager& displayManager)
{
    parent_ = parent;
    scheduler_ = &scheduler;
    runtimeManager_ = &runtimeManager;
    displayManager_ = &displayManager;

    listContainer_ = lv_obj_create(parent_);
    lv_obj_remove_style_all(listContainer_);
    lv_obj_set_size(listContainer_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_pos(listContainer_, 0, 0);
    lv_obj_set_style_bg_opa(listContainer_, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(listContainer_, 0, 0);
    lv_obj_set_style_pad_all(listContainer_, 0, 0);
    lv_obj_add_flag(listContainer_, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(listContainer_, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(listContainer_, LV_SCROLLBAR_MODE_AUTO);

    rebuildProgramList();
}

void ProgramsPage::show()
{
    // Die Seite wird beim Öffnen vollständig neu aufgebaut. Dadurch sind
    // LVGL-Objekte und Scheduler-Daten garantiert synchron.
    rebuildProgramList();
    if (listContainer_ != nullptr)
    {
        lv_obj_scroll_to_y(listContainer_, 0, LV_ANIM_OFF);
        lv_obj_move_foreground(listContainer_);
        lv_obj_invalidate(listContainer_);
    }
}

void ProgramsPage::refresh()
{
    if (scheduler_ == nullptr)
    {
        return;
    }
    for (uint8_t i = 0; i < Scheduler::MAX_PROGRAMS; ++i)
    {
        if (!scheduler_->isProgramUsed(i) || widgets_[i].card == nullptr)
        {
            continue;
        }
        const auto& p = scheduler_->program(i);
        if (p.enabled)
        {
            lv_obj_add_state(widgets_[i].enableSwitch, LV_STATE_CHECKED);
        }
        else
        {
            lv_obj_clear_state(widgets_[i].enableSwitch, LV_STATE_CHECKED);
        }
        updateProgramCard(i);
    }
}

void ProgramsPage::rebuildProgramList()
{
    if (parent_ == nullptr || scheduler_ == nullptr)
    {
        return;
    }

    if (listContainer_ == nullptr || !lv_obj_is_valid(listContainer_))
    {
        listContainer_ = lv_obj_create(parent_);
        lv_obj_remove_style_all(listContainer_);
        lv_obj_set_size(listContainer_, LV_PCT(100), LV_PCT(100));
        lv_obj_set_pos(listContainer_, 0, 0);
        lv_obj_set_style_bg_opa(listContainer_, LV_OPA_TRANSP, 0);
        lv_obj_set_style_border_width(listContainer_, 0, 0);
        lv_obj_set_style_pad_all(listContainer_, 0, 0);
        lv_obj_add_flag(listContainer_, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_scroll_dir(listContainer_, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(listContainer_, LV_SCROLLBAR_MODE_AUTO);
    }

    lv_obj_clean(listContainer_);

    for (auto& widget : widgets_)
    {
        widget = ProgramWidgets{};
    }

    int y = 8;
    for (uint8_t valve = 0; valve < Scheduler::VALVE_COUNT; ++valve)
    {
        lv_obj_t* heading = lv_label_create(listContainer_);
        lv_label_set_text_fmt(heading, "Ventil %u", static_cast<unsigned>(valve + 1));
        styleLabel(heading);
        lv_obj_set_pos(heading, 14, y);
        y += 26;

        uint8_t numberInValve = 0;
        for (uint8_t slot = 0; slot < Scheduler::MAX_PROGRAMS; ++slot)
        {
            if (!scheduler_->isProgramUsed(slot) || scheduler_->program(slot).valveIndex != valve)
            {
                continue;
            }
            ++numberInValve;
            createProgramCard(slot, numberInValve, y);
            y += 68;
        }

        addContexts_[valve].owner = this;
        addContexts_[valve].valveIndex = valve;
        lv_obj_t* addButton = createTextButton(listContainer_, "+ Programm", 14, y, 414, 40,
                                               addProgramEvent, &addContexts_[valve]);
        lv_obj_set_style_bg_color(addButton, Theme::panel(), 0);
        y += 54;
    }

    // Unsichtbares Abschlussobjekt legt die volle Scroll-Höhe fest.
    lv_obj_t* spacer = lv_obj_create(listContainer_);
    lv_obj_remove_style_all(spacer);
    lv_obj_set_size(spacer, 1, 1);
    lv_obj_set_pos(spacer, 0, y);

    lv_obj_update_layout(listContainer_);
    refresh();
}

void ProgramsPage::createProgramCard(uint8_t slotIndex, uint8_t numberInValve, int y)
{
    ProgramWidgets& ui = widgets_[slotIndex];
    ui.owner = this;
    ui.programIndex = slotIndex;
    ui.card = lv_obj_create(listContainer_);
    // Rechts bleiben 38 Pixel bewusst frei. Dieser Bereich dient als
    // sichere Scrollspur und enthält keine bedienbaren Elemente.
    lv_obj_set_size(ui.card, 414, 60);
    lv_obj_set_pos(ui.card, 14, y);
    stylePanel(ui.card);
    lv_obj_clear_flag(ui.card, LV_OBJ_FLAG_SCROLLABLE);
    // Die Karte selbst ist nicht mehr klickbar. Dadurch öffnet ein
    // Scrollversuch nicht versehentlich den Programmeditor.
    lv_obj_clear_flag(ui.card, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t* title = lv_label_create(ui.card);
    lv_label_set_text_fmt(title, "Programm %u", static_cast<unsigned>(numberInValve));
    styleLabel(title);
    lv_obj_set_pos(title, 0, 0);

    ui.details = lv_label_create(ui.card);
    lv_obj_set_width(ui.details, 292);
    lv_label_set_long_mode(ui.details, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_color(ui.details, Theme::textDim(), 0);
    lv_obj_set_pos(ui.details, 0, 26);

    ui.enableSwitch = lv_switch_create(ui.card);
    lv_obj_set_size(ui.enableSwitch, 46, 26);
    lv_obj_set_pos(ui.enableSwitch, 304, 9);
    lv_obj_add_event_cb(ui.enableSwitch, programSwitchEvent, LV_EVENT_VALUE_CHANGED, &ui);

    ui.editButton = lv_btn_create(ui.card);
    lv_obj_set_size(ui.editButton, 42, 36);
    lv_obj_set_pos(ui.editButton, 356, 4);
    styleButton(ui.editButton);
    lv_obj_add_event_cb(ui.editButton, programCardEvent, LV_EVENT_SHORT_CLICKED, &ui);
    lv_obj_t* editLabel = lv_label_create(ui.editButton);
    lv_label_set_text(editLabel, ">");
    lv_obj_center(editLabel);

    updateProgramCard(slotIndex);
}

void ProgramsPage::updateProgramCard(uint8_t slotIndex)
{
    if (scheduler_ == nullptr || !scheduler_->isProgramUsed(slotIndex) || widgets_[slotIndex].details == nullptr)
    {
        return;
    }
    const auto& p = scheduler_->program(slotIndex);
    char days[40];
    formatWeekdays(p.weekdays, days, sizeof(days));
    const bool running = runtimeManager_ != nullptr &&
                         runtimeManager_->isProgramRunning(slotIndex);

    if (running)
    {
        lv_label_set_text_fmt(
            widgets_[slotIndex].details,
            "LAEUFT  |  %u Min.  |  %s",
            static_cast<unsigned>(scheduler_->durationMinutes(slotIndex)),
            days);
    }
    else
    {
        lv_label_set_text_fmt(
            widgets_[slotIndex].details,
            "%02u:%02u  |  %u Min.  |  %s",
            static_cast<unsigned>(p.startHour),
            static_cast<unsigned>(p.startMinute),
            static_cast<unsigned>(scheduler_->durationMinutes(slotIndex)),
            days);
    }

    lv_obj_set_style_border_width(widgets_[slotIndex].card, running ? 2 : 1, 0);
    lv_obj_set_style_border_color(
        widgets_[slotIndex].card,
        running ? Theme::open() : Theme::border(),
        0);
}

void ProgramsPage::openEditor(uint8_t slotIndex)
{
    if (scheduler_ == nullptr || !scheduler_->isProgramUsed(slotIndex))
    {
        return;
    }
    closeEditor();
    editedProgramIndex_ = slotIndex;
    const auto& p = scheduler_->program(slotIndex);
    draftEnabled_ = p.enabled;
    draftHour_ = p.startHour;
    draftMinute_ = p.startMinute;
    draftDurationMinutes_ = scheduler_->durationMinutes(slotIndex);
    draftWeekdays_ = p.weekdays;

    editorOverlay_ = lv_obj_create(lv_layer_top());
    lv_obj_remove_style_all(editorOverlay_);
    lv_obj_set_size(editorOverlay_, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(editorOverlay_, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(editorOverlay_, LV_OPA_60, 0);
    lv_obj_clear_flag(editorOverlay_, LV_OBJ_FLAG_SCROLLABLE);

    editorPanel_ = lv_obj_create(editorOverlay_);
    lv_obj_set_size(editorPanel_, 452, 246);
    lv_obj_center(editorPanel_);
    stylePanel(editorPanel_);
    lv_obj_clear_flag(editorPanel_, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* title = lv_label_create(editorPanel_);
    lv_label_set_text_fmt(title, "Ventil %u - Programm",
                          static_cast<unsigned>(p.valveIndex + 1));
    styleLabel(title);
    lv_obj_set_pos(title, 6, 0);

    lv_obj_t* activeLabel = lv_label_create(editorPanel_);
    lv_label_set_text(activeLabel, "Aktiv");
    lv_obj_set_style_text_color(activeLabel, Theme::textDim(), 0);
    lv_obj_set_pos(activeLabel, 334, 0);

    editorEnabledSwitch_ = lv_switch_create(editorPanel_);
    lv_obj_set_size(editorEnabledSwitch_, 52, 28);
    lv_obj_set_pos(editorEnabledSwitch_, 382, -5);
    lv_obj_add_event_cb(editorEnabledSwitch_, editorEnabledEvent, LV_EVENT_VALUE_CHANGED, this);

    lv_obj_t* timeLabel = lv_label_create(editorPanel_);
    lv_label_set_text(timeLabel, "Start"); styleLabel(timeLabel); lv_obj_set_pos(timeLabel, 6, 48);
    createTextButton(editorPanel_, "-", 70, 36, 42, 38, hourMinusEvent);
    hourValueLabel_ = createValueLabel(editorPanel_, 116, 44, 42);
    createTextButton(editorPanel_, "+", 162, 36, 42, 38, hourPlusEvent);
    lv_obj_t* colon = lv_label_create(editorPanel_); lv_label_set_text(colon, ":"); styleLabel(colon); lv_obj_set_pos(colon, 214, 48);
    createTextButton(editorPanel_, "-", 230, 36, 42, 38, minuteMinusEvent);
    minuteValueLabel_ = createValueLabel(editorPanel_, 276, 44, 42);
    createTextButton(editorPanel_, "+", 322, 36, 42, 38, minutePlusEvent);

    lv_obj_t* durationLabel = lv_label_create(editorPanel_);
    lv_label_set_text(durationLabel, "Dauer"); styleLabel(durationLabel); lv_obj_set_pos(durationLabel, 6, 91);
    createTextButton(editorPanel_, "-", 70, 80, 42, 38, durationMinusEvent);
    durationValueLabel_ = createValueLabel(editorPanel_, 116, 88, 66);
    createTextButton(editorPanel_, "+", 186, 80, 42, 38, durationPlusEvent);
    lv_obj_t* minutes = lv_label_create(editorPanel_); lv_label_set_text(minutes, "Min.");
    lv_obj_set_style_text_color(minutes, Theme::textDim(), 0); lv_obj_set_pos(minutes, 238, 91);

    lv_obj_t* dayLabel = lv_label_create(editorPanel_);
    lv_label_set_text(dayLabel, "Tage"); styleLabel(dayLabel); lv_obj_set_pos(dayLabel, 6, 134);
    for (uint8_t i = 0; i < WEEKDAY_COUNT; ++i)
    {
        weekdayContexts_[i].owner = this;
        weekdayContexts_[i].weekdayIndex = i;
        weekdayButtons_[i] = createTextButton(editorPanel_, WEEKDAY_NAMES[i], 54 + i * 54, 124,
                                               48, 36, weekdayEvent, &weekdayContexts_[i]);
    }

    lv_obj_t* startButton = createTextButton(editorPanel_, "Start", 6, 190, 90, 40, editorStartNowEvent);
    lv_obj_set_style_bg_color(startButton, lv_palette_darken(LV_PALETTE_BLUE, 3), 0);
    lv_obj_t* deleteButton = createTextButton(editorPanel_, "Loeschen", 102, 190, 90, 40, editorDeleteEvent);
    lv_obj_set_style_bg_color(deleteButton, lv_palette_darken(LV_PALETTE_RED, 3), 0);
    createTextButton(editorPanel_, "Abbruch", 198, 190, 110, 40, editorCancelEvent);
    lv_obj_t* saveButton = createTextButton(editorPanel_, "Speichern", 314, 190, 120, 40, editorSaveEvent);
    lv_obj_set_style_bg_color(saveButton, lv_palette_darken(LV_PALETTE_GREEN, 3), 0);
    refreshEditorValues();
    refreshWeekdayButtons();
}

void ProgramsPage::closeEditor()
{
    if (editorOverlay_ != nullptr)
    {
        lv_obj_del(editorOverlay_);
    }
    editorOverlay_ = nullptr;
    editorPanel_ = nullptr;
    editorEnabledSwitch_ = nullptr;
    hourValueLabel_ = nullptr;
    minuteValueLabel_ = nullptr;
    durationValueLabel_ = nullptr;
    for (auto& button : weekdayButtons_)
    {
        button = nullptr;
    }
}

bool ProgramsPage::saveEditor()
{
    if (scheduler_ == nullptr || !scheduler_->isProgramUsed(editedProgramIndex_))
    {
        return false;
    }
    if (!scheduler_->setProgramEnabled(editedProgramIndex_, draftEnabled_) ||
        !scheduler_->setStartTime(editedProgramIndex_, draftHour_, draftMinute_) ||
        !scheduler_->setDurationMinutes(editedProgramIndex_, draftDurationMinutes_))
    {
        return false;
    }
    for (uint8_t i = 0; i < WEEKDAY_COUNT; ++i)
    {
        const bool enabled = (draftWeekdays_ & static_cast<uint8_t>(1U << i)) != 0;
        if (!scheduler_->setWeekday(editedProgramIndex_, static_cast<Scheduler::Weekday>(i), enabled))
        {
            return false;
        }
    }
    updateProgramCard(editedProgramIndex_);
    return true;
}

bool ProgramsPage::deleteEditedProgram()
{
    if (scheduler_ == nullptr || !scheduler_->isProgramUsed(editedProgramIndex_))
    {
        return false;
    }
    if (runtimeManager_ != nullptr && runtimeManager_->isProgramRunning(editedProgramIndex_))
    {
        return false;
    }
    return scheduler_->deleteProgram(editedProgramIndex_);
}

void ProgramsPage::refreshEditorValues()
{
    if (editorEnabledSwitch_)
    {
        if (draftEnabled_) lv_obj_add_state(editorEnabledSwitch_, LV_STATE_CHECKED);
        else lv_obj_clear_state(editorEnabledSwitch_, LV_STATE_CHECKED);
    }
    if (hourValueLabel_) lv_label_set_text_fmt(hourValueLabel_, "%02u", static_cast<unsigned>(draftHour_));
    if (minuteValueLabel_) lv_label_set_text_fmt(minuteValueLabel_, "%02u", static_cast<unsigned>(draftMinute_));
    if (durationValueLabel_) lv_label_set_text_fmt(durationValueLabel_, "%u", static_cast<unsigned>(draftDurationMinutes_));
}

void ProgramsPage::refreshWeekdayButtons()
{
    for (uint8_t i = 0; i < WEEKDAY_COUNT; ++i)
    {
        if (!weekdayButtons_[i]) continue;
        if ((draftWeekdays_ & static_cast<uint8_t>(1U << i)) != 0)
            lv_obj_add_state(weekdayButtons_[i], LV_STATE_CHECKED);
        else
            lv_obj_clear_state(weekdayButtons_[i], LV_STATE_CHECKED);
    }
}

lv_obj_t* ProgramsPage::createTextButton(lv_obj_t* parent, const char* text, int x, int y,
                                         int width, int height, lv_event_cb_t callback, void* userData)
{
    lv_obj_t* button = lv_btn_create(parent);
    lv_obj_set_size(button, width, height);
    lv_obj_set_pos(button, x, y);
    styleButton(button);
    // SHORT_CLICKED wird bei einer Scrollbewegung nicht ausgelöst.
    lv_obj_add_event_cb(button, callback, LV_EVENT_SHORT_CLICKED, userData ? userData : this);
    lv_obj_t* label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_center(label);
    return button;
}

lv_obj_t* ProgramsPage::createValueLabel(lv_obj_t* parent, int x, int y, int width)
{
    lv_obj_t* label = lv_label_create(parent);
    lv_obj_set_width(label, width);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    styleLabel(label);
    lv_obj_set_pos(label, x, y);
    return label;
}

void ProgramsPage::programSwitchEvent(lv_event_t* event)
{
    auto* ui = static_cast<ProgramWidgets*>(lv_event_get_user_data(event));
    if (!ui || !ui->owner || !ui->owner->scheduler_) return;
    lv_obj_t* sw = static_cast<lv_obj_t*>(lv_event_get_target(event));
    const bool enabled = lv_obj_has_state(sw, LV_STATE_CHECKED);
    const bool saved = ui->owner->scheduler_->setProgramEnabled(ui->programIndex, enabled);
    if (ui->owner->displayManager_)
        ui->owner->displayManager_->showMessage(saved ? (enabled ? "Programm aktiviert" : "Programm deaktiviert") : "Speichern fehlgeschlagen");
}

void ProgramsPage::programCardEvent(lv_event_t* event)
{
    auto* ui = static_cast<ProgramWidgets*>(lv_event_get_user_data(event));
    if (ui && ui->owner) ui->owner->openEditor(ui->programIndex);
}

void ProgramsPage::addProgramEvent(lv_event_t* event)
{
    auto* context = static_cast<AddButtonContext*>(lv_event_get_user_data(event));
    if (!context || !context->owner || !context->owner->scheduler_) return;
    ProgramsPage& page = *context->owner;
    const int16_t slot = page.scheduler_->createProgram(context->valveIndex);
    if (slot < 0)
    {
        if (page.displayManager_) page.displayManager_->showMessage("Maximal 16 Programme");
        return;
    }
    page.rebuildProgramList();
    page.openEditor(static_cast<uint8_t>(slot));
}

void ProgramsPage::hourMinusEvent(lv_event_t* e) { auto* p = static_cast<ProgramsPage*>(lv_event_get_user_data(e)); if (p) { p->draftHour_ = p->draftHour_ == 0 ? 23 : p->draftHour_ - 1; p->refreshEditorValues(); } }
void ProgramsPage::hourPlusEvent(lv_event_t* e) { auto* p = static_cast<ProgramsPage*>(lv_event_get_user_data(e)); if (p) { p->draftHour_ = p->draftHour_ >= 23 ? 0 : p->draftHour_ + 1; p->refreshEditorValues(); } }
void ProgramsPage::minuteMinusEvent(lv_event_t* e) { auto* p = static_cast<ProgramsPage*>(lv_event_get_user_data(e)); if (p) { p->draftMinute_ = p->draftMinute_ == 0 ? 59 : p->draftMinute_ - 1; p->refreshEditorValues(); } }
void ProgramsPage::minutePlusEvent(lv_event_t* e) { auto* p = static_cast<ProgramsPage*>(lv_event_get_user_data(e)); if (p) { p->draftMinute_ = p->draftMinute_ >= 59 ? 0 : p->draftMinute_ + 1; p->refreshEditorValues(); } }
void ProgramsPage::durationMinusEvent(lv_event_t* e) { auto* p = static_cast<ProgramsPage*>(lv_event_get_user_data(e)); if (p && p->draftDurationMinutes_ > Scheduler::MIN_DURATION_MINUTES) { --p->draftDurationMinutes_; p->refreshEditorValues(); } }
void ProgramsPage::durationPlusEvent(lv_event_t* e) { auto* p = static_cast<ProgramsPage*>(lv_event_get_user_data(e)); if (p && p->draftDurationMinutes_ < Scheduler::MAX_DURATION_MINUTES) { ++p->draftDurationMinutes_; p->refreshEditorValues(); } }

void ProgramsPage::weekdayEvent(lv_event_t* event)
{
    auto* c = static_cast<WeekdayButtonContext*>(lv_event_get_user_data(event));
    if (!c || !c->owner || c->weekdayIndex >= WEEKDAY_COUNT) return;
    c->owner->draftWeekdays_ ^= static_cast<uint8_t>(1U << c->weekdayIndex);
    c->owner->refreshWeekdayButtons();
}

void ProgramsPage::editorEnabledEvent(lv_event_t* event)
{
    auto* page = static_cast<ProgramsPage*>(lv_event_get_user_data(event));
    if (!page) return;
    lv_obj_t* sw = static_cast<lv_obj_t*>(lv_event_get_target(event));
    page->draftEnabled_ = lv_obj_has_state(sw, LV_STATE_CHECKED);
}

void ProgramsPage::editorSaveEvent(lv_event_t* event)
{
    auto* page = static_cast<ProgramsPage*>(lv_event_get_user_data(event));
    if (!page) return;
    const bool saved = page->saveEditor();
    if (page->displayManager_) page->displayManager_->showMessage(saved ? "Programm gespeichert" : "Speichern fehlgeschlagen");
    if (saved) page->closeEditor();
}

void ProgramsPage::editorCancelEvent(lv_event_t* event)
{
    auto* page = static_cast<ProgramsPage*>(lv_event_get_user_data(event));
    if (page) page->closeEditor();
}

void ProgramsPage::editorDeleteEvent(lv_event_t* event)
{
    auto* page = static_cast<ProgramsPage*>(lv_event_get_user_data(event));
    if (!page) return;
    const bool deleted = page->deleteEditedProgram();
    if (page->displayManager_)
        page->displayManager_->showMessage(deleted ? "Programm geloescht" : "Loeschen fehlgeschlagen");
    if (deleted)
    {
        page->closeEditor();
        page->rebuildProgramList();
    }
}

void ProgramsPage::editorStartNowEvent(lv_event_t* event)
{
    if (lv_event_get_code(event) != LV_EVENT_CLICKED)
    {
        return;
    }

    auto* page = static_cast<ProgramsPage*>(lv_event_get_user_data(event));
    if (page == nullptr || page->scheduler_ == nullptr || page->runtimeManager_ == nullptr)
    {
        return;
    }

    if (!page->saveEditor())
    {
        if (page->displayManager_ != nullptr)
        {
            page->displayManager_->showMessage("Speichern fehlgeschlagen");
        }
        return;
    }

    const bool started = page->runtimeManager_->startProgram(page->editedProgramIndex_);
    if (page->displayManager_ != nullptr)
    {
        page->displayManager_->showMessage(
            started ? "Bewasserung gestartet" : "Start nicht moeglich");
    }

    if (started)
    {
        page->closeEditor();
        page->refresh();
    }
}
