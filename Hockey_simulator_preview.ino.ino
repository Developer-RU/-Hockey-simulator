////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////               Name: "Hockey simulator"                       //////////////////////////////////////||||||||||||//////////////////////////////////////
//////////////////////////////////////////////////////               Developer: "Developer-RU"                       //////////////////////////////////////||||||||||||//////////////////////////////////////
//////////////////////////////////////////////////////               Email: p.masyukov@gmail.com                //////////////////////////////////////||||||||||||//////////////////////////////////////
//////////////////////////////////////////////////////               DateTime: 03.11.2016                       //////////////////////////////////////||||||||||||//////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <TimerOne.h>
#include <Shift595.h>
#define SIGNAL           13                     //  Пин "Датчиков темпиратуры" 

#define BTN_UP           A2                     //  Кнопка меньше            
#define BTN_DOWN         A1                     //  Кнопка больше                    
#define BTN_MENU         A3                     //  Кнопка меню       
#define BTN_START        A4                     //  Кнопка старт                    

#define latchPin         11                     //  Пин подключен к ST_CP входу 74HC595
#define clockPin         12                     //  Пин подключен к SH_CP входу 74HC595
#define dataPin          10                     //  Пин подключен к DS входу 74HC595
#define numOfRegisters    1                     // number of shift registers present

#define DATAZONE          2                     //  Пин на снятие показаний зон

Shift595 Shifter(dataPin, latchPin, clockPin, numOfRegisters);

LiquidCrystal lcd(8, 7, 6, 5, 4, 3);

int statusTraining = 0;                    //  Статус тренировки (0 - остановлена, если больше - в процессе)
int statusWaiting = 0;                     //  Статус задержки перед началом
int waiting = 5;                           //  Задержка перед началом в секундах

unsigned long showMenu = 0;                //  Хранит время показа меню
unsigned long loopTime = 0;                //  Хранит заданное время
//unsigned long currentTime = 0;                //  Хранит заданное время
unsigned long zoneStartTime = 0;           //  Время включения зоны

int trainingTime = 60;                     //  Продолжительность тренировки текущая
int levelSpeed = 5;                             //  Время сложности (длительности включения зоны)
int selectTime = 60;                       //  Выбранная продолжительность тренировки

int statusZones[] = {0,0,0,0,0,0,0,0};                    //  Состояния зон


int currentZone = 0;                       //  Номер текущей зоны
int previouZone = 0;                       //  Номер предыдущей зоны

int counterAllZones = 0;                   //  Всего было включено зон
int counterStopZones = 0;                  //  Захвачено зон 
int counterBrekZones = 0;                  //  Пропущено зон

int currentMenuItem = 0, state = 0, saveSettingsState = 1;
int flagX = 0, flagZ = 0, flagY = 0, flagS = 0;



void eepromClear()
{
    for (int i = 0 ; i < EEPROM.length() ; i++) EEPROM.write(i, 0);
}

int EEPROM_int_read(int addr) 
{    
    byte raw[2];
    for(byte i = 0; i < 2; i++) raw[i] = EEPROM.read(addr+i);
    int &num = (int&)raw;
  
    return num;
}

void EEPROM_int_write(int addr, int num) 
{
    byte raw[2];
    (int&)raw = num;
    for(byte i = 0; i < 2; i++) EEPROM.write(addr+i, raw[i]);
}

void defaultSettings()
{    
    eepromClear();  
    
    EEPROM.write(0, 1);              
    EEPROM_int_write(2, 60);
    EEPROM_int_write(4, 5);
}

void clearPrintTitle(String title)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(title);
    lcd.setCursor(0, 1);
}

void clearResults()
{
    clearPrintTitle("    TRAINING   ");
    lcd.print ("  PUSH TO START ");
    loopTime = millis();
}

void displayMenu(int x) 
{
    switch (x)
    {
        case 1:
            clearPrintTitle("Training time");
            lcd.print ("> SET: ");
            if (selectTime / 60 % 60 < 10) { lcd.print ("0"); }
            lcd.print ((selectTime / 60) % 60);
            lcd.print (":");
            if (selectTime % 60 < 10) { lcd.print ("0"); }
            lcd.print(selectTime % 60);
            break;
        case 2:
            clearPrintTitle("Speed level");
            lcd.print ("> SET: ");
            lcd.print(map(levelSpeed, 10, 1, 1, 10));
            break;
    }
}

void selectMenu(int x, int y)
{
  
    switch(x)
    {      
        case 1: 
                if(y) {
                    if(selectTime < 600) selectTime += 10; 
                } else {
                    if(selectTime >= 20) selectTime -= 10;
                }
            
            clearPrintTitle("Training time");
            lcd.print ("> SET: "); 
            if (selectTime / 60 % 60 < 10) { lcd.print ("0"); }
            lcd.print ((selectTime / 60) % 60);
            lcd.print (":");
            if (selectTime % 60 < 10) { lcd.print ("0"); }
            lcd.print(selectTime % 60);            
            break;
            
        case 2: 
                if(!y) {
                    if(levelSpeed < 10) levelSpeed++; 
                } else {
                    if(levelSpeed >= 2) levelSpeed--;
                }
            
            clearPrintTitle("Speed level");
            lcd.print ("> SET: ");
            lcd.print(map(levelSpeed, 10, 1, 1, 10));            
            break;
    }

    delay(50);
}

void mainMenu()
{
    // Кнопка UP
    if(analogRead(BTN_UP) < 800 && flagX == 0) flagX = 1;

    if(analogRead(BTN_UP) > 1000 && flagX == 1) { flagX = 0; state = 3; delay(10); }

    // Кноака MENU
    if(analogRead(BTN_MENU) < 800 && flagZ == 0) flagZ = 1;
    
    if(analogRead(BTN_MENU) > 1000 && flagZ == 1) { flagZ = 0; state = 1; delay(10); }

    // Кнопка DOWN
    if(analogRead(BTN_DOWN) < 800 && flagY == 0) flagY = 1;

    if(analogRead(BTN_DOWN) > 1000 && flagY == 1) { flagY = 0; state = 2; delay(10); }

    
    if (currentMenuItem > 2) 
    {
        currentMenuItem = 0; showMenu = 0;
        clearPrintTitle(" START TRAINING ");
        lcd.print ("  PRESS START  ");
    }
    
    if (state > 0) 
    {     
        tone(SIGNAL, 2500, 50);
                           
        lcd.setCursor(0, 0);
        
        if (state == 1) {
            currentMenuItem++; displayMenu(currentMenuItem);
        } else if (state == 2) {
            selectMenu(currentMenuItem, 0); // Left  -
        } else if (state == 3) {
            selectMenu(currentMenuItem, 1); // Right +
        }

        loopTime = millis(); state = 0;
    }
}

void lcdUpdate(String textTitle, String textInfo)
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(textTitle);
    lcd.setCursor(0, 1);
    lcd.print(textInfo);
}

void startZone()
{
    previouZone = currentZone;
    statusZones[previouZone] = 0;
    
    long randNumber = random(5);
    randNumber = random(5);
    
    while(currentZone == previouZone)
    {
        randNumber = random(5);
        currentZone = 0 + randNumber;
    }

    Serial.print("new currentZone: ");
    Serial.println(currentZone);
}

void Timer1_action()
{
    if(trainingTime) { trainingTime--; } else { Timer1.stop(); }
}

bool readSensorHoll()
{
    bool res =  digitalRead(DATAZONE);
    
    if(res)
        return 1;
    else 
        return 0;
}


void setup()
{    
    Serial.begin(9600);    
    pinMode(SIGNAL, OUTPUT); digitalWrite(SIGNAL, LOW);                                                   // Назначение пина спикера
    
    pinMode(BTN_UP, INPUT_PULLUP);                                                                        // Назначение пинов кнопок  BTN_UP
    pinMode(BTN_DOWN, INPUT_PULLUP);                                                                      // Назначение пинов кнопок  BTN_DOWN
    pinMode(BTN_MENU, INPUT_PULLUP);                                                                      // Назначение пинов кнопок  BTN_MENU
    pinMode(BTN_START, INPUT_PULLUP);                                                                     // Назначение пинов кнопок  BTN_START
    
    pinMode(DATAZONE, INPUT_PULLUP);                                                                      // Назначение пина чтения зон
    
    for(int i = 0; i < 8; i++) Shifter.setRegisterPin(i, HIGH);                                            //  Обнуляем счетчики и выключаем все зоны
    
    delay(1000);
    
    lcd.begin(16, 2);  // Инициализация дисплея

    clearPrintTitle("LOADING!");
    lcd.print ("PLEASE WAIT");
    delay(2500);
    
    clearPrintTitle("Developer-RU");
    lcd.print ("+7(951)795-65-05");
    delay(4500);
    
    clearPrintTitle("     Hockey!   ");
    lcd.print ("    SIMULATOR   ");
    delay(5000);
        
    if(EEPROM.read(0) != 1) {                 // Если первый запуск
        defaultSettings();                        //  Записываем настройки что по умолчанию
    } else {
        trainingTime = EEPROM_int_read(2);        //  Получаем предыдущие настройки времени тренировки
        levelSpeed = EEPROM_int_read(4);        //  Получаем предыдущие настройки уровня сложности
    }

    selectTime = trainingTime;

    for(int i = 0; i < 6; i++) statusZones[i] = 0;
    
    clearPrintTitle("    TRAINING   "); 
    lcd.print ("  PUSH TO START ");
    showMenu = 0;

    Timer1.initialize();
    Timer1.attachInterrupt(Timer1_action);
    Timer1.stop();
}

void loop() 
{
    while(1)
    {       
        if(millis() - loopTime < showMenu) 
        {
          mainMenu(); 
          break;
        } 
        else
        {               
            if(saveSettingsState == 0)
            {
                currentMenuItem = 0;
                trainingTime = selectTime;

                EEPROM_int_write(2, trainingTime);   // Сохраняем установленное время тренировки
                EEPROM_int_write(4, levelSpeed);  // Сохраняем установленную сложность тренировки

                clearPrintTitle("    SETTINGS    ");
                lcd.print ("     SAVED     ");
                
                for(int i = 0; i < 8; i++) Shifter.setRegisterPin(i, LOW);     
                for(int u = 0; u < 5; u++) { tone(SIGNAL, 1200, 30); delay(100); }
                for(int i = 0; i < 8; i++) Shifter.setRegisterPin(i, HIGH);  
                
                delay(300);
                
                for(int i = 0; i < 8; i++) Shifter.setRegisterPin(i, LOW);     
                for(int u = 0; u < 5; u++) { tone(SIGNAL, 1200, 30); delay(100); }
                for(int i = 0; i < 8; i++) Shifter.setRegisterPin(i, HIGH);  

                delay(300);
                
                for(int i = 0; i < 8; i++) Shifter.setRegisterPin(i, LOW);     
                for(int u = 0; u < 5; u++) { tone(SIGNAL, 1200, 30); delay(100); }
                for(int i = 0; i < 8; i++) Shifter.setRegisterPin(i, HIGH);  
                                 
                saveSettingsState = 1;
                loopTime = 0;

                clearPrintTitle("    TRAINING   ");
                lcd.print ("   PRESS START  ");
            }
                
            //  Если тренировка не запущена
            if(!statusTraining)
            {           
                loopTime = millis();
                                                        
                // Обработчик кнопки menu
                if(analogRead(BTN_MENU) < 800  && flagZ == 0) flagZ = 1;
            
                if(analogRead(BTN_MENU) > 1000 && flagZ == 1) {                     
                    flagZ = 0; 
                    showMenu = 15000; 
                    state = 1; 
                    saveSettingsState = 0;
                    delay(10);
                    break;
                }
                
                // Обработчик кнопки старт тренировка
                if(analogRead(BTN_START) < 800  && flagS == 0) flagS = 1;
            
                if(analogRead(BTN_START) > 1000 && flagS == 1) { 
                    //  Запуск тренировки
                    flagS = 0; 
                    statusTraining = 1;
                    trainingTime = selectTime; 
                    waiting = 5;
                    tone(SIGNAL, 2500, 50);  // Звуковой сигнал - короткий
                    break;
                }
            }
            
            //  Если тренировка запущена
            if(statusTraining) 
            {
                if(!statusWaiting)
                {                    
                    //  Ожидание запуска в миллисекундах (5000)
                    if(millis() - loopTime > 1000)
                    {  
                        loopTime = millis();
                        
                        if(waiting > 1)  
                        {
                            tone(SIGNAL, 750, 200);  // Звуковой сигнал - короткий
                        }
                        else
                        {
                            tone(SIGNAL, 850, 750); // Звуковой сигнал - длинный
                            statusWaiting = 1;
                            Timer1.start();  //  Запуск отсчета (таймера)
                        }
                        
                        waiting--;
                        lcdUpdate("    Waiting", "       " + String(waiting));  // Выводим обратный отсчет
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
                            counterStopZones++;  //  Засчитываем балл
                            for(int i = 0; i < 8; i++) Shifter.setRegisterPin(i, HIGH);    //  Выключаем красный фонарь подсветки зоны
                                                        
                            // Издаем сигнал о зачислении
                            for(int u = 0; u < 5; u++) { tone(SIGNAL, 1200, 30); delay(100); }                            
                            delay(300);
                            for(int u = 0; u < 5; u++) { tone(SIGNAL, 1200, 30); delay(100); }
                            
                            statusZones[currentZone] = 0;  //  Обнуляем текущую зону                     
                            loopTime = 0;  //  Обнуляем счетчик времени вывода на экран
                        }
                    }

                    //  Выводим обратный отсчет времени тренировки, подсчет текущих результатов (забито/пропущено)
                    if(millis() - loopTime > 300)
                    {     
                        loopTime = millis();  
                        
                        lcd.clear();
                        lcd.setCursor(5, 0);
                        
                        if (trainingTime / 60 % 60 < 10) { lcd.print ("0"); }
                        lcd.print ((trainingTime / 60) % 60);
                        lcd.print (":");
                        if (trainingTime % 60 < 10) { lcd.print ("0"); }
                        lcd.print(trainingTime % 60);
                         
                        lcd.setCursor(2, 1);

                        if(counterStopZones <= 9) {
                            lcd.print("00"); lcd.print(counterStopZones);
                        } else if(counterStopZones >= 10 && counterStopZones <= 99) {
                                lcd.print("0"); lcd.print(counterStopZones);
                        } else {
                            lcd.print(counterStopZones);
                        }

                        lcd.print("|");

                        if(counterAllZones <= 9) {
                              lcd.print("00"); lcd.print(counterAllZones);
                        } else if(counterAllZones >= 10 && counterAllZones <= 99) {
                              lcd.print("0"); lcd.print(counterAllZones);
                        } else {
                            lcd.print(counterAllZones);
                        }

                        lcd.print("|");

                        if(counterBrekZones <= 9) {
                              lcd.print("00"); lcd.print(counterBrekZones);
                        } else if(counterBrekZones >= 10 && counterBrekZones <= 99) {
                              lcd.print("0"); lcd.print(counterBrekZones);
                        } else {
                              lcd.print(counterBrekZones);
                        }
                    }
                
                    //  Если текущая (сперва нулевая) зона не запущена
                    if(statusZones[currentZone] == 0)
                    {                        
                        startZone();  //  Выбираем рандомно зону и включаем и записываем время включения
                        statusZones[currentZone] = 1;
                        Shifter.setRegisterPin(currentZone, LOW);     //  Включаем красный фонарь подсветки зоны
                        zoneStartTime = millis();  //  Помечаем время запуска зоны
                        counterAllZones++;  //  Плюсуем общий счетчик зон
                    }
    
                    //  Смотрим состояние включеной зоны и время, не пора ли сменить если вышло время (тоесть выключить до следующего круга программы - а там и включится новая)
                    if( millis() - zoneStartTime > (levelSpeed * 1000) && statusZones[currentZone] == 1)
                    {                                                
                        statusZones[currentZone] = 0;  //  Обнуляем текущую зону
                        for(int i = 0; i < 8; i++) Shifter.setRegisterPin(i, HIGH);   //  Выключаем красный фонарь подсветки зоны
                        counterBrekZones++;  //  Плюсуем упущенную
                        loopTime = 0;
                        break;
                    }
                }
            }
            
            if(trainingTime == 0 && waiting == 0) 
            {
                for(int i = 0; i < 8; i++) Shifter.setRegisterPin(i, HIGH);   //  Обнуляем счетчики и выключаем все зоны
                lcdUpdate("Stop zones: " + String(counterStopZones), "Break zones: " + String(counterBrekZones));  //  Выводим результаты (если надо дополняем ожиданием до нажатия кнопки)
                statusTraining = statusWaiting = 0;
                zoneStartTime = 0;
                counterStopZones = counterAllZones = counterBrekZones = 0;
                statusZones[currentZone] = 0;
                trainingTime = selectTime;
                delay(15000); clearResults();
            }
        }
    }
}

