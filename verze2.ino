// Configuration constants in seconds
#define HEARTBEAT_TIMEOUT 60      // Timeout for heartbeat detection
#define POST_RESET_DELAY 60       // Delay after reset for RPi boot
#define MOSFET_OFF_TIME 10        // Time to keep MOSFET off during reset
#define MAX_RESETS 5              // Maximum number of reset attempts
#define LED_ON_TIME 50            // LED blink ON time in ms for error state
#define LED_OFF_TIME 2000         // LED blink OFF time in ms for error state

// Pin definitions
#define LED_PIN PB0               // Status LED pin
#define MOSFET_PIN PB1            // MOSFET output pin
#define HB_INPUT PB2              // Heartbeat input pin

// Global variables
volatile bool heartbeatDetected = false;  // Flag for heartbeat detection
unsigned long lastHeartbeatTime = 0;     // Last heartbeat timestamp in seconds
int resetCount = 0;                      // Counter for reset attempts
bool errorState = false;                 // Error state flag

// Interrupt Service Routine for heartbeat detection
ISR(PCINT0_vect) {
  if (digitalRead(HB_INPUT) == HIGH) {
    heartbeatDetected = true;
  }
}

void setup() {
  // Initialize pins
  pinMode(LED_PIN, OUTPUT);
  pinMode(MOSFET_PIN, OUTPUT);
  pinMode(HB_INPUT, INPUT);
  
  // Initialize MOSFET to ON (RPi powered)
  digitalWrite(MOSFET_PIN, HIGH);
  
  // Enable pin change interrupt for PB2
  GIMSK |= (1 << PCIE);     // Enable pin change interrupt
  PCMSK |= (1 << PCINT2);   // Enable interrupt for PB2
  sei();                    // Enable global interrupts
}

void loop() {
  // Check if in error state
  if (errorState) {
    // Blink LED in error pattern (H50/L2000)
    digitalWrite(LED_PIN, HIGH);
    delay(LED_ON_TIME);
    digitalWrite(LED_PIN, LOW);
    delay(LED_OFF_TIME);
    return;
  }

  // Check for heartbeat
  if (heartbeatDetected) {
    // Blink LED to indicate heartbeat received
    digitalWrite(LED_PIN, HIGH);
    delay(50);  // Short blink for heartbeat
    digitalWrite(LED_PIN, LOW);
    
    lastHeartbeatTime = 0;  // Reset timer
    heartbeatDetected = false;  // Clear flag
  }

  // Increment time (1 second per loop)
  delay(1000);
  lastHeartbeatTime++;

  // Check for heartbeat timeout
  if (lastHeartbeatTime >= HEARTBEAT_TIMEOUT && resetCount < MAX_RESETS) {
    // Perform reset
    digitalWrite(MOSFET_PIN, LOW);  // Turn off RPi
    delay(MOSFET_OFF_TIME * 1000);  // Keep off for specified time
    digitalWrite(MOSFET_PIN, HIGH); // Turn RPi back on
    
    resetCount++;  // Increment reset counter
    
    // Wait for RPi to boot
    lastHeartbeatTime = 0;
    delay(POST_RESET_DELAY * 1000);
  }
  
  // Check if max resets reached
  if (resetCount >= MAX_RESETS) {
    errorState = true;  // Enter error state
    digitalWrite(MOSFET_PIN, LOW);  // Permanently turn off RPi
  }
}
