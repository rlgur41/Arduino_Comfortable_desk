#include <LiquidCrystal.h>  // LCD 
#include <MsTimer2.h>       // time interrupt
#include <DHT11.h>          // 온습도
#include <util/delay.h>     // 인터럽트 delay

//===========================매크로 함수================================================================================
#define GET_DISCOMFORT_INDEX(temp, humi) (((0,72)*(temp + humi) + (40.6)) / 100)   // 불쾌지수 구하는 공식 
#define GET_ULTRASOUND_DISTANCE(duration) ((float)((340*duration)/10000 )/2)       // 초음파 센서로 거리를 구하는 공식
//=====================================================================================================================

//===========================아두이노 핀================================================================================
#define INT_BTN   2         // 버튼 ( 인터럽트 핀 )
#define INA       3         // 팬모터 A라인
#define INB       4         // 팬모터 B라인
#define DHT_11    5         // 온습도 센서

#define TRIG      8         // 초음파센서 TRIG핀
#define ECHO      9         // 초음파센서 ECHO핀
//=====================================================================================================================


//===========================센서 객체 정의=============================================================================
LiquidCrystal lcd(6, 7, 10, 11, 12, 13);
DHT11 obj_dht11(DHT_11);
//=====================================================================================================================

void setup() {

 Serial.begin(9600);   // Serial port 정의                           

 //==========I/O 정의==============
 pinMode(ECHO,    INPUT);
 pinMode(INT_BTN, INPUT);
 pinMode(TRIG,    OUTPUT); 
 pinMode(INA,     OUTPUT);
 pinMode(INB,     OUTPUT);
 //================================
 
 lcd.begin(16, 2);     // LCD 창 크기 정의
 
 MsTimer2::set(5000, get_us_100_data);        // 타임 인터럽트 정의, 5초에 한번씩 get_us_100_data() ( 초음파 센서 함수) 호출
 MsTimer2::start();                           // 타임 인터럽트 시작
 
 attachInterrupt(INT0, fan_control, FALLING); // 아두이노 2번핀(버튼)의 전기가 0으로 떨어지면 fan_control() ( 팬모터 함수 ) 를 호출하는 인터럽트 
 
}

volatile boolean _stop = false;               // 팬모터를 제어하는 변수

void loop() {

  float temp, humi;             // 온습도센서에서 받아올 온도와 습도를 저장하는 변수
  float discomfort_index;       // 온도와 습도를 불쾌지수 공식에 넣어 나온 값을 저장하는 변수


  if( (obj_dht11.read(humi, temp) ) == 0) {               // 온습도 센서 값 읽음 ( 제대로 읽으면 )
    discomfort_index = GET_DISCOMFORT_INDEX(temp, humi);  // 불쾌지수 값 얻음

    //=================================lcd 출력============================
    lcd.setCursor(0, 0);                
    lcd.print("Discomfort index");
    lcd.setCursor(0, 1);
    lcd.print(">>> ");
    lcd.print(String(discomfort_index));
    lcd.print(" % ");
    //=====================================================================
  }
  else{             // 읽기 에러
    //=================================lcd 출력============================
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor perhaps");
    lcd.setCursor(0, 1);
    lcd.print("crushed");
    //=====================================================================
  }

  if(discomfort_index >= 50.0) {            // 불쾌지수가 50 이상이면
    
    if(!_stop) {                            // 사용자가 팬모터 기능을 막지 않았다면(버튼)
      digitalWrite(INA, HIGH);              // 팬 회전
      digitalWrite(INB, LOW);
    }
    else {                                  // 사용자가 팬모터 기능을 막았다면 (버튼)
      digitalWrite(INA, LOW);               // 팬 정지
      digitalWrite(INB, LOW);    
    }
  }
  else {
     digitalWrite(INA, LOW);                // 불쾌지수가 50 미만이면 팬은 항상 정지
     digitalWrite(INB, LOW);
  }
  
  delay(1000);                              // delay time
}

void get_us_100_data()      // 초음파센서의 값을 얻어옴 
{ 
  float duration, distance;                     // trig 값과 echo 값을 저장 할 변수들
  digitalWrite(TRIG, HIGH);                     // 초음파 발생
  delay(10);                                    // 초음파가 돌아 올 동안 잠시 대기..
  digitalWrite(ECHO, LOW);                      // 초음파 받음
  duration = pulseIn(ECHO, HIGH);               // 초음파가 도달한 시간 구함
  distance = GET_ULTRASOUND_DISTANCE(duration); // 초음파의 거리 구함
  
  Serial.print(distance);                       // Serial 통신을 통해 초음파센서에서 얻은 거리정보를 PC로 전송함
}

void fan_control()                        //인터럽트 버튼이 눌리면 수행되는 코드
{
  _delay_ms(100);                         //바운싱 방지

  if(digitalRead(INT_BTN) == HIGH)        //바운싱 방지
     return;
  
  _stop = !_stop;                         //토글
}

