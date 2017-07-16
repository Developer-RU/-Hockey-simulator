////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////               Name: "Hockey simulator"                       //////////////////////////////////////||||||||||||//////////////////////////////////////
//////////////////////////////////////////////////////               Developer: "Developer-RU"                       //////////////////////////////////////||||||||||||//////////////////////////////////////
//////////////////////////////////////////////////////               Email: p.masyukov@gmail.com                //////////////////////////////////////||||||||||||//////////////////////////////////////
//////////////////////////////////////////////////////               DateTime: 03.11.2016                       //////////////////////////////////////||||||||||||//////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <TimerOne.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
                                                                                            /////////
#define SIGNAL           13                     //  Пин "Датчиков темпиратуры"              /////////
                                                                                            /////////
#define START_PIN_ZONE    8                     //  Первый вывод датчиков зон               /////////
#define STOP_PIN_ZONE    12                     //  Последний вывод датчиков зон            /////////
                                                                                            /////////
#define BTN_UP           A0                     //  Кнопка меньше                           /////////
#define BTN_DOWN         A1                     //  Кнопка больше                           /////////
#define BTN_MENU         A2                     //  Кнопка выбор|подтвердить                /////////
#define BTN_START        A3                     //  Кнопка стоп                             /////////
                                                                                            /////////
#define RED_LED_START    22                     //  Первый вывод красных фонарей            /////////   
#define RED_LED_STOP     26                     //  Последний вывод красных фонарей         /////////   
                                                                                            /////////
#define GREEN_LED_START  28                     //  Первый вывод зеленых фонарей            /////////   
#define GREEN_LED_STOP   32                     //  Последний вывод зеленых фонарей         /////////  
                                                                                            /////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

#define LCD_HEIGHT        2                //  Высота дисплея в строках
#define LCD_WIDTH        16                //  Ширина дисплея в символах

LiquidCrystal lcd(7, 6, 2, 3, 4, 5);

int statusTraining = 0;                    //  Статус тренировки (0 - остановлена, если больше - в процессе)
int statusWaiting = 0;                     //  Статус задержки перед началом
int waiting = 5;                           //  Задержка перед началом в секундах

unsigned long showMenu = 0;                //  Хранит время показа меню
unsigned long currentTime = 0;             //  Хранит текущее время
unsigned long loopTime = 0;                //  Хранит заданное время

int trainingTime = 60;                     //  Продолжительность тренировки текущая
int durationTime = 1;                      //  Время сложности (длительности включения зоны)

unsigned long zoneStartTime = 0;           //  Время включения зоны
unsigned long zoneStopTime = 0;            //  Время выключения зоны игроком
unsigned long zoneBrekTime = 0;            //  Время выключения зоны по длительности


int statusZones[5] = {0, 0, 0, 0, 0};      //  Состояния зон

int currentZone = 0;                       //  Номер текущей зоны
int previouZone = 0;                       //  Номер предыдущей зоны

int counterAllZones = 0;                   //  Всего было включено зон
int counterStopZones = 0;                   //  Захвачено зон 
int counterBrekZones = 0;                   //  Пропущено зон

uint8_t currentMenuItem = 0, state = 0, saveSettingsState = 1;
uint8_t flagX = 0, flagZ = 0, flagY = 0, flagS = 0;

//////////////////////////////////////////////////////////////////////////
//  Очистка адреса епром        //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void eepromClear()
{
    for (int i = 0 ; i < EEPROM.length() ; i++) 
    {
        EEPROM.write(i, 0);
    }
}

//////////////////////////////////////////////////////////////////////////
//   Чтение настроек              ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// чтение
int EEPROM_int_read(int addr) {    
  byte raw[2];
  for(byte i = 0; i < 2; i++) raw[i] = EEPROM.read(addr+i);
  int &num = (int&)raw;
  return num;
}

//////////////////////////////////////////////////////////////////////////
//  Запись  настроек              ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// запись
void EEPROM_int_write(int addr, int num) {
  byte raw[2];
  (int&)raw = num;
  for(byte i = 0; i < 2; i++) EEPROM.write(addr+i, raw[i]);
}

//////////////////////////////////////////////////////////////////////////
//  Установка настроект по умолчанию       ///////////////////////////////
//////////////////////////////////////////////////////////////////////////
void defaultSettings()
{    
    eepromClear();  
    EEPROM.write(0, 1);              
    EEPROM.write(1, 60);
    EEPROM.write(5, 10);
}

//////////////////////////////////////////////////////////////////////////
//  Установка заголовка меню    //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void clearPrintTitle(String title)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(title);
    lcd.setCursor(0, 1);
}

//////////////////////////////////////////////////////////////////////////
//  Вывод главного меню         //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void displayMenu(int x) 
{
    switch (x)
    {
        case 1:
            clearPrintTitle("Training time");
            lcd.print ("> SET: ");
            if (trainingTime / 60 % 60 < 10) { lcd.print ("0"); }
            lcd.print ((trainingTime / 60) % 60);
            lcd.print (":");
            if (trainingTime % 60 < 10) { lcd.print ("0"); }
            lcd.print(trainingTime % 60);
            break;
            
        case 2:
            clearPrintTitle("Speed level");
            lcd.print ("> SET: ");
            lcd.print(durationTime);
            break;
    }
}

//////////////////////////////////////////////////////////////////////////
//  Выбранный элеменот меню     //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void selectMenu(int x, int y)
{
  
    switch(x)
    {      
        case 1: 
                if(y) {
                    if(trainingTime < 600) trainingTime += 10; 
                } else {
                    if(trainingTime >= 20) trainingTime -= 10;
                }
            
            clearPrintTitle("Training time");
            lcd.print ("> SET: "); 
            if (trainingTime / 60 % 60 < 10) { lcd.print ("0"); }
            lcd.print ((trainingTime / 60) % 60);
            lcd.print (":");
            if (trainingTime % 60 < 10) { lcd.print ("0"); }
            lcd.print(trainingTime % 60);            
            break;
            
        case 2: 
                if(y) {
                    if(durationTime < 10) durationTime++; 
                } else {
                    if(durationTime >= 2) durationTime--;
                }
            
            clearPrintTitle("Speed level");
            lcd.print ("> SET: ");
            lcd.print(durationTime);            
            break;
    }

    delay(50);
}

//////////////////////////////////////////////////////////////////////////
//  Обработчик меню и кнопок    //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void mainMenu()
{

    // Кнопка UP
    if(analogRead(BTN_UP) < 800 && flagX == 0) flagX = 1;

    if(analogRead(BTN_UP) > 1000 && flagX == 1)
    { 
        flagX = 0; 
        state = 3; 
        delay(10); 
    }

    // Кноака MENU
    if(analogRead(BTN_MENU) < 800 && flagZ == 0) flagZ = 1;

    if(analogRead(BTN_MENU) > 1000 && flagZ == 1) 
    { 
        flagZ = 0; 
        state = 1; 
        delay(10); 
    }

    // Кнопка DOWN
    if(analogRead(BTN_DOWN) < 800 && flagY == 0) flagY = 1;

    if(analogRead(BTN_DOWN) > 1000 && flagY == 1)
    { 
        flagY = 0; 
        state = 2; 
        delay(10); 
    }

    
    if (currentMenuItem > 2) 
    {
        currentMenuItem = 0;
        showMenu = 0;
        clearPrintTitle(" START TRAINING ");
        lcd.print ("  PRESS START  ");
    }
    
    if (state > 0) 
    {     
        tone(SIGNAL, 1000, 100);
                           
        lcd.setCursor(0, 0);
        
        if (state == 1) 
        {
            currentMenuItem++;
            displayMenu(currentMenuItem);
        } 
        else if (state == 2)
        {
            selectMenu(currentMenuItem, 0); // Left  -
        } 
        else if (state == 3)
        {
            selectMenu(currentMenuItem, 1); // Right +
        }

        loopTime = currentTime;
        state = 0;
    }
}




//////////////////////////////////////////////////////////////////////////
//  Обновление и вывод информации на дисплей        //////////////////////
//////////////////////////////////////////////////////////////////////////
void lcdUpdate(String textTitle, String textInfo)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(textTitle);
    lcd.setCursor(0, 1);
    lcd.print(textInfo);
}

//////////////////////////////////////////////////////////////////////////
//  Выбор случайной зоны        //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void startZone()
{
    previouZone = currentZone;
    statusZones[previouZone] = 0;
    
    currentZone = random(0, 5);
    currentZone = random(0, 5);
    currentZone = random(0, 5);
    
    while(currentZone == previouZone) 
    {
        currentZone = random(0, 5);
    }
}


//////////////////////////////////////////////////////////////////////////
//  Функция таймера обратного отсчета     ////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void Timer1_action()
{
    if(trainingTime) 
    {
        trainingTime--; 
    } else {
        Timer1.stop();
    }
}

//////////////////////////////////////////////////////////////////////////
//  Начальная настройка         //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void setup()
{
    Serial.begin(9600);
    Serial.println("Start settings");
    
    Timer1.initialize(); // 1 секунда по умолчанию - то, что надо
    
    // Назначение пина спикера
    pinMode(SIGNAL, OUTPUT); digitalWrite(SIGNAL, LOW);


    // Назначение пинов зеленых фонарей
    for(int i = GREEN_LED_START; i <= GREEN_LED_STOP; i++) { pinMode(i, OUTPUT); digitalWrite(i, LOW); }


    // Назначение пинов красных фонарей
    for(int i = RED_LED_START; i <= RED_LED_STOP; i++) { pinMode(i, OUTPUT); digitalWrite(i, LOW); }

    
    // Назначение пинов датчиков зон
    for(int i = START_PIN_ZONE; i <= STOP_PIN_ZONE; i++) pinMode(i, INPUT_PULLUP);


    // Назначение пинов кнопок
    pinMode(BTN_UP, INPUT_PULLUP); 
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_MENU, INPUT_PULLUP);
    pinMode(BTN_START, INPUT_PULLUP);
    
    // Инициализация дисплея
    lcd.begin(LCD_WIDTH, LCD_HEIGHT);  
    
    Timer1.attachInterrupt(Timer1_action);
    Timer1.stop();

    Serial.println("Stop settings");

    clearPrintTitle("LOADING!");
    lcd.print ("PLEASE WAIT");
    
    delay(3000);  

    //  Мелодия заставки

    tone(SIGNAL, 700, 300); delay(600);
    tone(SIGNAL, 700, 300); delay(600);
    tone(SIGNAL, 780, 150); delay(300);
    tone(SIGNAL, 700, 150); delay(300);
    tone(SIGNAL, 625, 450); delay(600);
    tone(SIGNAL, 590, 150); delay(300);
    tone(SIGNAL, 520, 150); delay(300);
    tone(SIGNAL, 460, 450); delay(600);
    tone(SIGNAL, 350, 450); delay(600);
    delay(400);
    
    for(int i = RED_LED_START; i <= RED_LED_STOP; i++) { digitalWrite(i, HIGH); }
    delay(100);
    for(int i = RED_LED_START; i <= RED_LED_STOP; i++) { digitalWrite(i, LOW); }

    tone(SIGNAL, 1200, 300);
    
    for(int i = GREEN_LED_START; i <= GREEN_LED_STOP; i++) { digitalWrite(i, HIGH); }
    delay(100);
    for(int i = GREEN_LED_START; i <= GREEN_LED_STOP; i++) { digitalWrite(i, LOW); }

    clearPrintTitle("Developer-RU");
    lcd.print ("+7(951)795-65-05");
    
    tone(SIGNAL, 350, 450); delay(600);
    tone(SIGNAL, 460, 450); delay(600);
    tone(SIGNAL, 520, 150); delay(300);
    tone(SIGNAL, 590, 150); delay(300);
    tone(SIGNAL, 625, 450); delay(600);
    tone(SIGNAL, 590, 150); delay(300);
    tone(SIGNAL, 520, 150); delay(300);
    tone(SIGNAL, 700, 1350); delay(1600);

    for(int i = RED_LED_START; i <= RED_LED_STOP; i++) { digitalWrite(i, HIGH); }
    delay(100);
    for(int i = RED_LED_START; i <= RED_LED_STOP; i++) { digitalWrite(i, LOW); }

    tone(SIGNAL, 1200, 300);
    
    for(int i = GREEN_LED_START; i <= GREEN_LED_STOP; i++) { digitalWrite(i, HIGH); }
    delay(100);
    for(int i = GREEN_LED_START; i <= GREEN_LED_STOP; i++) { digitalWrite(i, LOW); }

    clearPrintTitle("     Hockey!   ");
    lcd.print ("    SIMULATOR   ");
    
    tone(SIGNAL, 700, 300); delay(600);
    tone(SIGNAL, 700, 300); delay(600);
    tone(SIGNAL, 780, 150); delay(300);
    tone(SIGNAL, 700, 150); delay(300);
    tone(SIGNAL, 625, 450); delay(600);
    tone(SIGNAL, 590, 150); delay(300);
    tone(SIGNAL, 520, 150); delay(300);
    tone(SIGNAL, 460, 450); delay(600);
    tone(SIGNAL, 350, 450); delay(600);
    delay(400);

    
    for(int i = RED_LED_START; i <= RED_LED_STOP; i++) { digitalWrite(i, HIGH); }
    delay(100);
    for(int i = RED_LED_START; i <= RED_LED_STOP; i++) { digitalWrite(i, LOW); }

    tone(SIGNAL, 1200, 300);
    
    for(int i = GREEN_LED_START; i <= GREEN_LED_STOP; i++) { digitalWrite(i, HIGH); }
    delay(100);
    for(int i = GREEN_LED_START; i <= GREEN_LED_STOP; i++) { digitalWrite(i, LOW); }


    tone(SIGNAL, 350, 450); delay(600);
    tone(SIGNAL, 625, 450); delay(600);
    tone(SIGNAL, 590, 150); delay(300);
    tone(SIGNAL, 520, 150); delay(300);
    tone(SIGNAL, 700, 450); delay(600);
    tone(SIGNAL, 590, 150); delay(300);
    tone(SIGNAL, 520, 150); delay(300);
    tone(SIGNAL, 460, 1350); delay(5000);
    
    //eepromClear(); 
    
    // Если первый запуск
    if(EEPROM.read(0) == 0)
    {
        defaultSettings();  //  Записываем настройки что по умолчанию
    } else {
        //  Получаем предыдущие настройки
        trainingTime = EEPROM.read(1);
        durationTime = EEPROM.read(5);
    }

    clearPrintTitle("    TRAINING   ");
    lcd.print ("  PUSH TO START ");
    
    //  Получаем текущее время
    loopTime = currentTime = millis(); 

    //  Обнуляем показ меню
    showMenu = 0;
}

//////////////////////////////////////////////////////////////////////////
//  Чтение датчика холла     /////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
bool readSensorHoll()
{
    return (bool)digitalRead(8 + currentZone);
}

//////////////////////////////////////////////////////////////////////////
//  Очистка результатов с экрана     /////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
clearResults()
{
    clearPrintTitle("    TRAINING   ");
    lcd.print ("  PUSH TO START ");
    loopTime = millis();
}

//////////////////////////////////////////////////////////////////////////
//  Основной цикл программы     //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void loop() 
{
    while(1)
    {
        //  Получаем текущее время
        currentTime = millis();       
       
        if(currentTime <= (loopTime + showMenu))
        {
            mainMenu(); 
            break;
        } 
        else 
        {               
            if(saveSettingsState == 0)
            {
                currentMenuItem = 0;

                EEPROM.write(1, trainingTime);   // Сохраняем установленное время тренировки
                EEPROM.write(5, durationTime);  // Сохраняем установленную сложность тренировки

                for(int u = 0; u < 5; u++)
                {
                    for(int i = RED_LED_START; i <= RED_LED_STOP; i++) { digitalWrite(i, HIGH); }
                    delay(100);
                    for(int i = RED_LED_START; i <= RED_LED_STOP; i++) { digitalWrite(i, LOW); }

                    tone(SIGNAL, 1200, 300);
                    
                    for(int i = GREEN_LED_START; i <= GREEN_LED_STOP; i++) { digitalWrite(i, HIGH); }
                    delay(100);
                    for(int i = GREEN_LED_START; i <= GREEN_LED_STOP; i++) { digitalWrite(i, LOW); }

                    delay(100);
                }
                
                saveSettingsState = 1;

                clearPrintTitle("    SETTINGS    ");
                lcd.print ("     SAVED     ");
                delay(500);
                loopTime = 0;

                clearPrintTitle("    TRAINING   ");
                lcd.print ("  PUSH TO START ");
            }
                
            //  Если тренировка не запущена
            if(!statusTraining)
            {                       
                //  Обновление счетчика времени дисплея
                loopTime = currentTime;
                
                // Обработчик кнопки menu
                if(analogRead(BTN_MENU) < 800  && flagZ == 0) flagZ = 1;
            
                if(analogRead(BTN_MENU) > 1000 && flagZ == 1) 
                {                     
                    flagZ = 0; 
                    showMenu = 15000; 
                    state = 1; 
                    saveSettingsState = 0;
                    delay(10);
                    break;
                }
                
                // Обработчик кнопки старт тренировка
                if(analogRead(BTN_START) < 800  && flagS == 0) flagS = 1;
            
                if(analogRead(BTN_START) > 1000 && flagS == 1) 
                { 
                    //  Запуск тренировки
                    flagS = 0; 
                    statusTraining++;
                    trainingTime = trainingTime + 1;
                    loopTime = 0;
                    waiting = 5;
                    break;
                }
            
            }
            
            //  Если тренировка запущена
            if(statusTraining)
            {
                if(!statusWaiting)
                {                    
                    //  Ожидание запуска в миллисекундах (5000)
                    if(currentTime >= (loopTime + 1000))
                    {  
                        loopTime = millis();
                        
                        if(waiting > 1)
                        {
                            // Звуковой сигнал - короткий
                            tone(SIGNAL, 500, 300);
                        }
                        else
                        {
                            // Звуковой сигнал - длинный
                            tone(SIGNAL, 500, 750);
                            
                            statusWaiting++;
    
                            //  Запуск отсчета (таймера)
                            Timer1.start();
                        }
    
                        waiting--;
                        
                        // Выводим обратный отсчет
                        lcdUpdate("    Waiting", "       " + String(waiting));
                    }
                }
                else  //  Если время тренировки не прошло
                {
                    //  Если текущая зона включена читаем состояние холла
                    if(statusZones[currentZone] == 1)
                    {
                        //  Если шайба глушит зону
                        if(!readSensorHoll()) 
                        {
                            //  Засчитываем балл
                            counterStopZones++;

                            //  Выключаем красный фонарь подсветки зоны
                            digitalWrite( RED_LED_START + currentZone, LOW);
                            
                            //  Включаем зеленый фонарь подсветки зоны
                            digitalWrite( GREEN_LED_START + currentZone, HIGH);
                            
                            // Издаем сигнал о зачислении
                            tone(SIGNAL, 1000, 50); delay(5);
                            tone(SIGNAL, 1000, 50); delay(5);
                            tone(SIGNAL, 1000, 50); delay(5);
    
                            //  Обнуляем текущую зону
                            statusZones[currentZone] = 0;

                            //  Выключаем зеленый фонарь подсветки зоны
                            digitalWrite( GREEN_LED_START + currentZone, LOW);
                            
                            //  Обнуляем счетчик времени вывода на экран
                            loopTime = 0;
                        }
                    }
                    
                    if(currentTime >= (loopTime + 500))
                    {  
                        //  Записываем новую метку времени
                        loopTime = millis();                  
                        
                        //  Выводим обратный отсчет времени тренировки, подсчет текущих результатов (забито/пропущено)

                        lcd.clear();
                        
                        lcd.setCursor(5, 0);
                        
                        if (trainingTime / 60 % 60 < 10) { lcd.print ("0"); }
                        lcd.print ((trainingTime / 60) % 60);
                        lcd.print (":");
                        if (trainingTime % 60 < 10) { lcd.print ("0"); }
                        lcd.print(trainingTime % 60);
                         
                        lcd.setCursor(2, 1);

                        if(counterStopZones <= 9) 
                        {
                             lcd.print("00"); lcd.print(counterStopZones);
                        }
                        else if(counterStopZones >= 10 && counterStopZones <= 99)
                        {
                             lcd.print("0"); lcd.print(counterStopZones);
                        } else {
                             lcd.print(counterStopZones);
                        }

                        lcd.print("|");

                        if(counterAllZones <= 9) 
                        {
                             lcd.print("00"); lcd.print(counterAllZones);
                        }
                        else if(counterAllZones >= 10 && counterAllZones <= 99)
                        {
                             lcd.print("0"); lcd.print(counterAllZones);
                        } else {
                             lcd.print(counterAllZones);
                        }

                        lcd.print("|");

                        if(counterBrekZones <= 9) 
                        {
                             lcd.print("00"); lcd.print(counterBrekZones);
                        }
                        else if(counterBrekZones >= 10 && counterBrekZones <= 99)
                        {
                             lcd.print("0"); lcd.print(counterBrekZones);
                        } else {
                             lcd.print(counterBrekZones);
                        }

                        lcd.setCursor(15, 1);
                        lcd.print("^");
                        lcd.setCursor(15, 0);
                        lcd.print(String(1 + currentZone));
                    }
                
                    //  Если текущая (сперва нулевая) зона не запущена
                    if(!statusZones[currentZone]) 
                    {
                        //  Выбираем рандомно зону и включаем и записываем время включения
                        startZone();
    
                        statusZones[currentZone] = 1;


                        //  Включаем красный фонарь подсветки зоны
                        digitalWrite( RED_LED_START + currentZone, HIGH);
                        
                        
                        //  Помечаем время запуска зоны
                        zoneStartTime = millis();
                        
                        //  Плюсуем общий счетчик зон
                        counterAllZones++; 
    
                        //  Издаем сигнал о старте зоны
                        //tone(SIGNAL, 1000, 50);
                        //delay(10);
                        //tone(SIGNAL, 1000, 50);
                    }
    
                    //  Смотрим состояние включеной зоны и время, не пора ли сменить если вышло время (тоесть выключить до следующего круга программы - а там и включится новая)
                    if( currentTime >= (zoneStartTime + (durationTime * 1000)) && statusZones[currentZone] == 1)
                    {                        
                        //  Обнуляем текущую зону
                        statusZones[currentZone] = 0;

                        //  Выключаем красный фонарь подсветки зоны
                        digitalWrite( RED_LED_START + currentZone, LOW);

    
                        //  Плюсуем упущенную
                        counterBrekZones++;
    
                        //  Издаем сигнал о пропуске
                        //tone(SIGNAL, 100, 50);
                        //delay(10);
                        //tone(SIGNAL, 100, 50);

                        //  Обнуляем счетчик времени
                        loopTime = 0;

                        break;
                    }
                }
            }
        
            if(!trainingTime && !waiting)
            {
                //  Остановка отсчета (таймера)
                Timer1.stop();
                
                //  Выводим результаты (если надо дополняем ожиданием до нажатия кнопки)
                lcdUpdate("Stop zones: " + String(counterStopZones), "Break zones: " + String(counterBrekZones));
                
                //  Обнуляем счетчики и выключаем все зоны

                for(int i = RED_LED_START; i <= RED_LED_STOP; i++) { digitalWrite(i, LOW); }
                
                for(int i = GREEN_LED_START; i <= GREEN_LED_STOP; i++) { digitalWrite(i, LOW); }

                statusTraining = 0;
                statusWaiting = 0;
                
                zoneStartTime = 0;
                zoneStopTime = 0;
                zoneBrekTime = 0;

                counterStopZones = 0;
                counterAllZones = 0;
                counterBrekZones = 0;
                                
                statusZones[currentZone] = 0;
                
                delay(15000);
                
                clearResults();
            }

        }
    }
}

