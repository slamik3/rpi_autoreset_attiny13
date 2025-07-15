// -----------------------------------------------------------
// ATtiny13 RPi Heartbeat Monitor & Reset
// Detekuje heartbeat na PB2 (INT0). Pokud není detekován
// po TIMEOUT_SEC, přeruší napájení RPi vypnutím MOSFETu
// (PB1) na RESET_DURATION_SEC. Max. počet resetů MAX_RESETS,
// pak přejde do chy­bového stavu (bliká LED na PB0).
// -----------------------------------------------------------

#define TIMEOUT_SEC            60      // čekání na heartbeat
#define RESET_DURATION_SEC     10      // doba vypnutí napájení RPi
#define MAX_RESETS             5       // maximální počet resetů
#define HB_LED_ON_MS           100     // doba bliknutí LED při příjmu hb
#define FAULT_LED_ON_MS        50      // LED ON při chybovém stavu
#define FAULT_LED_OFF_MS       2000    // LED OFF při chybovém stavu

const uint8_t LED_PIN     = PB0;  // stavová LED
const uint8_t MOSFET_PIN  = PB1;  // ovládání napájení
const uint8_t HB_PIN      = PB2;  // heartbeat input (INT0)

volatile bool hbReceived = false;

ISR(PCINT0_vect) {
  // zachytíme náběžnou hranu hb signálu
  if (digitalRead(HB_PIN) == HIGH) {
    hbReceived = true;
  }
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(MOSFET_PIN, OUTPUT);
  pinMode(HB_PIN, INPUT);
  
  // RPi napájení zapnuto a LED zhasnuta
  digitalWrite(MOSFET_PIN, HIGH);
  digitalWrite(LED_PIN, LOW);

  // Povolit pin-change interrupt na PB2 (PCINT2)
  GIMSK  |= (1 << PCIE);
  PCMSK  |= (1 << PCINT2);
  sei();
}

void loop() {
  static uint8_t resetCount = 0;

  // ---------------------------------------------------------
  // 1) Čekání na heartbeat (max TIMEOUT_SEC)
  // ---------------------------------------------------------
  long elapsed = 0;
  hbReceived = false;
  while (elapsed < TIMEOUT_SEC) {
    if (hbReceived) {
      // úspěšné přijetí - bliknout LED
      digitalWrite(LED_PIN, HIGH);
      delay(HB_LED_ON_MS);
      digitalWrite(LED_PIN, LOW);
      break;
    }
    delay(1000);
    elapsed++;
  }

  // ---------------------------------------------------------
  // 2) Pokud heartbeat neproběhl, provést reset RPi
  // ---------------------------------------------------------
  if (!hbReceived) {
    if (resetCount < MAX_RESETS) {
      // vypnout napájení MOSFETem
      digitalWrite(MOSFET_PIN, LOW);
      delay(RESET_DURATION_SEC * 1000);
      // znovu zapnout napájení
      digitalWrite(MOSFET_PIN, HIGH);
      resetCount++;

      // nechat RPi naběhnout před dalším čekáním
      long bootWait = 0;
      while (bootWait < TIMEOUT_SEC) {
        delay(1000);
        bootWait++;
      }
    }
    else {
      // -----------------------------------------------------
      // 3) Chybový stav: překročen počet resetů -> trvalé vypnutí
      //    a blikání LED (FAULT_LED_ON_MS/HIGH, FAULT_LED_OFF_MS/LOW)
      // -----------------------------------------------------
      digitalWrite(MOSFET_PIN, LOW);
      while (true) {
        digitalWrite(LED_PIN, HIGH);
        delay(FAULT_LED_ON_MS);
        digitalWrite(LED_PIN, LOW);
        delay(FAULT_LED_OFF_MS);
      }
    }
  }
  // pokud hbReceived == true, smyčka se jen zopakuje a čeká znovu
}
