# Raport: Ćwiczenie 1 — Interfejs sterujący urządzeniem (Protokół + Driver)

### 1. Cel
Zaprojektowanie i implementacja nieblokującego interfejsu sterującego opartego na własnym protokole binarnym (ramki), wraz z obsługą błędów transmisji, maszyną stanów (FSM) odbiornika oraz symulacją urządzenia wykonawczego.

### 2. Specyfikacja Protokołu

**Format ramki:**
```text
[ STX (1B) | LEN (1B) | CMD (1B) | PAYLOAD (0..N B) | CRC (1B) ]

STX: Stały bajt startu 0xAA.

LEN: Długość pola Payload (0–60 bajtów).

CMD: Kod komendy.

CRC: Suma kontrolna XOR (liczona od pola LEN do końca PAYLOAD).

Endianness: Little-Endian.
```

**Lista Komend:**
| CMD ID | Nazwa     | Parametry (Payload)                       | Odpowiedź  | Opis                                        |
|--------|-----------|-------------------------------------------|------------|---------------------------------------------|
| 0x01   | SET SPEED | [uint8_t speed] (0-100)                   | ACK / NACK | Ustawia prędkość silnika.                   |
| 0x02   | MODE      | [uint8_t mode] (0:OPEN, 1:CLOSED, 2:SAFE) | ACK / NACK | Zmienia tryb pracy.                         |
| 0x03   | STOP      | Brak                                      | ACK        | Natychmiastowe przejście do SAFE i speed=0. |
| 0x04   | GET STAT  | Brak                                      | STAT {...} | Pobiera telemetrię.                         |

**Kody Odpowiedzi:**

ACK (0x80): Operacja powiodła się.

NACK (0x81): Błąd logiczny. Payload zawiera kod błędu:

    0x03: NACK_PARAM (wartość poza zakresem).

    0x04: NACK_LEN (niepoprawna długość ramki).

STAT (0x82): Odpowiedź z danymi telemetrycznymi.

### 3. Implementacja

**Maszyna Stanów (FSM) Odbiornika:**
Driver protokołu (proto.c) działa w oparciu o maszynę stanów, co pozwala na nieblokujące przetwarzanie bajt po bajcie:

    RX_IDLE: Oczekiwanie na bajt STX (0xAA). Każdy inny bajt jest traktowany jako śmieć (rx_dropped).

    RX_WAIT_LEN: Odbiór długości payloadu. Weryfikacja czy długość nie przekracza rozmiaru bufora.

    RX_WAIT_CMD: Odbiór kodu rozkazu.

    RX_PAYLOAD: Pobieranie danych (jeśli LEN > 0).

    RX_WAIT_CRC: Weryfikacja sumy kontrolnej.

**Driver Urządzenia:** 
Symulator (driver.c) implementuje logikę biznesową: waliduje zakresy (np. czy speed <= 100), zmienia stan wewnętrzny urządzenia i generuje odpowiednie ramki zwrotne.

### 4. Telemetria
Statystyki końcowe po wykonaniu testów:

    Valid Frames: 3 (Poprawne ramki SET i GET)

    CRC Errors: 1 (Test celowego błędu)

    RX Dropped: 0 (Brak śmieci na wejściu w symulacji idealnej)

### 5. Wnioski

**Odporność FSM:** 
Maszyna stanów poprawnie synchronizuje się na początku ramki i resetuje w przypadku błędów (np. błędne CRC). Zapobiega to blokowaniu odbiornika w nieskończoność.

**Obsługa błędów:** 
Rozdzielenie błędów warstwy łącza (CRC -> odrzucenie) od błędów warstwy aplikacji (zły parametr -> NACK) pozwala na skuteczną diagnostykę problemów.

**Sugestie rozwoju:**

W obecnej wersji FSM nie posiada timeoutu między bajtami (inter-byte timeout). W przypadku urwania transmisji w połowie ramki, odbiornik "wisi" czekając na resztę danych. Należy dodać timer resetujący FSM do RX_IDLE po np. 5ms ciszy.

Dodanie numeru sekwencyjnego (SEQ) do nagłówka pozwoliłoby na wykrywanie duplikatów ramek (np. gdy ACK zaginie, a nadawca ponowi wysyłkę).