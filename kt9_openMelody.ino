//======================================
// kitchenTimer 2022(C)by ke2a 06 ラウレンシユス　アラン
//======================================

//追加　KT03 ***
#include <Wire.h>
#include <ST7032.h>

//------[ define定義 ]------
#define SW_ON  LOW    //LOW   0
#define SW_OFF HIGH   //HIGH  1
#define DELAY 10      //メインループのウェイト時間(n x ms)

//From kt16
#define LEDC_CHANNEL_0 0
#define LEDC_TIMER_13_BIT 13
#define LEDC_BASE_FREQ 5000


//------[ global(広域)変数定義 ]------
const int LED_RED = 16;     //IO16 赤色LED
const int LED_GREEN = 15;   //IO15 緑色LED
const int LED_BLUE = 4;     //IO4  青赤色LED

//From kt16
const int NOTE_NONE = NOTE_MAX;

//追加
const int buzPin = 23;
const int GPIO_SW1 = 17;         //IO17 スイッチ1
const int GPIO_SW2 = 0;          //IO0  スイッチ2
const int GPIO_SW3 = 35;         //IO35  スイッチ3
//スイッチの前回値を保存する変数を宣言(初期値はOFF)
int sw1old = SW_OFF; //スイッチ一つずつ旧データ
int sw2old = SW_OFF;
int sw3old = SW_OFF;


int gMode = 0;        // 表示モード
int gMin = 0;     //分タイマー
int gSec = 0;     //秒タイマー
int gSecOld = 0;  //旧秒タイマー
int gMinBack;     //分タイマー（前回の値）
int gSecBack;     //秒タイマー（前回の値）
ST7032 lcd;

//タイマー用変数の追加  kt6 #
hw_timer_t* timer1 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

//From kt16
int melody [] = {
  NOTE_NONE, NOTE_E, NOTE_C, NOTE_E, NOTE_C, NOTE_E, NOTE_C, NOTE_E, NOTE_C
};
int noteOctaves[] = {0, 4, 4, 4, 4, 4, 4, 4, 4};
int noteDurations[] = {8, 8, 8, 8, 8, 8, 8, 8, 4};

int melody2[] = {
  NOTE_C, NOTE_D, NOTE_E, NOTE_F, NOTE_G, NOTE_A, NOTE_B, NOTE_C
};
int noteOctaves2[] = {
  4, 4, 4, 4, 4, 4, 4, 5
};

int noteDurations2[] = {
  8, 8, 8, 8, 8, 8, 8, 8
};

void playMelody(int num) {
  switch (num) {
    case 1:
      if (digitalRead(GPIO_SW1) == HIGH) {
        for (int thisNote = 0 ; thisNote < 9 ; thisNote++) {
          ledcWriteNote(LEDC_CHANNEL_0, (note_t)melody[thisNote], noteOctaves[thisNote]);
          int pauseBetweenNotes = 1000 / noteDurations[thisNote] * 1.30;
          delay(pauseBetweenNotes);
          ledcWriteTone(LEDC_CHANNEL_0, 0);
        }
        break;
      case 2:
        if (digitalRead(GPIO_SW1) == HIGH) {
          for (int thisNote = 0 ; thisNote < 8 ; thisNote++) {
            ledcWriteNote(LEDC_CHANNEL_0, (note_t)melody2[thisNote], noteOctaves2[thisNote]);
            int pauseBetweenNotes = 1000 / noteDurations2[thisNote] * 1.30;
            delay(pauseBetweenNotes);
            ledcWriteTone(LEDC_CHANNEL_0, 0);
          }
          break;
        default:
          delay(2000);
          break;
        }
      }
  }
}

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);

  if (gMode == 2) {
    if (gSec == 0 ) {
      if (gMin > 0 ) {
        gMin--;
        gSec = 60;
      }
    }
    if (gSec > 0) {
      gSec--;
    }
  }
  portEXIT_CRITICAL_ISR(&timerMux);
}
/**
   セットアップ処理
*/
void setup() {
  // 入力ポート設定
  pinMode(GPIO_SW1, INPUT);
  pinMode(GPIO_SW2, INPUT);
  pinMode(GPIO_SW3, INPUT);
  // 出力ポート設定
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  //ここが追加する
  pinMode(buzPin, OUTPUT);
  //LEDをオンする
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  delay(1000);  //1sec wait
  //LEDをオフする
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, LOW);
  // シリアルポートを115200bps[ビット/秒]で初期化
  Serial.begin(115200);

  // LCD初期化
  delay(40);            //n[ms]ウェイト
  Wire.begin(SDA, SCL); //SDA = 21, SCL = 22
  lcd.begin(8, 2);      // 液晶ディスプレイの表示サイズを指定
  lcd.setContrast(30);  // コントラスト設定

  lcd.setCursor(0, 0);    //1行目に移動
  lcd.printf("Laurent "); //表示
  lcd.setCursor(0, 1);    //2行目に移動
  lcd.printf("Alang "); //表示

  //3秒にストップ
  delay(3000);
  gMode = 1;


  //kt6 #
  timer1 = timerBegin(0, 80, true);
  timerAlarmWrite(timer1, 1000000, true);
  timerAttachInterrupt(timer1, &onTimer, true);
  timerAlarmEnable(timer1);

  //From kt16
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(buzPin, LEDC_CHANNEL_0);
  //kt09
  playMelody(2);

}
void swOperation(void) {
  int sw1 = digitalRead(GPIO_SW1);
  int sw2 = digitalRead(GPIO_SW2);
  int sw3 = digitalRead(GPIO_SW3);

  if ((sw1 != sw1old || sw2 != sw2old) && (sw1 == SW_ON &&  sw2 == SW_ON)) {
    beep(300);
    gMin = 0;
    gSec = 0;
  }

  else if (sw1 != sw1old) {                // スイッチ1に変化があったか
    if (sw1 == SW_ON) {               // スイッチ1がONされたか
      digitalWrite(LED_RED, HIGH);
      beep(300);

      //kt05 の追加 *****

      if (gMin == 99) {
        gMin = 99;
      } else {
        gMin++;
      }
    } else {
      digitalWrite(LED_RED, LOW);
    }
  }
  else if (sw2 != sw2old) {
    if (sw2 == SW_ON) {
      digitalWrite(LED_BLUE, HIGH);
      beep(300);

      if (gSec == 59) {
        gMin += 01;
        gSec = 00;
      } else {
        gSec++;
      }
    } else {
      digitalWrite(LED_BLUE, LOW);
    }
  }
  else if (sw3 != sw3old) {
    if (sw3 == SW_ON) {
      digitalWrite(LED_GREEN, HIGH);
      beep(500);

      //      追加　KT04
      //        gMode++;
      //        if(gMode >= 4) {
      //          gMode = 1;
      //        }

      //##KT07
      if (gMode == 1) {
        Serial.println("タイマースタート");
        gMode = 2;

        gMinBack = gMin;
        gSecBack = gSec;
      } else if (gMode == 2) {
        Serial.println("タイマーストップ");
        gMode = 1;
      }

    } else {
      digitalWrite(LED_GREEN, LOW);
    }
  }
  // 変化検出用旧データを更新
  sw1old = sw1;
  sw2old = sw2;
  sw3old = sw3;
}

void beep(int time) {
  /*
    int i;
    for ( i = 0; i < time; i++) { // time[ms]の間発音する
    // H port
    digitalWrite(buzPin, HIGH);
    delayMicroseconds(500); // 500us待つ
    // L port
    digitalWrite(buzPin, LOW);
    delayMicroseconds(500); // 500us待つ
    }
  */
  ledcWriteTone(LEDC_CHANNEL_0, 1000);
  delay(time);
  ledcWriteTone(LEDC_CHANNEL_0, 0);
}

void lcdDisp(void) {
  char str1[ ] = "Kitchen ";
  char str2[ ] = "Timer   ";
  char str3[ ] = "STOP   ";
  char str4[ ] = "START  ";
  char str5[ ] = "TIME UP";
  char str[9] = {'\0'};

  switch (gMode) {
    case 0: //初期画面表示
      //------[ 表示（１行目）]------
      lcd.setCursor(0, 0);
      lcd.printf(str1);

      //------[ 表示（２行目）]------
      lcd.setCursor(0, 1);
      lcd.printf(str2);
      break;

    case 1: //時間設定画面表示

      //KT 04 ****
      lcd.setCursor(0, 0);
      lcd.printf(str3);

      lcd.setCursor(0, 1);
      lcd.printf("%.2d : %.2d", gMin, gSec);
      break;

    case 2:
      lcd.setCursor(0, 0);
      lcd.printf(str4);

      lcd.setCursor(0, 1);
      lcd.printf("%.2d : %.2d", gMin, gSec);
      break;

    case 3:
      lcd.setCursor(0, 0);
      lcd.printf(str5);

      lcd.setCursor(0, 1);
      lcd.printf("%.2d : %.2d", gMin, gSec);
      break;
    default:
      break;
  }
}

//## KT 07
void timeUpCheck() {
  if (gMode == 2) {
    if (gMin == 0 && gSec == 0) {
      Serial.println("タイマーストップ");

      gMode = 3;
      lcdDisp();
      delay(1000);
      //      for (int i = 0 ; i < 3 ; i++) {
      //        beep(500);
      //        delay(200);
      //      }
      //
      //From kt16
      playMelody(1);

      gMode = 1;
      //## KT 07
      gMin = gMinBack;
      gSec = gSecBack;
    }
  }
}

void loop() {
  static int loopCnt = 0;

  swOperation();
  if (++loopCnt > 20) {   //20 x DELAY[ms]毎に処理を行う
    loopCnt = 0;

    timeUpCheck();

    lcdDisp();
  }
  delay(DELAY); //n[ms]ウェイト

}
