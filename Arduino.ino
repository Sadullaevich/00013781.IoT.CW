#include <Wire.h>
#include <LCD_I2C.h>

// Define pins for LEDs, buttons, ultrasonic sensor, RGB LED, and buzzer
#define LED1_PIN 4
#define LED2_PIN 5
#define BUTTON1_PIN 8
#define BUTTON2_PIN 10
#define TRIG_PIN 12
#define ECHO_PIN 11
#define RED_PIN 3
#define GREEN_PIN 2
#define BLUE_PIN 9
#define BUZZER_PIN 7  // Pin for the buzzer

LCD_I2C lcd(0x27);  // Initialize the LCD (Replace 0x27 with your I2C address)

int level = 1;             // Start at Level 1
int score = 0;             // Initialize score
int ledSequence[10];       // Store LED sequence for levels
int sequenceLength = 1;    // Start with 1 LED in the sequence
int baseSpeed = 500;       // Base speed for LEDs (decreases with levels)

// Function Prototypes
void playStartupMusic();
void generateSequence();
void displaySequence();
bool checkPlayerInput();
void setRGBColor(int red, int green, int blue);
void playSuccessSound();
void playGameOverSound();
float getDistance();
void sendGameData(int level, int score);

void setup() {
  Serial.begin(9600);  // Communication with NodeMCU

  // Initialize pins
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT); // Initialize buzzer pin

  // Turn off LEDs initially
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  setRGBColor(0, 0, 0); // Turn off RGB LED

  // Initialize LCD
  lcd.begin();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Press button or");
  lcd.setCursor(0, 1);
  lcd.print("Wave to start!");

  // Play startup sound
  playStartupMusic();
  setRGBColor(0, 0, 255); // Blue for game start and gameplay
}

void loop() {
  // Wait for a hand wave (distance < 10 cm)
  float distance = getDistance();
  if (distance > 0 && distance < 10) { // Hand detected
    startGame();
  }

  // Wait for the button press
  if (digitalRead(BUTTON1_PIN) == HIGH || digitalRead(BUTTON2_PIN) == HIGH) {
    startGame();
  }
}

void startGame() {
  level = 1;             // Reset game parameters
  score = 0;             // Reset score
  sequenceLength = 1;
  baseSpeed = 500;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Level ");
  lcd.print(level);
  delay(1000);

  playLevel(); // Start gameplay
}

void playLevel() {
  if (level > 10) { // Game over after 10 levels
    sendGameData(level - 1, score);  // Send level and score
    gameOver();
    return;
  }

  lcd.setCursor(0, 0);
  lcd.print("Level ");
  lcd.print(level);

  generateSequence();
  displaySequence();

  if (checkPlayerInput()) {
    playSuccessSound();
    level++;
    sequenceLength++;
    baseSpeed -= 50; // Increase difficulty
    score = level * 5; // Calculate score for the current level
    delay(1000);
    playLevel(); // Proceed to the next level
  } else {
    // If player fails, score will be based on the last successfully completed level
    score = (level - 1) * 5;  // Calculate score based on the last completed level
    sendGameData(level - 1, score); // Player's score based on last completed level
    gameOver();
  }
}

void gameOver() {
  playGameOverSound();
  setRGBColor(255, 0, 0); // Red for game over
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Game Over!");
  delay(2000);

  level = 1;          // Reset to Level 1
  sequenceLength = 1; // Reset sequence length
  baseSpeed = 500;    // Reset speed

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press button or");
  lcd.setCursor(0, 1);
  lcd.print("Wave to start!");
}

void sendGameData(int level, int score) {
  String data = "Level:" + String(level) + ",Score:" + String(score);
  Serial.println(data);  // Send data to NodeMCU via Serial
}

void generateSequence() {
  for (int i = 0; i < sequenceLength; i++) {
    ledSequence[i] = random(1, 3); // Random sequence: 1 for LED1, 2 for LED2
  }
}

void displaySequence() {
  for (int i = 0; i < sequenceLength; i++) {
    int led = ledSequence[i];
    if (led == 1) {
      digitalWrite(LED1_PIN, HIGH);
      tone(BUZZER_PIN, 800, baseSpeed - 200);
      delay(baseSpeed);
      digitalWrite(LED1_PIN, LOW);
      noTone(BUZZER_PIN);
    } else if (led == 2) {
      digitalWrite(LED2_PIN, HIGH);
      tone(BUZZER_PIN, 1000, baseSpeed - 200);
      delay(baseSpeed);
      digitalWrite(LED2_PIN, LOW);
      noTone(BUZZER_PIN);
    }
    delay(300); // Short gap between LEDs
  }
}

bool checkPlayerInput() {
  for (int i = 0; i < sequenceLength; i++) {
    int correctButton = (ledSequence[i] == 1) ? BUTTON1_PIN : BUTTON2_PIN;

    // Wait for player input
    while (true) {
      if (digitalRead(BUTTON1_PIN) == HIGH || digitalRead(BUTTON2_PIN) == HIGH) {
        if (digitalRead(correctButton) == HIGH) {
          delay(300); // Debounce delay
          break;
        } else {
          return false; // Incorrect button pressed
        }
      }
    }
  }
  return true; // Sequence completed successfully
}

void setRGBColor(int red, int green, int blue) {
  analogWrite(RED_PIN, red);
  analogWrite(GREEN_PIN, green);
  analogWrite(BLUE_PIN, blue);
}

void playStartupMusic() {
  tone(BUZZER_PIN, 262, 200); // C4
  delay(300);
  tone(BUZZER_PIN, 330, 200); // E4
  delay(300);
  tone(BUZZER_PIN, 392, 200); // G4
  delay(300);
  tone(BUZZER_PIN, 523, 300); // C5
  delay(500);
  noTone(BUZZER_PIN);
}

void playSuccessSound() {
  tone(BUZZER_PIN, 784, 200); // G5
  delay(300);
  tone(BUZZER_PIN, 1046, 300); // C6
  delay(300);
  noTone(BUZZER_PIN);
}

void playGameOverSound() {
  tone(BUZZER_PIN, 196, 400); // G3
  delay(400);
  tone(BUZZER_PIN, 131, 500); // C3
  delay(500);
  noTone(BUZZER_PIN);
}

float getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH);
  return (duration * 0.034 / 2); // Convert to centimeters
}
