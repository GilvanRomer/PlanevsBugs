#include <LiquidCrystal.h> //Import LiquidCrystal Library

#define PIN_BUTTON 2 //Play button port
#define PIN_AUTOPLAY 1 //Autoplay button port
#define PIN_READWRITE 10 //LCD R/W
#define PIN_CONTRAST 12 //LCD V0

#define SPRITE_AVIAO_BAIXO 1
#define SPRITE_AVIAO_ALTO 2
#define SPRITE_INSETO_BAIXO 3
#define SPRITE_JUMP_UPPER '.'         // Use the '.' character for the head
#define SPRITE_INSETO_ALTO 4
#define SPRITE_VAZIO ' '      // User the ' ' character
#define SPRITE_DOIS_INSETOS 5
#define SPRITE_TIRO_BAIXO 6
#define SPRITE_SPRITE_TIRO_ALTO 7

#define POS_AVIAO 0    // Horizontal position of hero on screen

#define TERRAIN_WIDTH 16
#define VAZIO 0
#define TERRAIN_LOWER_BLOCK 1
#define TERRAIN_UPPER_BLOCK 2

#define HERO_POSITION_OFF 0          // Hero is invisible
#define HERO_POSITION_RUN_LOWER_1 1  // Hero is running on lower row (pose 1)
#define HERO_POSITION_RUN_LOWER_2 2  //                              (pose 2)

#define HERO_POSITION_JUMP_1 3       // Starting a jump
#define HERO_POSITION_JUMP_2 4       // Half-way up
#define HERO_POSITION_JUMP_3 5       // Jump is on upper row
#define HERO_POSITION_JUMP_4 6       // Jump is on upper row
#define HERO_POSITION_JUMP_5 7       // Jump is on upper row
#define HERO_POSITION_JUMP_6 8       // Jump is on upper row
#define HERO_POSITION_JUMP_7 9       // Half-way down
#define HERO_POSITION_JUMP_8 10      // About to land

#define HERO_POSITION_RUN_UPPER_1 11 // Hero is running on upper row (pose 1)
#define HERO_POSITION_RUN_UPPER_2 12 //                              (pose 2)

LiquidCrystal lcd(11, 9, 6, 5, 4, 3); //Initialize lcd
static char terrainUpper[TERRAIN_WIDTH + 1];
static char terrainLower[TERRAIN_WIDTH + 1];
static bool buttonPushed = false;

void initializeGraphics(){
  static byte graphics[] = { //Store new lcd characters
    // Plane down
    B00000,
    B00000,
    B00000,
    B00000,
    B10100,
    B11111,
    B00100,
    B00000,
    // Plane up
    B00000,
    B10100,
    B11111,
    B00100,
    B00000,
    B00000,
    B00000,
    B00000,
    // Bug down
    B00000,
    B00000,
    B00000,
    B00000,
    B10101,
    B01111,
    B10101,
    B00000,
    // Bug up
    B00000,
    B10101,
    B01111,
    B10101,
    B00000,
    B00000,
    B00000,
    B00000,
    // Bug duo
    B00000,
    B10101,
    B01111,
    B10101,
    B00000,
    B10101,
    B01111,
    B10101,
    // Shot down
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00100,
    B00000,
    B00000,
    // Shot up
    B00000,
    B00000,
    B00100,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
  };
  int i;
  // Skip using character 0, this allows lcd.print() to be used to
  // quickly draw multiple characters
  for (i = 0; i < 7; ++i) {
	  lcd.createChar(i + 1, &graphics[i * 8]); // Create 7 new caracters(defined in the binary graphics array)
  }
  for (i = 0; i < TERRAIN_WIDTH; ++i) {
    terrainUpper[i] = SPRITE_VAZIO;
    terrainLower[i] = SPRITE_VAZIO;
  }
}

// Slide the terrain to the left in half-character increments
//
void ProxFrame(char* terrain, byte newTerrain){
  for (int i = 0; i < TERRAIN_WIDTH; ++i) {
    char current = terrain[i];
    char next = (i == TERRAIN_WIDTH-1) ? newTerrain : terrain[i+1];
    switch (current){
      case SPRITE_VAZIO:
        terrain[i] = (next == SPRITE_DOIS_INSETOS) ? SPRITE_TIRO_BAIXO : SPRITE_VAZIO;
        break;
      case SPRITE_DOIS_INSETOS:
        terrain[i] = (next == SPRITE_VAZIO) ? SPRITE_SPRITE_TIRO_ALTO : SPRITE_DOIS_INSETOS;
        break;
      case SPRITE_TIRO_BAIXO:
        terrain[i] = SPRITE_DOIS_INSETOS;
        break;
      case SPRITE_SPRITE_TIRO_ALTO:
        terrain[i] = SPRITE_VAZIO;
        break;
    }
  }
}

bool drawHero(byte position, char* terrainUpper, char* terrainLower, unsigned int score) {
  bool collide = false;
  char upperSave = terrainUpper[POS_AVIAO];
  char lowerSave = terrainLower[POS_AVIAO];
  byte upper, lower;
  switch (position) {
    case HERO_POSITION_OFF:
      upper = lower = SPRITE_VAZIO;
      break;
    case HERO_POSITION_RUN_LOWER_1:
      upper = SPRITE_VAZIO;
      lower = SPRITE_AVIAO_BAIXO;
      break;
    case HERO_POSITION_RUN_LOWER_2:
      upper = SPRITE_VAZIO;
      lower = SPRITE_AVIAO_ALTO;
      break;
    case HERO_POSITION_JUMP_1:
    case HERO_POSITION_JUMP_8:
      upper = SPRITE_VAZIO;
      lower = SPRITE_INSETO_BAIXO;
      break;
    case HERO_POSITION_JUMP_2:
    case HERO_POSITION_JUMP_7:
      upper = SPRITE_JUMP_UPPER;
      lower = SPRITE_INSETO_ALTO;
      break;
    case HERO_POSITION_JUMP_3:
    case HERO_POSITION_JUMP_4:
    case HERO_POSITION_JUMP_5:
    case HERO_POSITION_JUMP_6:
      upper = SPRITE_INSETO_BAIXO;
      lower = SPRITE_VAZIO;
      break;
    case HERO_POSITION_RUN_UPPER_1:
      upper = SPRITE_AVIAO_BAIXO;
      lower = SPRITE_VAZIO;
      break;
    case HERO_POSITION_RUN_UPPER_2:
      upper = SPRITE_AVIAO_ALTO;
      lower = SPRITE_VAZIO;
      break;
  }
  if (upper != ' ') {
    terrainUpper[POS_AVIAO] = upper;
    collide = (upperSave == SPRITE_VAZIO) ? false : true;
  }
  if (lower != ' ') {
    terrainLower[POS_AVIAO] = lower;
    collide |= (lowerSave == SPRITE_VAZIO) ? false : true;
  }
  
  byte digits = (score > 9999) ? 5 : (score > 999) ? 4 : (score > 99) ? 3 : (score > 9) ? 2 : 1;
  
  // Draw the scene
  terrainUpper[TERRAIN_WIDTH] = '\0';
  terrainLower[TERRAIN_WIDTH] = '\0';
  char temp = terrainUpper[16-digits];
  terrainUpper[16-digits] = '\0';
  lcd.setCursor(0,0);
  lcd.print(terrainUpper);
  terrainUpper[16-digits] = temp;  
  lcd.setCursor(0,1);
  lcd.print(terrainLower);
  
  lcd.setCursor(16 - digits,0);
  lcd.print(score);

  terrainUpper[POS_AVIAO] = upperSave;
  terrainLower[POS_AVIAO] = lowerSave;
  return collide;
}

// Handle the button push as an interrupt
void buttonPush() {
  buttonPushed = true;
}

void setup(){
  pinMode(PIN_READWRITE, OUTPUT);
  digitalWrite(PIN_READWRITE, LOW);
  pinMode(PIN_CONTRAST, OUTPUT);
  digitalWrite(PIN_CONTRAST, LOW);
  pinMode(PIN_BUTTON, INPUT);
  digitalWrite(PIN_BUTTON, HIGH);
  pinMode(PIN_AUTOPLAY, OUTPUT);
  digitalWrite(PIN_AUTOPLAY, HIGH);
  
  // Digital pin 2 maps to interrupt 0
  attachInterrupt(digitalPinToInterrupt(2), buttonPush, FALLING);
  
  initializeGraphics();
  
  lcd.begin(16, 2);
}

void loop(){
  static byte heroPos = HERO_POSITION_RUN_LOWER_1;
  static byte newTerrainType = VAZIO;
  static byte newTerrainDuration = 1;
  static bool playing = false;
  static bool blink = false;
  static unsigned int distance = 0;
  
  if (!playing) {
    drawHero((blink) ? HERO_POSITION_OFF : heroPos, terrainUpper, terrainLower, distance >> 3);
    if (blink) {
      lcd.setCursor(0,0);
      lcd.print("Press Start");
    }
    delay(250);
    blink = !blink;
    if (buttonPushed) {
      initializeGraphics();
      heroPos = HERO_POSITION_RUN_LOWER_1;
      playing = true;
      buttonPushed = false;
      distance = 0;
    }
    return;
  }

  // Shift the terrain to the left
  ProxFrame(terrainLower, newTerrainType == TERRAIN_LOWER_BLOCK ? SPRITE_DOIS_INSETOS : SPRITE_VAZIO);
  ProxFrame(terrainUpper, newTerrainType == TERRAIN_UPPER_BLOCK ? SPRITE_DOIS_INSETOS : SPRITE_VAZIO);
  
  // Make new terrain to enter on the right
  if (--newTerrainDuration == 0) {
    if (newTerrainType == VAZIO) {
      newTerrainType = (random(3) == 0) ? TERRAIN_UPPER_BLOCK : TERRAIN_LOWER_BLOCK;
      newTerrainDuration = 2 + random(10);
    } else {
      newTerrainType = VAZIO;
      newTerrainDuration = 10 + random(10);
    }
  }
    
  if (buttonPushed) {
    if (heroPos <= HERO_POSITION_RUN_LOWER_2) heroPos = HERO_POSITION_JUMP_1;
    buttonPushed = false;
  }  

  if (drawHero(heroPos, terrainUpper, terrainLower, distance >> 3)) {
    playing = false; // The hero collided with something. Too bad.
  } else {
    if (heroPos == HERO_POSITION_RUN_LOWER_2 || heroPos == HERO_POSITION_JUMP_8) {
      heroPos = HERO_POSITION_RUN_LOWER_1;
    } else if ((heroPos >= HERO_POSITION_JUMP_3 && heroPos <= HERO_POSITION_JUMP_5) && terrainLower[POS_AVIAO] != SPRITE_VAZIO) {
      heroPos = HERO_POSITION_RUN_UPPER_1;
    } else if (heroPos >= HERO_POSITION_RUN_UPPER_1 && terrainLower[POS_AVIAO] == SPRITE_VAZIO) {
      heroPos = HERO_POSITION_JUMP_5;
    } else if (heroPos == HERO_POSITION_RUN_UPPER_2) {
      heroPos = HERO_POSITION_RUN_UPPER_1;
    } else {
      ++heroPos;
    }
    ++distance;
    
    digitalWrite(PIN_AUTOPLAY, terrainLower[POS_AVIAO + 2] == SPRITE_VAZIO ? HIGH : LOW);
  }
  delay(50);
}
