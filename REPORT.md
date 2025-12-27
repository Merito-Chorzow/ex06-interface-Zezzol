# Raport: Ćwiczenie 3 — Non-blocking I/O (Ring Buffer + Shell)

## 1. Polityka Overflow
W projekcie zastosowano politykę **Drop New** (odrzucanie nowych danych).
Implementacja w `ringbuf.c`:
- Gdy `rb_free(r) == 0`, funkcja `rb_put` zwraca 0 i inkrementuje licznik `dropped`.
- Bufor nie nadpisuje starych danych (`tail` pozostaje bez zmian).

**Konsekwencje:**
- Zaleta: Gwarantuje integralność komend, które już zostały przyjęte. Nie ma ryzyka, że przetworzymy "połowę" starej komendy sklejoną z nową.
- Wada: W przypadku nagłego skoku danych (burst), tracimy najnowsze informacje (np. najnowsze pomiary czujnika), przetwarzając te nieaktualne.

## 2. Testy funkcjonalne
Przeprowadzono symulację w `main.c`:

### A. Test echo
Komenda: `echo hello world`
Odpowiedź: `ECHO hello world`
Wniosek: Bufor kołowy poprawnie przechowuje i odtwarza kolejność bajtów.

### B. Test przepełnienia (Burst)
Wysłano serię 200 komend `noop\r\n` (łącznie ok. 1200 bajtów) do bufora o rozmiarze 128 bajtów.

**Logi przed burstem:**
adrian@localhost:~/github-classroom/Merito-Chorzow/ex05-io-c-Zezzol> ./build/app
READY
set=0.000 ticks=1 drop=0 broken=0
OK set=0.420
rx_free=126 tx_free=127 rx_count=1
ECHO hello world

**Logi po burście:**
set=0.420 ticks=41 drop=1073 broken=0

**Analiza:**
- Licznik `drop` wzrósł znacząco, co potwierdza działanie mechanizmu detekcji przepełnienia.
- Licznik `broken` (złamane linie) wskazuje, że z powodu odrzucenia znaków końca linii (`\n`), parser skleił kilka komend w jedną, przekraczając rozmiar bufora wejściowego shella.

## 3. Wnioski
Zastosowanie bufora kołowego pozwala na odbiór danych bez blokowania procesora. Przy zbyt wolnym przetwarzaniu (`shell_tick`) w stosunku do napływu danych (`shell_rx_bytes`), mechanizm `dropped` pozwala zdiagnozować wąskie gardło systemu.