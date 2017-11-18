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
#define DEFAULTLAMPS 8



//let g:neomake_make_maker = { 'exe': 'make',  'args': [], 'errorformat': '%f:%l:%c: %m' }

volatile int CURRENTLAMPS=1;

typedef void (*hookF)(void*);

struct timeT {
    int hour0;
    int minute0;
    int hourF;
    int minuteF;
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
const timeT lampTimers[DEFAULTLAMPS] = {{.hour0=0, .minute0=5, .hourF=0, .minuteF=8}};
lampT lamps[DEFAULTLAMPS];

void SerialPrintF(char *fmt, ... ){
        char buf[128]; // resulting string limited to 128 chars
        va_list args;
        va_start (args, fmt );
        vsnprintf(buf, 128, fmt, args);
        va_end (args);
        Serial.print(buf);
}

void onTimerOn(void *data) {
  lampT *lamp = (lampT *)data;
}

void onTimerOff(void *data) {
  lampT *lamp = (lampT *)data;
}

lampT * findLampByAlarmId(AlarmID_t alarmId) {
  lampT lamp;
  for (int i=0;i < CURRENTLAMPS;i++) {
    lamp = lamps[i];
    if (alarmId == lamp.timer.alarmId0 || alarmId == lamp.timer.alarmIdF)
      return &lamp;
  }
  return NULL;
}

void onAlarmHook() {
  lampT *lamp;
  AlarmID_t currentAlarm = Alarm.getTriggeredAlarmId();
  lamp = findLampByAlarmId(currentAlarm);
  SerialPrintF("Got alarm: %d\n", lamp);
  if (lamp) {
    Serial.println("Triggered alarm with id ...");
    if (lamp->state) {
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
  SerialPrintF("Adding alarm starting %d %d\n", time.hour0, time.minute0);
  timer-> alarmId0 = Alarm.alarmRepeat(time.hour0, time.minute0, 0, onAlarmHook);
  SerialPrintF("Adding alarm ending %d %d\n", time.hourF, time.minuteF);
  timer-> alarmIdF = Alarm.alarmRepeat(time.hourF, time.minuteF, 0, onAlarmHook);
}

void cancelTimer(timerT *timer) {
  Alarm.free(timer->alarmId0);
  Alarm.free(timer->alarmIdF);
  timer->alarmId0 = dtINVALID_ALARM_ID;
  timer->alarmIdF = dtINVALID_ALARM_ID;
}

int toogleLamp(lampT *lamp) {
  int mode;
  lamp->state = !lamp->state;
  if (lamp->state)
    mode = LOW;
  else
    mode = HIGH;
  digitalWrite(lamp->pin, mode);
  
  return lamp->state;
}

void initLamps() {
  lampT *lamp;
  int pin;
  for (int i=0;i<CURRENTLAMPS;i++) {
    lamp = &lamps[i];
    pin = lampPins[i];
    lamp->pin = pin;
    pinMode(lamp->pin, OUTPUT);
    digitalWrite(lamp->pin, LOW);
    lamp->state = false;
    setTimer(&(lamp->timer), lampTimers[i]);
    SerialPrintF("Setting up lamp %d with state %d [%d:%d - %d:%d]\n", lamp->pin, 
                 lamp->state, lamp->timer.timerTime.hour0, lamp->timer.timerTime.minute0,
                 lamp->timer.timerTime.hourF, lamp->timer.timerTime.minuteF);
  }
}

void initPins() {
    for (int i=0;i<NLAMPS;i++) 
      pinMode(i, OUTPUT);
}

void setup() {
  Serial.begin(9600);
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


