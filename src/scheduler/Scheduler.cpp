#include "Scheduler.h"
#include "profiles/GardenProfiles.h"

namespace
{
    constexpr const char* NVS_NAMESPACE = "scheduler";
    constexpr const char* KEY_VERSION = "version";
    constexpr const char* KEY_PROGRAMS = "programs";
    constexpr const char* KEY_NEXT_ID = "next_id";
}

void Scheduler::begin()
{
    preferences_.begin(NVS_NAMESPACE, false);

    if (!load())
    {
        Serial.println("Keine gueltigen Programmdaten gefunden");
        restoreDefaults();
        if (!save())
        {
            Serial.println("Fehler beim Speichern der Standardprogramme");
        }
    }

    Serial.println("Scheduler initialisiert");
}


void Scheduler::update()
{
    // Automatische Zeitsteuerung folgt mit TimeManager.
}

Scheduler::IrrigationProgram& Scheduler::program(uint8_t index)
{
    static IrrigationProgram invalidProgram;
    return validProgramIndex(index) ? programs_[index] : invalidProgram;
}

const Scheduler::IrrigationProgram& Scheduler::program(uint8_t index) const
{
    static IrrigationProgram invalidProgram;
    return validProgramIndex(index) ? programs_[index] : invalidProgram;
}

uint8_t Scheduler::programCount() const
{
    return MAX_PROGRAMS;
}

uint8_t Scheduler::usedProgramCount() const
{
    uint8_t count = 0;
    for (uint8_t i = 0; i < MAX_PROGRAMS; ++i)
    {
        if (programs_[i].used)
        {
            ++count;
        }
    }
    return count;
}

uint8_t Scheduler::usedProgramCountForValve(uint8_t valveIndex) const
{
    if (!validValveIndex(valveIndex))
    {
        return 0;
    }

    uint8_t count = 0;
    for (uint8_t i = 0; i < MAX_PROGRAMS; ++i)
    {
        if (programs_[i].used && programs_[i].valveIndex == valveIndex)
        {
            ++count;
        }
    }
    return count;
}

bool Scheduler::isProgramUsed(uint8_t index) const
{
    return validProgramIndex(index) && programs_[index].used;
}

int16_t Scheduler::createProgram(uint8_t valveIndex)
{
    if (!validValveIndex(valveIndex))
    {
        return -1;
    }

    const int16_t freeSlot = findFreeSlot();
    if (freeSlot < 0)
    {
        return -1;
    }

    IrrigationProgram created;
    created.id = allocateProgramId();
    created.used = true;
    created.valveIndex = valveIndex;
    created.profileId = 0;
    created.startHour = 6;
    created.startMinute = 0;
    created.durationSeconds = 15UL * 60UL;
    created.weekdays = 0x7F;

    programs_[freeSlot] = created;

    if (!save())
    {
        programs_[freeSlot] = IrrigationProgram{};
        return -1;
    }

    return freeSlot;
}

bool Scheduler::deleteProgram(uint8_t index)
{
    if (!validUsedProgramIndex(index))
    {
        return false;
    }

    programs_[index] = IrrigationProgram{};
    return save();
}

uint32_t Scheduler::programId(uint8_t index) const
{
    return validUsedProgramIndex(index) ? programs_[index].id : INVALID_PROGRAM_ID;
}

int16_t Scheduler::findProgramIndexById(uint32_t id) const
{
    if (id == INVALID_PROGRAM_ID)
    {
        return -1;
    }

    for (uint8_t i = 0; i < MAX_PROGRAMS; ++i)
    {
        if (programs_[i].used && programs_[i].id == id)
        {
            return static_cast<int16_t>(i);
        }
    }
    return -1;
}

bool Scheduler::setProgramEnabled(uint8_t index, bool enabled)
{
    if (!validUsedProgramIndex(index))
    {
        return false;
    }
    if (programs_[index].enabled == enabled)
    {
        return true;
    }
    programs_[index].enabled = enabled;
    return save();
}

bool Scheduler::setValve(uint8_t programIndex, uint8_t valveIndex)
{
    if (!validUsedProgramIndex(programIndex) || !validValveIndex(valveIndex))
    {
        return false;
    }
    if (programs_[programIndex].valveIndex == valveIndex)
    {
        return true;
    }
    programs_[programIndex].valveIndex = valveIndex;
    return save();
}

bool Scheduler::setProfile(uint8_t programIndex, uint8_t profileId)
{
    if (!validUsedProgramIndex(programIndex) || !GardenProfiles::isValid(profileId)) return false;
    if (programs_[programIndex].profileId == profileId) return true;
    programs_[programIndex].profileId = profileId;
    return save();
}

bool Scheduler::setStartTime(uint8_t programIndex, uint8_t hour, uint8_t minute)
{
    if (!validUsedProgramIndex(programIndex) || hour > 23 || minute > 59)
    {
        return false;
    }
    IrrigationProgram& selected = programs_[programIndex];
    if (selected.startHour == hour && selected.startMinute == minute)
    {
        return true;
    }
    selected.startHour = hour;
    selected.startMinute = minute;
    return save();
}

bool Scheduler::setDurationMinutes(uint8_t programIndex, uint16_t minutes)
{
    if (!validUsedProgramIndex(programIndex) ||
        minutes < MIN_DURATION_MINUTES || minutes > MAX_DURATION_MINUTES)
    {
        return false;
    }
    const uint32_t seconds = static_cast<uint32_t>(minutes) * 60UL;
    if (programs_[programIndex].durationSeconds == seconds)
    {
        return true;
    }
    programs_[programIndex].durationSeconds = seconds;
    return save();
}

uint16_t Scheduler::durationMinutes(uint8_t programIndex) const
{
    if (!validUsedProgramIndex(programIndex))
    {
        return 0;
    }
    return static_cast<uint16_t>(programs_[programIndex].durationSeconds / 60UL);
}

bool Scheduler::setWeekday(uint8_t programIndex, Weekday weekday, bool enabled)
{
    if (!validUsedProgramIndex(programIndex))
    {
        return false;
    }
    const uint8_t day = static_cast<uint8_t>(weekday);
    if (day > 6)
    {
        return false;
    }
    const uint8_t mask = static_cast<uint8_t>(1U << day);
    const bool current = (programs_[programIndex].weekdays & mask) != 0;
    if (current == enabled)
    {
        return true;
    }
    if (enabled)
    {
        programs_[programIndex].weekdays |= mask;
    }
    else
    {
        programs_[programIndex].weekdays &= static_cast<uint8_t>(~mask);
    }
    return save();
}

bool Scheduler::isWeekdayEnabled(uint8_t programIndex, Weekday weekday) const
{
    if (!validUsedProgramIndex(programIndex))
    {
        return false;
    }
    const uint8_t day = static_cast<uint8_t>(weekday);
    if (day > 6)
    {
        return false;
    }
    return (programs_[programIndex].weekdays & static_cast<uint8_t>(1U << day)) != 0;
}

bool Scheduler::save()
{
    const size_t expected = sizeof(programs_);
    const size_t written = preferences_.putBytes(KEY_PROGRAMS, programs_, expected);
    if (written != expected)
    {
        Serial.printf("Scheduler speichern fehlgeschlagen: %u von %u Bytes\n",
                      static_cast<unsigned>(written), static_cast<unsigned>(expected));
        return false;
    }
    if (preferences_.putUInt(KEY_NEXT_ID, nextProgramId_) == 0 ||
        preferences_.putUInt(KEY_VERSION, STORAGE_VERSION) == 0)
    {
        Serial.println("Scheduler-Metadaten konnten nicht gespeichert werden");
        return false;
    }
    return true;
}

bool Scheduler::load()
{
    const uint32_t version = preferences_.getUInt(KEY_VERSION, 0);
    if (version == STORAGE_VERSION)
    {
        return loadCurrentVersion();
    }
    if (version == LEGACY_STORAGE_VERSION_3)
    {
        Serial.println("Migriere Scheduler-Speicherversion 3 auf 4");
        return migrateFromVersion3();
    }
    if (version == LEGACY_STORAGE_VERSION_2)
    {
        Serial.println("Migriere Scheduler-Speicherversion 2 auf 3");
        return migrateFromVersion2();
    }
    if (version == LEGACY_STORAGE_VERSION_1)
    {
        Serial.println("Migriere Scheduler-Speicherversion 1 auf 3");
        return migrateFromVersion1();
    }
    return false;
}

bool Scheduler::loadCurrentVersion()
{
    const size_t expected = sizeof(programs_);
    if (preferences_.getBytesLength(KEY_PROGRAMS) != expected)
    {
        return false;
    }
    if (preferences_.getBytes(KEY_PROGRAMS, programs_, expected) != expected)
    {
        return false;
    }
    nextProgramId_ = preferences_.getUInt(KEY_NEXT_ID, 1);
    resetRuntimeState();
    return true;
}

bool Scheduler::migrateFromVersion1()
{
    LegacyIrrigationProgramV1 legacy[MAX_PROGRAMS];
    const size_t expected = sizeof(legacy);
    if (preferences_.getBytesLength(KEY_PROGRAMS) != expected ||
        preferences_.getBytes(KEY_PROGRAMS, legacy, expected) != expected)
    {
        return false;
    }

    nextProgramId_ = 1;
    for (uint8_t i = 0; i < MAX_PROGRAMS; ++i)
    {
        programs_[i] = IrrigationProgram{};
        const bool used = i < 2 || legacy[i].enabled || legacy[i].weekdays != 0;
        if (!used)
        {
            continue;
        }
        programs_[i].id = allocateProgramId();
        programs_[i].used = true;
        programs_[i].enabled = legacy[i].enabled;
        programs_[i].valveIndex = validValveIndex(legacy[i].valveIndex) ? legacy[i].valveIndex : 0;
        programs_[i].startHour = legacy[i].startHour <= 23 ? legacy[i].startHour : 6;
        programs_[i].startMinute = legacy[i].startMinute <= 59 ? legacy[i].startMinute : 0;
        programs_[i].durationSeconds = legacy[i].durationSeconds;
        programs_[i].weekdays = legacy[i].weekdays & 0x7F;
    }
    resetRuntimeState();
    return save();
}

bool Scheduler::migrateFromVersion3()
{
    LegacyIrrigationProgramV3 legacy[MAX_PROGRAMS];
    const size_t expected=sizeof(legacy);
    if(preferences_.getBytesLength(KEY_PROGRAMS)!=expected||preferences_.getBytes(KEY_PROGRAMS,legacy,expected)!=expected)return false;
    nextProgramId_=preferences_.getUInt(KEY_NEXT_ID,1);
    for(uint8_t i=0;i<MAX_PROGRAMS;++i){
        programs_[i]=IrrigationProgram{};
        programs_[i].id=legacy[i].id; programs_[i].used=legacy[i].used; programs_[i].enabled=legacy[i].enabled;
        programs_[i].valveIndex=validValveIndex(legacy[i].valveIndex)?legacy[i].valveIndex:0;
        programs_[i].profileId=0; programs_[i].startHour=legacy[i].startHour<=23?legacy[i].startHour:6;
        programs_[i].startMinute=legacy[i].startMinute<=59?legacy[i].startMinute:0;
        programs_[i].durationSeconds=legacy[i].durationSeconds; programs_[i].weekdays=legacy[i].weekdays&0x7F;
        if(programs_[i].used&&programs_[i].id>=nextProgramId_)nextProgramId_=programs_[i].id+1;
    }
    resetRuntimeState(); return save();
}

bool Scheduler::migrateFromVersion2()
{
    LegacyIrrigationProgramV2 legacy[MAX_PROGRAMS];
    const size_t expected = sizeof(legacy);
    if (preferences_.getBytesLength(KEY_PROGRAMS) != expected ||
        preferences_.getBytes(KEY_PROGRAMS, legacy, expected) != expected)
    {
        return false;
    }

    nextProgramId_ = preferences_.getUInt(KEY_NEXT_ID, 1);
    for (uint8_t i = 0; i < MAX_PROGRAMS; ++i)
    {
        programs_[i] = IrrigationProgram{};
        const bool used = i < 2 || legacy[i].enabled || legacy[i].weekdays != 0;
        if (!used)
        {
            continue;
        }
        programs_[i].id = legacy[i].id != INVALID_PROGRAM_ID ? legacy[i].id : allocateProgramId();
        programs_[i].used = true;
        programs_[i].enabled = legacy[i].enabled;
        programs_[i].valveIndex = validValveIndex(legacy[i].valveIndex) ? legacy[i].valveIndex : 0;
        programs_[i].startHour = legacy[i].startHour <= 23 ? legacy[i].startHour : 6;
        programs_[i].startMinute = legacy[i].startMinute <= 59 ? legacy[i].startMinute : 0;
        programs_[i].durationSeconds = legacy[i].durationSeconds;
        programs_[i].weekdays = legacy[i].weekdays & 0x7F;
        if (programs_[i].id >= nextProgramId_)
        {
            nextProgramId_ = programs_[i].id + 1;
        }
    }
    resetRuntimeState();
    return save();
}

void Scheduler::restoreDefaults()
{
    nextProgramId_ = 1;
    for (uint8_t i = 0; i < MAX_PROGRAMS; ++i)
    {
        programs_[i] = IrrigationProgram{};
    }

    programs_[0].id = allocateProgramId();
    programs_[0].used = true;
    programs_[0].enabled = true;
    programs_[0].valveIndex = 0;
    programs_[0].profileId = 1;
    programs_[0].startHour = 6;
    programs_[0].startMinute = 30;
    programs_[0].durationSeconds = 15UL * 60UL;
    programs_[0].weekdays = 0x1F;

    programs_[1].id = allocateProgramId();
    programs_[1].used = true;
    programs_[1].enabled = false;
    programs_[1].valveIndex = 1;
    programs_[1].profileId = 2;
    programs_[1].startHour = 20;
    programs_[1].startMinute = 0;
    programs_[1].durationSeconds = 10UL * 60UL;
    programs_[1].weekdays = 0x60;
}

void Scheduler::resetRuntimeState()
{
    for (uint8_t i = 0; i < MAX_PROGRAMS; ++i)
    {
        programs_[i].running = false;
        programs_[i].startedAtMs = 0;
    }
}

uint32_t Scheduler::allocateProgramId()
{
    if (nextProgramId_ == INVALID_PROGRAM_ID)
    {
        nextProgramId_ = 1;
    }
    return nextProgramId_++;
}

int16_t Scheduler::findFreeSlot() const
{
    for (uint8_t i = 0; i < MAX_PROGRAMS; ++i)
    {
        if (!programs_[i].used)
        {
            return static_cast<int16_t>(i);
        }
    }
    return -1;
}

bool Scheduler::validProgramIndex(uint8_t index) const
{
    return index < MAX_PROGRAMS;
}

bool Scheduler::validUsedProgramIndex(uint8_t index) const
{
    return validProgramIndex(index) && programs_[index].used;
}

bool Scheduler::validValveIndex(uint8_t index) const
{
    return index < VALVE_COUNT;
}
