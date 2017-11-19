#include <stdarg.h>
#include <TimeLib.h>
#include <Time.h>
#include <TimeAlarms.h>

#define LAMP0 2
#define LAMP1 3
#define LAMP2 4
#define LAMP3 5
#define LAMP4 6
#define LAMP5 7
#define LAMP6 8
#define LAMP7 9
#define NLAMPS 8

typedef void (*hookF)(void*);

struct timeT {
    int hour0;
    int minute0;
    int second0;
    int hourF;
    int minuteF;
    int secondF;
};

struct timerT {
  AlarmID_t alarmId0;
  AlarmID_t alarmIdF;
  timeT timerTime;
  hookF onHook;
  hookF offHook;
};

struct lampT {
 int pin;
 bool state;
 timerT timer;
};

const int lampPins[NLAMPS] = {LAMP0, LAMP1, LAMP2, LAMP3, LAMP4, LAMP5, LAMP6, LAMP7};
const timeT lampTimers[NLAMPS] = {
  {.hour0=0, .minute0=0, .second0=30, .hourF=0, .minuteF=1, .secondF=30},
  {.hour0=0, .minute0=2, .second0=30, .hourF=0, .minuteF=3, .secondF=30},
  {.hour0=0, .minute0=3, .second0=30, .hourF=0, .minuteF=4, .secondF=30},
  {.hour0=0, .minute0=1, .second0=30, .hourF=0, .minuteF=3, .secondF=30},
  {.hour0=0, .minute0=2, .second0=30, .hourF=0, .minuteF=4, .secondF=30},
  {.hour0=0, .minute0=1, .second0=30, .hourF=0, .minuteF=5, .secondF=30},
  {.hour0=0, .minute0=5, .second0=30, .hourF=0, .minuteF=12, .secondF=30},
  {.hour0=0, .minute0=7, .second0=30, .hourF=0, .minuteF=20, .secondF=30}
};
lampT lamps[NLAMPS];

void SerialPrintF(const char *fmt, ... ){
        char buf[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(buf, 128, fmt, args);
        va_end (args);
        Serial.print(buf);
}

int toogleLamp(lampT *lamp) {
  int mode;
  lamp->state = !lamp->state;
  if (!lamp->state)
    mode = LOW;
  else
    mode = HIGH;
  digitalWrite(lamp->pin, mode);
  
  return lamp->state;
}

void onTimerOn(void *data) {
  lampT *lamp = (lampT *)data;
  SerialPrintF("onTimerOn: lamp %d[%p], current state: %d\n", lamp->pin, lamp, lamp->state);
  toogleLamp(lamp);
}

void onTimerOff(void *data) {
  lampT *lamp = (lampT *)data;
  SerialPrintF("onTimerOff: lamp %d[%p], current state: %d\n", lamp->pin, lamp, lamp->state);
  toogleLamp(lamp);
}

lampT * findLampByAlarmId(AlarmID_t alarmId) {
  lampT lamp;
  SerialPrintF("Searching lamp by alarm in %p\n", lamps);
  for (int i=0;i<NLAMPS;i++) {
    lamp = lamps[i];
    if (alarmId == lamp.timer.alarmId0 || alarmId == lamp.timer.alarmIdF)
      return &lamps[i];
  }
  return NULL;
}

void onAlarmHook() {
  lampT *lamp;
  AlarmID_t currentAlarm = Alarm.getTriggeredAlarmId();
  lamp = findLampByAlarmId(currentAlarm);
  if (lamp) {
    SerialPrintF("Got alarm %d for lamp %d[%p] in state %d\n", currentAlarm, lamp->pin, lamp, lamp->state);
    if (!lamp->state) {
      lamp->timer.onHook((void *)lamp);
    } else {
      lamp->timer.offHook((void *)lamp);
    }
  } else {
    Serial.println("WARNING. Triggered alarm was not registered");
  }
}

void setTimer(timerT *timer, timeT time) {
  timer->timerTime = time;
  timer->onHook = onTimerOn;
  timer->offHook = onTimerOff;
  SerialPrintF("Adding alarm starting %d:%d:%d\n", time.hour0, time.minute0, time.second0);
  timer-> alarmId0 = Alarm.alarmRepeat(time.hour0, time.minute0, time.second0, onAlarmHook);
  SerialPrintF("Adding alarm ending %d:%d:%d\n", time.hourF, time.minuteF, time.secondF);
  timer-> alarmIdF = Alarm.alarmRepeat(time.hourF, time.minuteF, time.secondF, onAlarmHook);
}

void cancelTimer(timerT *timer) {
  Alarm.free(timer->alarmId0);
  Alarm.free(timer->alarmIdF);
  timer->alarmId0 = dtINVALID_ALARM_ID;
  timer->alarmIdF = dtINVALID_ALARM_ID;
}

void initLamps() {
  lampT *lamp;
  int pin;
  for (int i=0;i<NLAMPS;i++) {
    lamp = &lamps[i];
    pin = lampPins[i];
    lamp->pin = pin;
    pinMode(lamp->pin, OUTPUT);
    digitalWrite(lamp->pin, LOW);
    lamp->state = false;
    setTimer(&(lamp->timer), lampTimers[i]);
    SerialPrintF("Set up lamp %d[%p] with state %d\n", lamp->pin, lamp, lamp->state);
  }
}

void initPins() {
    for (int i=0;i<NLAMPS;i++) 
      pinMode(i, OUTPUT);
}

void setup() {
  Serial.begin(9600);
  while (!Serial) ;
  Serial.println("Setting time ...");
  setTime(0, 0, 0, 18, 11, 2017);
  Serial.println("Initialiazing pins ...");
  initPins();
  Serial.println("Initialiazing default lamps ...");
  initLamps();
  Serial.println("Ready ...");
}

void loop() {
  Alarm.delay(1000);
}


