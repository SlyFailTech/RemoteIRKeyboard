#include <avr/delay.h>
#include "TrinketHidCombo.h"

boolean PRINT_KEYS = false; //Нужно ли печатать нажимаемые клавиши. Ставить true для отладки

long LAST_KEY = 0;
long VOLUME_UP = 3877175040; //Добавить звук
long VOLUME_DOWN = 2907897600; //Убавить звук
long NEXT_TRACK = 2774204160; //Следующий трек
long PREV_TRACK = 4144561920; //Предыдущий трек
long PLAY_PAUSE = 3810328320; //Старт/пауза
long ARR_LEFT = 3910598400; //Стрелка влево
long ARR_RIGHT = 4061003520; //Стрелка вправо
long KEY_SPACE = 3860463360; // Пробел
long HOT_SPOTIFY = 3125149440; //Горячие клавиши запуска Спотифай (У меня это Ctrl + Shift + M)
long CLOSE_WINDOWS = 3091726080; //Закрыть окно (Alt + F4)

volatile uint8_t m = 0, complete = 0, tcnt = 0, startflag = 0;
uint32_t irdata = 0, keydata = 0 ;

void setup() {
  DDRB |= (1 << DDB1);
  PORTB |= 1 << PB2;
  GIMSK |= 1 << INT0;
  MCUCR |= 1 << ISC00;
  GTCCR |= 1 << PSR0; TCCR0A = 0;
  TCCR0B = (1 << CS02) | (1 << CS00);
  TIMSK = 1 << TOIE0;
  TrinketHidCombo.begin();
}
void loop() {
  if (complete) {

    if (keydata <= 1) {
      keydata = LAST_KEY;
    }

    //Нужные комбинации клавишь смотри в файле: https://github.com/adafruit/Adafruit-Trinket-USB/blob/master/TrinketHidCombo/TrinketHidCombo.h
    if (keydata == VOLUME_UP) TrinketHidCombo.pressMultimediaKey(MMKEY_VOL_UP); // Добавить звук
    if (keydata == VOLUME_DOWN) TrinketHidCombo.pressMultimediaKey(MMKEY_VOL_DOWN); // Убавить звук
    if (keydata == NEXT_TRACK) TrinketHidCombo.pressMultimediaKey(MMKEY_SCAN_NEXT_TRACK); // Следующий трек
    if (keydata == PREV_TRACK) TrinketHidCombo.pressMultimediaKey(MMKEY_SCAN_PREV_TRACK); // Предыдущий трек
    if (keydata == PLAY_PAUSE) TrinketHidCombo.pressMultimediaKey(MMKEY_PLAYPAUSE); // Стоп/пауза
    if (keydata == ARR_LEFT) {
      TrinketHidCombo.pressKey(0, KEYCODE_ARROW_LEFT);  // Влево
      TrinketHidCombo.pressKey(0, 0);
    }
    if (keydata == ARR_RIGHT) {
      TrinketHidCombo.pressKey(0, KEYCODE_ARROW_RIGHT);  // Вправо
      TrinketHidCombo.pressKey(0, 0);
    }
    if (keydata == KEY_SPACE) {
      TrinketHidCombo.pressKey(0, KEYCODE_SPACE);  // Пробел
      TrinketHidCombo.pressKey(0, 0);
    }
    if (keydata == HOT_SPOTIFY) {
      TrinketHidCombo.pressKey(KEYCODE_MOD_LEFT_CONTROL | KEYCODE_MOD_LEFT_SHIFT, KEYCODE_M);  // Набрать Ctrl + Shift + M
      TrinketHidCombo.pressKey(0, 0);
    }
    if (keydata == CLOSE_WINDOWS) {
      TrinketHidCombo.pressKey(KEYCODE_LEFT_ALT, KEYCODE_F4);  // Закрыть окно
      TrinketHidCombo.pressKey(0, 0);
    }

    if (keydata == CLOSE_WINDOWS) { //Не повторять кнопку закрыть
      keydata = 0;
    }
    LAST_KEY = keydata;

    if (PRINT_KEYS) {
      TrinketHidCombo.println(keydata);
    }

    PORTB |= (1 << 1);
    PORTB &= ~(1 << 1);

    complete = 0;
  }
  _delay_ms(1);
  TrinketHidCombo.poll();

}

ISR (INT0_vect) {
  if (PINB & 1 << 2) { // Если лог1
    TCNT0 = 0;
  }
  else {
    tcnt = TCNT0; // если лог 0
    if (startflag) {
      if (30 > tcnt  && tcnt > 2) {
        if (tcnt > 15 && m < 32) {
          irdata |= ((uint32_t)1 << m);
        }
        m++;
      }
    }
    else  startflag = 1;
  }
}
ISR (TIMER0_OVF_vect) {

  if (m) {
    complete = 1; m = 0; startflag = 0; keydata = irdata; irdata = 0; // если индекс не 0, то создать флаг конца
  }
}

void ms_delay(uint16_t x) { // функция задержки с USB поллом
  for (uint16_t m = 0; m < (x / 10); m++) {
    _delay_ms(10);
    TrinketHidCombo.poll();
  }
}
