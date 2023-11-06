#include <Arduino.h>

const int LED_PINS[] = {26, 27, 32, 33};
const int BUTTON_PINS[] = {2, 4, 5, 18};
const int RESET_PIN = 19;
const int NUM_COLORS = 4;

int sequence[NUM_COLORS];
int currentPhase = 1;

enum GameState {
  NOT_STARTED,
  PLAYING,
  AWAITING_USER_INPUT,
  ROUND_SUCCESS,
  GAME_OVER
};

GameState gameState = NOT_STARTED;

void setup() {
  Serial.begin(115200);
  
  for(int i = 0; i < NUM_COLORS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
    digitalWrite(LED_PINS[i], LOW);
  }

  pinMode(RESET_PIN, INPUT_PULLUP);
}

void setPhaseSequence(int phase) {
  switch(phase) {
    case 1: // Passa por todos os LEDs sem repetir nenhum
      for(int i = 0; i < NUM_COLORS; i++) {
        sequence[i] = i;
      }
      break;
    case 2: // Comece pelo último LED
      sequence[0] = 3;
      sequence[1] = 2;
      sequence[2] = 1;
      sequence[3] = 0;
      break;
    case 3: // Intercalado: uma cor sim, outra cor não
      sequence[0] = 0;
      sequence[1] = 2;
      sequence[2] = 1;
      sequence[3] = 3;
      break;
    case 4: // Inverso da terceira fase
      sequence[0] = 1;
      sequence[1] = 3;
      sequence[2] = 0;
      sequence[3] = 2;
      break;
  }
}

void displaySequence() {
  for(int i = 0; i < NUM_COLORS; i++) {
    digitalWrite(LED_PINS[sequence[i]], HIGH);
    delay(500);
    digitalWrite(LED_PINS[sequence[i]], LOW);
    delay(500);
  }
  gameState = AWAITING_USER_INPUT;
}

bool getUserInput() {
  for(int i = 0; i < NUM_COLORS; i++) {
    int buttonPressed = false;
    while(!buttonPressed) {
      for(int j = 0; j < NUM_COLORS; j++) {
        if(digitalRead(BUTTON_PINS[j]) == LOW) {
          if(sequence[i] != j) {
            return false;
          }
          digitalWrite(LED_PINS[j], HIGH);
          delay(500);
          digitalWrite(LED_PINS[j], LOW);
          buttonPressed = true;
          break;  // Saia do loop interno
        }
      }
    }
    delay(500);
  }
  return true;
}

void loop() {
  if(gameState == NOT_STARTED && digitalRead(RESET_PIN) == LOW) {
    currentPhase = 1;
    gameState = PLAYING;
    delay(200);
    setPhaseSequence(currentPhase);
    displaySequence();
  }

  switch (gameState) {
    case AWAITING_USER_INPUT:
      if(getUserInput()) {
        gameState = ROUND_SUCCESS;
      } else {
        gameState = GAME_OVER;
      }
      break;

    case ROUND_SUCCESS:
      Serial.println("Success!");
      currentPhase++;
      if(currentPhase > 4) {
        Serial.println("Congratulations! You've completed all phases. Press reset to play again.");
        gameState = NOT_STARTED;
      } else {
        gameState = PLAYING;
        setPhaseSequence(currentPhase);
        displaySequence();
      }
      delay(1000);
      break;

    case GAME_OVER:
      Serial.println("Try Again! Press reset to play again.");
      gameState = NOT_STARTED;
      delay(1000);
      break;

    default:
      break;
  }
}
