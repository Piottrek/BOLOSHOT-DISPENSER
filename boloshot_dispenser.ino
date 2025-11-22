#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

byte ZNAK_KIELISZKA[8] = {
  B00100,
  B01110,
  B10101,
  B10101,
  B10101,
  B11111,
  B01110,
  B00000
};


const uint8_t PIN_POMPA     = 10; // pompa
const uint8_t PIN_SERWO     = 9;  // serwo
const uint8_t PIN_START     = 13; // start
const uint8_t PIN_PLUS      = 2;  // +
const uint8_t PIN_MINUS     = 12; // -
const uint8_t PINY_CZUJNIKOW[6]   = {3, 4, 5, 6, 7, 8}; // piny czujnika

int POZYCJE_SERWA[6] = {25, 55, 88, 118, 143, 175};
const int KAT_STARTOWY = 5;

const bool PRZEKAZNIK_AKTYWNY_NISKIM = true;

//100l/h
const float PRZEPLYW_ML_NA_S = 27.8f;
const float KALIBRACJA = 6.6f;  

const int ML_MIN = 5;
const int ML_MAX = 120;
int docelowe_ml = 30;

// "odkapanie"
const unsigned long OCZEKIWANIE_KAPANIE_MS = 10000;
const unsigned long SERWO_STABIL_MS = 250;
const unsigned long ANTYDRGAN_MS = 40;

Servo serwo;

enum PrzyciskIdx : uint8_t { IDX_START = 0, IDX_PLUS = 1, IDX_MINUS = 2 };
unsigned long czasZmianyPrzycisku[3] = {0, 0, 0};
bool stanPrzycisku[3] = {HIGH, HIGH, HIGH};

void ustawPompe(bool wlacz)
{
    if (PRZEKAZNIK_AKTYWNY_NISKIM)
    {
        if (wlacz)
        {
            digitalWrite(PIN_POMPA, LOW);
        }
        else
        {
            digitalWrite(PIN_POMPA, HIGH);
        }
    }
    else
    {
        if (wlacz)
        {
            digitalWrite(PIN_POMPA, HIGH);
        }
        else
        {
            digitalWrite(PIN_POMPA, LOW);
        }
    }
}

unsigned long obliczCzasWylewaniaMs(int ml)
{
    return (unsigned long)((ml * KALIBRACJA / PRZEPLYW_ML_NA_S) * 1000.0f);
}

void pokazLogo()
{
    lcd.clear();

    lcd.setCursor(4, 0);
    lcd.print("BOLOSHOT");

    lcd.setCursor(1, 1);
    for (int i = 0; i < 6; i++)
    {
        lcd.write((uint8_t)0); 
        lcd.print("  ");
    }

    delay(7000);
    lcd.clear();
}


bool wcisnieto(int pin, int idx)
{
    bool aktualnyStanPinu = digitalRead(pin);

    if (aktualnyStanPinu != stanPrzycisku[idx] && (millis() - czasZmianyPrzycisku[idx]) > ANTYDRGAN_MS)
    {
        stanPrzycisku[idx] = aktualnyStanPinu;
        czasZmianyPrzycisku[idx] = millis();

        if (aktualnyStanPinu == LOW)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}

int policzWykryte()
{
    uint8_t licznik = 0;

    for (uint8_t i = 0; i < 6; i++)
    {
        if (digitalRead(PINY_CZUJNIKOW[i]) == LOW)
        {
            licznik++;
        }
    }

    return licznik;
}

void obrocISpoczekaj(int kat)
{
    serwo.write(kat);
    delay(SERWO_STABIL_MS);
}


void lcdPokazIlosc()
{
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ilosc: ");
    lcd.print(docelowe_ml);
    lcd.print(" ml   ");

    lcd.setCursor(0, 1);
    lcd.print("Kieliszki:   ");
    lcd.print(policzWykryte());
    lcd.print("/6     ");
}

void lejTylkoObecne()
{
    bool kieliszekWykryty[6];
    uint8_t liczbaWykrytychKieliszkow = 0;

    for (uint8_t i = 0; i < 6; i++)
    {
        if (digitalRead(PINY_CZUJNIKOW[i]) == LOW) 
        {
            kieliszekWykryty[i] = true;
            liczbaWykrytychKieliszkow++;
        }
        else
        {
            kieliszekWykryty[i] = false;
        }
    }

    if (liczbaWykrytychKieliszkow == 0)
    {
        obrocISpoczekaj(KAT_STARTOWY);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Brak kieliszkow");
        lcd.setCursor(0, 1);
        lcd.print("POWROT");

        delay(900);
        lcdPokazIlosc();

        return;
    }


    if (liczbaWykrytychKieliszkow == 1)
    {
        obrocISpoczekaj(KAT_STARTOWY);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("NI CHUJA SAM");
        lcd.setCursor(0, 1);
        lcd.print("NIE PIJESZ!");

        delay(2000);
        lcdPokazIlosc();
        return;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nalewanie! Czekaj!");
    lcd.setCursor(0, 1);
    lcd.print(docelowe_ml);
    lcd.print(" ml (");
    lcd.print(liczbaWykrytychKieliszkow);
    lcd.print(") ");

    unsigned long czasWylewania_ms = obliczCzasWylewaniaMs(docelowe_ml);

    for (uint8_t i = 0; i < 6; i++)
    {
        if (kieliszekWykryty[i] == false)
        {
            continue;
        }

        obrocISpoczekaj(POZYCJE_SERWA[i]);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Kieliszek ");
        lcd.print(i + 1);

        lcd.setCursor(0, 1);
        lcd.print("Nalewamy!");

        ustawPompe(true);
        delay(czasWylewania_ms);
        ustawPompe(false);

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Czekaj 10s...");
        lcd.setCursor(0, 1);
        lcd.print("Kap kap kap...");
        delay(OCZEKIWANIE_KAPANIE_MS);
    }

    obrocISpoczekaj(KAT_STARTOWY);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Zakonczono");
    lcd.setCursor(0, 1);
    lcd.print("Powrot do 5 st");
    delay(900);

    lcdPokazIlosc();
}

void setup()
{
    pinMode(PIN_POMPA, OUTPUT);
    ustawPompe(false);

    pinMode(PIN_START, INPUT_PULLUP);
    pinMode(PIN_PLUS,  INPUT_PULLUP);
    pinMode(PIN_MINUS, INPUT_PULLUP);

    for (uint8_t i = 0; i < 6; i++)
    {
        pinMode(PINY_CZUJNIKOW[i], INPUT);
    }

        serwo.attach(PIN_SERWO);
    serwo.write(KAT_STARTOWY);
    delay(300);

    lcd.init();
    lcd.backlight();

    lcd.createChar(0, ZNAK_KIELISZKA);
    pokazLogo();
    lcdPokazIlosc();

}

void loop()
{
    if (wcisnieto(PIN_PLUS, IDX_PLUS))
    {
        if (docelowe_ml < ML_MAX)
        {
            docelowe_ml++;
        }

        lcdPokazIlosc();
    }

    if (wcisnieto(PIN_MINUS, IDX_MINUS))
    {
        if (docelowe_ml > ML_MIN)
        {
            docelowe_ml--;
        }

        lcdPokazIlosc();
    }

    if (wcisnieto(PIN_START, IDX_START))
    {
        lejTylkoObecne();
    }

    static unsigned long ostatniaAktualizacja = 0;

    if (millis() - ostatniaAktualizacja > 600)
    {
        ostatniaAktualizacja = millis();

        lcd.setCursor(12, 1);
        lcd.print("     ");
        lcd.setCursor(12, 1);
        lcd.print(policzWykryte());
        lcd.print("/6 ");
    }
}
