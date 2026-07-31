#include "runtime/RuntimeManager.h"
#include "scheduler/Scheduler.h"
#include "hardware/ValveManager.h"
#include "time/TimeManager.h"
#include "weather/WeatherManager.h"

void RuntimeManager::begin(Scheduler& s, ValveManager& v, TimeManager& t){scheduler_=&s;valveManager_=&v;timeManager_=&t;clearState();Serial.println("RuntimeManager initialisiert");}
void RuntimeManager::setWeatherManager(WeatherManager& weatherManager){weatherManager_=&weatherManager;}
void RuntimeManager::update(){
    checkAutomaticStart();
    if(!isRunning()) return;
    const uint32_t elapsed=(millis()-startedAtMs_)/1000UL;
    if(elapsed<durationSeconds_ || valveManager_->channel(valveIndex_).pulseActive) return;
    if(valveManager_->pulse(valveIndex_)){Serial.printf("Programmlauf beendet, Ventil %u wird geschlossen\n",valveIndex_+1);clearState();}
}
void RuntimeManager::checkAutomaticStart(){
    if(isRunning()||!timeManager_||!timeManager_->isValid()) return;
    if(weatherManager_ && weatherManager_->automaticPauseActive()) return;
    struct tm t={}; if(!timeManager_->getLocalTime(t)) return;
    const int32_t dayKey=(t.tm_year+1900)*1000+t.tm_yday; const int16_t minute=t.tm_hour*60+t.tm_min;
    if(dayKey==lastCheckedDayKey_ && minute==lastCheckedMinute_) return;
    lastCheckedDayKey_=dayKey; lastCheckedMinute_=minute;
    const uint8_t wd=timeManager_->weekdayMondayZero();
    for(uint8_t i=0;i<Scheduler::MAX_PROGRAMS;++i){
        if(!scheduler_->isProgramUsed(i)) continue; const auto&p=scheduler_->program(i);
        if(!p.enabled || !(p.weekdays&(1U<<wd)) || p.startHour!=t.tm_hour || p.startMinute!=t.tm_min) continue;
        if(lastStartedProgramId_==p.id && lastStartedDayKey_==dayKey && lastStartedMinute_==minute) continue;
        if(startProgram(i,true)){lastStartedProgramId_=p.id;lastStartedDayKey_=dayKey;lastStartedMinute_=minute;} break;
    }
}
bool RuntimeManager::startProgram(uint8_t i,bool automatic){
    if(!scheduler_||!valveManager_||isRunning()||!scheduler_->isProgramUsed(i)||!allValvesIdleAndClosed()) return false;
    const auto&p=scheduler_->program(i); if(p.valveIndex>=Scheduler::VALVE_COUNT||p.durationSeconds==0||!valveManager_->pulse(p.valveIndex)) return false;
    runningProgramIndex_=i;startedAtMs_=millis();durationSeconds_=p.durationSeconds;valveIndex_=p.valveIndex;automaticRun_=automatic;
    Serial.printf("Programm %lu %s gestartet: Ventil %u, %lu Sekunden\n",(unsigned long)p.id,automatic?"automatisch":"manuell",p.valveIndex+1,(unsigned long)p.durationSeconds);return true;
}
bool RuntimeManager::stop(){if(!isRunning()||valveManager_->channel(valveIndex_).pulseActive||!valveManager_->pulse(valveIndex_))return false;Serial.printf("Programmlauf abgebrochen, Ventil %u wird geschlossen\n",valveIndex_+1);clearState();return true;}
bool RuntimeManager::isRunning()const{return scheduler_&&valveManager_&&runningProgramIndex_>=0&&runningProgramIndex_<Scheduler::MAX_PROGRAMS;}
bool RuntimeManager::isProgramRunning(uint8_t i)const{return isRunning()&&runningProgramIndex_==i;}
bool RuntimeManager::isAutomaticRun()const{return isRunning()&&automaticRun_;}
int16_t RuntimeManager::runningProgramIndex()const{return isRunning()?runningProgramIndex_:-1;}
uint32_t RuntimeManager::remainingSeconds()const{if(!isRunning())return 0;uint32_t e=(millis()-startedAtMs_)/1000UL;return e>=durationSeconds_?0:durationSeconds_-e;}
uint32_t RuntimeManager::durationSeconds()const{return isRunning()?durationSeconds_:0;}
uint8_t RuntimeManager::runningValveIndex()const{return isRunning()?valveIndex_:0;}
int16_t RuntimeManager::nextProgramIndex()const{
    if(!scheduler_||!timeManager_||!timeManager_->isValid())return -1; struct tm t={};timeManager_->getLocalTime(t);int now=t.tm_hour*60+t.tm_min;uint8_t today=timeManager_->weekdayMondayZero();int best=99999;int16_t idx=-1;
    for(uint8_t i=0;i<Scheduler::MAX_PROGRAMS;++i){if(!scheduler_->isProgramUsed(i))continue;const auto&p=scheduler_->program(i);if(!p.enabled)continue;for(int d=0;d<8;++d){uint8_t wd=(today+d)%7;if(!(p.weekdays&(1U<<wd)))continue;int delta=d*1440+p.startHour*60+p.startMinute-now;if(delta<0)continue;if(delta<best){best=delta;idx=i;}break;}}return idx;
}
void RuntimeManager::clearState(){runningProgramIndex_=-1;startedAtMs_=0;durationSeconds_=0;valveIndex_=0;automaticRun_=false;}
bool RuntimeManager::allValvesIdleAndClosed()const{for(uint8_t v=0;v<Scheduler::VALVE_COUNT;++v){const auto&c=valveManager_->channel(v);if(c.pulseActive||c.assumedOpen)return false;}return true;}
