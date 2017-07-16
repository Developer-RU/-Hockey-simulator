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
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////

/*
#define LCD_HEIGHT        2                //  Высота дисплея в строках
#define LCD_WIDTH        16                //  Ширина дисплея в символах

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

unsigned long zoneStartTime = 0, zoneStopTime = 0, zoneBrekTime = 0, showMenu = 0, currentTime = 0, loopTime = 0;

int statusZones[5] = {0, 0, 0, 0, 0};

int statusTraining = 0, statusWaiting = 0, waiting = 5, trainingTime = 300, durationTime = 5;
int currentZone = 0, previouZone = 0, counterAllZones = 0, counterStopZone = 0, counterBrekZone = 0;
int currentMenuItem = 0, state = 0, saveSettingsState = 1, flagX = 0, flagZ = 0, flagY = 0, flagS = 0;
*/

#define LCD_HEIGHT        2                //  Высота дисплея в строках
#define LCD_WIDTH        16                //  Ширина дисплея в символах

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

int statusTraining = 0;                    //  Статус тренировки (0 - остановлена, если больше - в процессе)
int statusWaiting = 0;                     //  Статус задержки перед началом
int waiting = 5;                           //  Задержка перед началом в секундах

unsigned long showMenu = 0;                //  Хранит время показа меню
unsigned long currentTime = 0;             //  Хранит текущее время
unsigned long loopTime = 0;                //  Хранит заданное время


int trainingTime = 300;                    //  Продолжительность тренировки 
int durationTime = 5;                      //  Время сложности (длительности включения зоны)


unsigned long zoneStartTime = 0;           //  Время включения зоны
unsigned long zoneStopTime = 0;            //  Время выключения зоны игроком
unsigned long zoneBrekTime = 0;            //  Время выключения зоны по длительности


int statusZones[5] = {0, 0, 0, 0, 0};      //  Состояния зон

int currentZone = 0;                       //  Номер текущей зоны
int previouZone = 0;                       //  Номер предыдущей зоны

int counterAllZones = 0;                   //  Всего было включено зон
int counterStopZone = 0;                   //  Захвачено зон 
int counterBrekZone = 0;                   //  Пропущено зон

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
//   Чтение адреса епром   int  //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
byte eepromRead(int address)
{
    return EEPROM.read(address);
}

//////////////////////////////////////////////////////////////////////////
//   Чтение адреса епром  float   ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
float EEPROM_float_read(int addr)
{    
    byte raw[4];
    for(byte i = 0; i < 4; i++) raw[i] = EEPROM.read(addr+i);
    float &num = (float&)raw;
    delay(100);

    return num;
}

//////////////////////////////////////////////////////////////////////////
//  Запись  адреса епром  int   //////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void eepromWrite(int address, int value)
{
    EEPROM.write(address, value);
    delay(100);
}

//////////////////////////////////////////////////////////////////////////
//  Запись  адреса епром  float   ////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void EEPROM_float_write(int addr, float num)
{
    byte raw[4];
    (float&)raw = num;
    
    for(byte i = 0; i < 4; i++) 
    {
        EEPROM.write(addr+i, raw[i]);
    }
    
    delay(100);
}

//////////////////////////////////////////////////////////////////////////
//  Установка настроект по умолчанию       ///////////////////////////////
//////////////////////////////////////////////////////////////////////////
void defaultSettings()
{        
    eepromClear();
            
    //EEPROM.write(0, 1);
    //EEPROM_float_write(5, 22.0);
    
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
            clearPrintTitle("Duration time");
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
            
            clearPrintTitle("Duration time");
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
    
    while(currentZone == previouZone && !((bool)digitalRead(8 + currentZone))) currentZone = random(0, 5);
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
    pinMode(SIGNAL, OUTPUT);
    digitalWrite(SIGNAL, LOW);

    // Назначение пинов зон
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

    clearPrintTitle("     Hockey!   ");
    lcd.print ("    SIMULATOR   ");
    
    delay(3000);  

    //  Мелодия заставки
    
    /*
        tone(SIGNAL, 700, 300); delay(600);
        tone(SIGNAL, 700, 300); delay(600);
        tone(SIGNAL, 780, 150); delay(300);
        tone(SIGNAL, 700, 150); delay(300);
        tone(SIGNAL, 625, 450); delay(600);
        tone(SIGNAL, 590, 150); delay(300);
        tone(SIGNAL, 520, 150); delay(300);
        tone(SIGNAL, 460, 450); delay(600);
        tone(SIGNAL, 350, 450); delay(600);
        delay(600);
        tone(SIGNAL, 350, 450); delay(600);
        tone(SIGNAL, 460, 450); delay(600);
        tone(SIGNAL, 520, 150); delay(300);
        tone(SIGNAL, 590, 150); delay(300);
        tone(SIGNAL, 625, 450); delay(600);
        tone(SIGNAL, 590, 150); delay(300);
        tone(SIGNAL, 520, 150); delay(300);
        tone(SIGNAL, 700, 1350); delay(1800);
        tone(SIGNAL, 700, 300); delay(600);
        tone(SIGNAL, 700, 300); delay(600);
        tone(SIGNAL, 780, 150); delay(300);
        tone(SIGNAL, 700, 150); delay(300);
        tone(SIGNAL, 625, 450); delay(600);
        tone(SIGNAL, 590, 150); delay(300);
        tone(SIGNAL, 520, 150); delay(300);
        tone(SIGNAL, 460, 450); delay(600);
        tone(SIGNAL, 350, 450); delay(600);
        delay(600);
        tone(SIGNAL, 350, 450); delay(600);
        tone(SIGNAL, 625, 450); delay(600);
        tone(SIGNAL, 590, 150); delay(300);
        tone(SIGNAL, 520, 150); delay(300);
        tone(SIGNAL, 700, 450); delay(600);
        tone(SIGNAL, 590, 150); delay(300);
        tone(SIGNAL, 520, 150); delay(300);
        tone(SIGNAL, 460, 1350); delay(5000);
     */
 
    clearPrintTitle("    TRAINING   ");
    lcd.print ("   PRESS START  ");

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
        } 
        else 
        {               
            if(saveSettingsState == 0)
            {
                currentMenuItem = 0;
                
                //EEPROM_float_write(5, temp_room);   // Сохраняем установленную темпиратуру комнаты
                //EEPROM.write(4, size_boiler);

                saveSettingsState = 1;

                clearPrintTitle("    SETTINGS    ");
                lcd.print ("     SAVED     ");
                delay(500);
                loopTime = 0;

                clearPrintTitle("    TRAINING   ");
                lcd.print ("   PRESS START  ");
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
                }
                
                // Обработчик кнопки старт тренировка
                if(analogRead(BTN_START) < 800  && flagS == 0) flagS = 1;
            
                if(analogRead(BTN_START) > 1000 && flagS == 1) 
                { 
                    //  Запуск тренировки
                    flagS = 0; 
                    statusTraining++;
                    loopTime = 0;
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
                        //loopTime = currentTime;
                        
                        
                        if(waiting > 1)
                        {
                            // Звуковой сигнал - короткий
                            tone(SIGNAL, 500, 300);
                        }
                        else
                        {
                            // Звуковой сигнал - длинный
                            tone(SIGNAL, 500, 550);
                            
                            statusWaiting++;
    
                            //  Запуск отсчета (таймера)
                            Timer1.start();
                        }
    
                        waiting--;
                        
                        // Выводим обратный отсчет
                        lcdUpdate("    Waiting", "       " + String(waiting));
                        
                        loopTime = currentTime;                   
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
                            counterStopZone++;
    
                            // Издаем сигнал о зачислении
                            tone(SIGNAL, 1000, 100);
    
                            //  Обнуляем текущую зону
                            statusZones[currentZone] = 0;
    
                            //  Обнуляем счетчик времени вывода на экран
                            loopTime = 0;
                        }
                    }
                    
                    if(currentTime >= (loopTime + 500))
                    {  
                        //  Записываем новую метку времени
                        loopTime = currentTime;                  
                        
                        //  Выводим обратный отсчет времени тренировки, подсчет текущих результатов (забито/пропущено)

                        lcd.clear();
                        
                        lcd.setCursor(5, 0);
                        
                        if (trainingTime / 60 % 60 < 10) { lcd.print ("0"); }
                        lcd.print ((trainingTime / 60) % 60);
                        lcd.print (":");
                        if (trainingTime % 60 < 10) { lcd.print ("0"); }
                        lcd.print(trainingTime % 60);
                         
                        lcd.setCursor(2, 1);

                        if(counterStopZone <= 9) 
                        {
                             lcd.print("00"); lcd.print(counterStopZone);
                        }
                        else if(counterStopZone >= 10 && counterStopZone <= 99)
                        {
                             lcd.print("0"); lcd.print(counterStopZone);
                        } else {
                             lcd.print(counterStopZone);
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

                        if(counterBrekZone <= 9) 
                        {
                             lcd.print("00"); lcd.print(counterBrekZone);
                        }
                        else if(counterBrekZone >= 10 && counterBrekZone <= 99)
                        {
                             lcd.print("0"); lcd.print(counterBrekZone);
                        } else {
                             lcd.print(counterBrekZone);
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
                        
                        //  Помечаем время запуска зоны
                        zoneStartTime = currentTime;
                        
                        //  Плюсуем общий счетчик зон
                        counterAllZones++; 
    
                        //  Издаем сигнал о старте зоны
                        //tone(SIGNAL, 1000, 100);
                    }
    
                    //  Смотрим состояние включеной зоны и время, не пора ли сменить если вышло время (тоесть выключить до следующего круга программы - а там и включится новая)
                    if( currentTime >= (zoneStartTime + (durationTime*1000)) && statusZones[currentZone] == 1)
                    {                        
                        //  Обнуляем текущую зону
                        statusZones[currentZone] = 0;
    
                        //  Плюсуем упущенную
                        counterBrekZone++;
    
                        //  Издаем сигнал о пропуске
                        tone(SIGNAL, 200, 100);
                            
                        //  Обнуляем счетчик времени
                        loopTime = 0;
                    }
                }
            }
        
            if(!trainingTime)
            {
                //  Остановка отсчета (таймера)
                Timer1.stop();
                
                //  Выводим результаты (если надо дополняем ожиданием до нажатия кнопки)
                lcdUpdate("Stop zones: " + String(counterStopZone), "Break zones: " + String(counterBrekZone));
                delay(100);
            }

        }
    }
}

