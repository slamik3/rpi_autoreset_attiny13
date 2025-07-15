Watchdog pro Raspberry Pi
Arduino kód pro mikrokontrolér (např. ATtiny), který monitoruje heartbeat signál z Raspberry Pi přes pin PB1. Pokud signál chybí déle než definovaný čas, vypne napájení RPi přes MOSFET (PB2) na 10 sekund a pokusí se o restart. Po překročení maximálního počtu neúspěšných restartů trvale vypne napájení a signalizuje chybu třemi rychlými bliknutími LED (PB0) každých 5 sekund. Po startu čeká 600 sekund na nabootování RPi.

Parametry na začátku kódu
Časové konstanty:
BOOT_WAIT_SECONDS (600): Doba (v sekundách), po kterou mikrokontrolér čeká po zapnutí nebo restartu RPi, aby mělo čas nabootovat.
HEARTBEAT_TIMEOUT_SECONDS (30): Maximální doba (v sekundách) bez přijetí heartbeat signálu, po které se spustí restart RPi.
RESET_DURATION_SECONDS (10): Doba (v sekundách), po kterou je napájení RPi vypnuto během restartu.
Ochrana proti cyklování:
MAX_FAILED_RESETS (5): Maximální počet neúspěšných pokusů o restart RPi. Pokud je překročen, systém přejde do režimu trvalé chyby (vypne NAPÁJENÍ a bliká LED).
Definice pinů:
MOSFET_PIN (2, PB2, pin 7): Výstupní pin pro ovládání MOSFETu, který zapíná/vypíná napájení RPi.
HB_PIN (1, PB1, pin 6): Vstupní pin pro příjem heartbeat signálu od RPi, propojený na přerušení INT0.
FEEDBACK_LED (0, PB0, pin 5): Výstupní pin pro LED, která indikuje stav (bliká při heartbeatu nebo signalizuje trvalou chybu).
Použití
Ideální pro systémy, kde je třeba zajistit nepřetržitý provoz RPi (např. IoT, servery). Kód je jednoduchý, efektivní a využívá přerušení pro detekci heartbeat signálu, aby minimalizoval zátěž mikrokontroléru.
