# BOLOSHOT-DISPENSER

Automatyczny nalewak do alkoholu oparty na mikrokontrolerze Arduino, zdolny do obsługi sześciu kieliszków jednocześnie. Projekt łączy elektronikę, mechanikę oraz oprogramowanie embedded, tworząc w pełni funkcjonalne urządzenie imprezowo-użytkowe.

## Opis działania

Automatyczny nalewak BOLOSHOT-DISPENSER wykorzystuje sześć czujników optycznych do wykrywania obecności kieliszków. Mikrokontroler Arduino analizuje stan każdego sensora i na tej podstawie ustala liczbę aktywnych stanowisk. System blokuje nalewanie w dwóch przypadkach:

- gdy nie wykryto żadnego kieliszka,
- gdy wykryty jest tylko jeden kieliszek (z informacją ostrzegawczą).

Jeśli wykryte zostaną co najmniej dwa kieliszki, urządzenie rozpoczyna sekwencję nalewania. Serwomechanizm ustawia dyszę nad kolejnymi stanowiskami zgodnie z przypisanymi kątami, a pompka jest aktywowana na czas obliczony na podstawie parametrów kalibracyjnych. Po każdym nalaniu wykonywana jest przerwa umożliwiająca odcieknięcie pozostałości płynu. Po zakończeniu pracy serwo wraca do pozycji startowej, a urządzenie wyświetla ekran główny.

## Galeria zdjęć

Zdjęcia urządzenia znajdują się w folderze docs/.

## Krótki filmik pokazujący działanie

https://www.youtube.com/shorts/wK4lQKyGzNw

## Użyte komponenty

- Mikrokontroler: Arduino UNO R3
- Serwomechanizm: TowerPro SG90
- Pompa dozująca 12V
- Przekaźnik SRD-5VDC-SL-C
- Wyświetlacz LCD 16×2 (I2C)
- 6 czujników optycznych TCRT5000
- 3 przyciski chwilowe
- Zasilacz + okablowanie
- Konstrukcja: kantówki + płyta HDF
- Wężyk 6 mm


## Kalibracja systemu dozowania

Dozowanie płynu oparte jest na zależności czasowej — urządzenie oblicza, jak długo pompka powinna pracować, aby uzyskać określoną objętość. W kodzie znajduje się współczynnik kalibracyjny:

```cpp
const float KALIBRACJA = 6.6f;
```

Parametr ten koryguje różnice między teoretycznym a rzeczywistym przepływem pompki. Jego wartość ustalana jest doświadczalnie.
