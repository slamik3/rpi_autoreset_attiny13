// --- ČASOVÉ KONSTANTY ---
#define BOOT_WAIT_SECONDS         600   // Čekání po startu systému (např. RPi start)
#define HEARTBEAT_TIMEOUT_SECONDS 30    // Počet sekund bez heartbeat signálu před restartem
#define RESET_DURATION_SECONDS    10    // Doba vypnutí napájení (reset)

// --- OCHRANA PROTI CYKLENÍ ---
#define MAX_FAILED_RESETS         5     // Maximální počet pokusů o restart bez heartbeat

// --- PIN DEFINICE ---
#define MOSFET_PIN     2   // PB2 = pin 7 (výstup na MOSFET pro napájení RPi)
#define HB_PIN         1   // PB1 = pin 6 (vstup - heartbeat, INT0)
#define FEEDBACK_LED   0   // PB0 = pin 5 (výstup - stavová LED)

// --- GLOBÁLNÍ PROMĚNNÉ ---
volatile bool heartbeatReceived = false;
long secondsWithoutHeartbeat = 0;
int failedResets = 0;
bool permanentFailure = false;

void setup() {
  // Nastavení pinů
  pinMode(MOSFET_PIN, OUTPUT);
  digitalWrite(MOSFET_PIN, HIGH);  // Sepni napájení RPi

  pinMode(FEEDBACK_LED, OUTPUT);
  digitalWrite(FEEDBACK_LED, LOW);

  pinMode(HB_PIN, INPUT);  // Vstup pro heartbeat signál

  // Nastavení přerušení INT0 (PB1), náběžná hrana
  GIMSK |= (1 << INT0);
  MCUCR |= (1 << ISC01) | (1 << ISC00);
  sei();

  // Čekání po zapnutí systému
  for (int i = 0; i < BOOT_WAIT_SECONDS; i++) {
    digitalWrite(FEEDBACK_LED, LOW);
    delay(1000);
  }
}

void loop() {
  if (permanentFailure) {
    signalPermanentFailure();
    return;
  }

  delay(1000);

  if (heartbeatReceived) {
    heartbeatReceived = false;
    secondsWithoutHeartbeat = 0;
    failedResets = 0;

    digitalWrite(FEEDBACK_LED, HIGH);
    delay(50);
    digitalWrite(FEEDBACK_LED, LOW);
  } else {
    secondsWithoutHeartbeat++;
  }

  if (secondsWithoutHeartbeat >= HEARTBEAT_TIMEOUT_SECONDS) {
    failedResets++;

    if (failedResets > MAX_FAILED_RESETS) {
      // TRVALÁ PORUCHA – Vypni napájení a přepni do režimu signalizace
      digitalWrite(MOSFET_PIN, LOW);  // Napájení trvale vypnuto
      permanentFailure = true;
      return;
    }

    // Restartuj RPi (dočasné vypnutí napájení)
    digitalWrite(MOSFET_PIN, LOW);
    digitalWrite(FEEDBACK_LED, LOW);
    delay(RESET_DURATION_SECONDS * 1000);

    digitalWrite(MOSFET_PIN, HIGH); // Znovu zapni napájení

    // Čekání po resetu (např. RPi boot)
    for (int i = 0; i < BOOT_WAIT_SECONDS; i++) {
      digitalWrite(FEEDBACK_LED, LOW);
      delay(1000);
    }

    secondsWithoutHeartbeat = 0;
  }
}

// ISR pro externí přerušení na INT0 (PB1)
ISR(INT0_vect) {
  heartbeatReceived = true;
}

// Signalizace trvalé chyby – 3× bliknutí každých 5 sekund
void signalPermanentFailure() {
  // Trvalé vypnutí napájení – pro jistotu i zde
  digitalWrite(MOSFET_PIN, LOW);

  // LED signalizace chyby
  while (true) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(FEEDBACK_LED, HIGH);
      delay(100);
      digitalWrite(FEEDBACK_LED, LOW);
      delay(200);
    }
    delay(5000);  // Pauza 5 sekund mezi trojblikem
  }
}
