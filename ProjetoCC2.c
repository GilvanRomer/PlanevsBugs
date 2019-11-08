#include <LiquidCrystal.h> //Import LiquidCrystal Library

#define PIN_BUTTON 2 //Play button port
#define PIN_READWRITE 10 //LCD R/W
#define PIN_CONTRAST 12 //LCD V0

#define SPRITE_AVIAO_BAIXO 1
#define SPRITE_AVIAO_ALTO 2
#define SPRITE_INSETO_BAIXO 3
#define SPRITE_JUMP_alto '.'         // Use the '.' character for the head
#define SPRITE_INSETO_ALTO 4
#define SPRITE_VAZIO ' '      // User the ' ' character
#define SPRITE_DOIS_INSETOS 5
#define SPRITE_TIRO_BAIXO 6
#define SPRITE_TIRO_ALTO 7

#define POS_AVIAO 0    // Horizontal position of hero on screen

#define NUM_CELULAS 16
#define VAZIO 0
#define INSETO_BAIXO_1 1
#define INSETO_BAIXO_2 1
#define INSETO_ALTO_1 2
#define INSETO_ALTO_2 2
#define DOIS_INSETOS_1 3
#define DOIS_INSETOS_2 3

#define POS_AVIAO_NULO 0          // Hero is invisible

#define AVIAO_POSICAO_1 1       // Starting a jump
#define AVIAO_POSICAO_2 2       // Half-way up
#define AVIAO_POSICAO_3 3       // Jump is on alto row
#define AVIAO_POSICAO_4 4       // Jump is on alto row

#define HERO_POSITION_RUN_alto_1 11 // Hero is running on alto row (pose 1)
#define HERO_POSITION_RUN_alto_2 12 //                              (pose 2)

LiquidCrystal lcd(11, 9, 6, 5, 4, 3); //Initialize lcd
static char CelulaAlto[NUM_CELULAS + 1];
static char CelulaBaixo[NUM_CELULAS + 1];
static bool buttonPushed = false;

void initializeGraphics(){
  static byte graphics[] = { //Store new lcd characters
    // Avião baixo
    B00000,
    B00000,
    B00000,
    B00000,
    B10100,
    B11111,
    B00100,
    B00000,
    // Avião alto
    B00000,
    B10100,
    B11111,
    B00100,
    B00000,
    B00000,
    B00000,
    B00000,
    // Inseto baixo
    B00000,
    B00000,
    B00000,
    B00000,
    B10101,
    B01111,
    B10101,
    B00000,
    // Inseto alto
    B00000,
    B10101,
    B01111,
    B10101,
    B00000,
    B00000,
    B00000,
    B00000,
    // Dois insetos
    B00000,
    B10101,
    B01111,
    B10101,
    B00000,
    B10101,
    B01111,
    B10101,
    // Tiro baixo
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00100,
    B00000,
    B00000,
    // Tiro alto
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
  for (i = 0; i < NUM_CELULAS; ++i) {
    CelulaAlto[i] = SPRITE_VAZIO;
    CelulaBaixo[i] = SPRITE_VAZIO;
  }
}

// Slide the Celula to the left in half-character increments
//
static int frames = 0;
void ProxFrame(char* Celula, byte novaCelula, byte novoTiro, int vel, byte pos){ //vel controla velocidade dos insetos
  for (int i = 0; i < NUM_CELULAS; ++i) {
    char atual = Celula[i];
    char prox = (i == NUM_CELULAS-1) ? novaCelula : Celula[i+1];
	char tiro = ' ';
	if (frames > vel*2){
	  if (atual > 5 && prox == ' '){ //Atual e um tiro e prox e vazio
		tiro = atual;
	    Celula[i] = SPRITE_VAZIO;
	  }
	  else if(tiro != ' '){ //caso haja um tiro armazenado
	    Celula[i] = tiro;
		tiro = ' ';
	  }
	  else if(atual < 6 || atual == ' '){ //Atual nao e um tiro
	    Celula[i] = (prox < 6 || prox == ' ') ? prox : SPRITE_VAZIO;
	  }
	  else if(atual == ' '){
	    Celula[i] = (prox < 6 || prox == ' ') ? prox : SPRITE_VAZIO;
	  }
		Celula[i] = prox;
	    frames = (i == NUM_CELULAS-1 && pos) ? 0 : frames;
    }
  }
  ++frames;
}

bool drawHero(byte position, char* CelulaAlto, char* CelulaBaixo, unsigned int score) {
  bool collide = false;
  char altoSave = CelulaAlto[POS_AVIAO];
  char baixoSave = CelulaBaixo[POS_AVIAO];
  byte alto, baixo;
  switch (position) {
    case POS_AVIAO_NULO:
      alto = baixo = SPRITE_VAZIO;
      break;
    case AVIAO_POSICAO_1:
      alto = SPRITE_VAZIO;
      baixo = SPRITE_AVIAO_BAIXO;
      break;
    case AVIAO_POSICAO_2:
      alto = SPRITE_VAZIO;
      baixo = SPRITE_AVIAO_ALTO;
      break;
    case AVIAO_POSICAO_3:
      alto = SPRITE_AVIAO_BAIXO;
      baixo = SPRITE_VAZIO;
      break;
    case AVIAO_POSICAO_4:
      alto = SPRITE_AVIAO_ALTO;
      baixo = SPRITE_VAZIO;
      break;
  }
  if (alto != ' ') {
    CelulaAlto[POS_AVIAO] = alto;
    collide = (altoSave == SPRITE_VAZIO) ? false : true;
  }
  if (baixo != ' ') {
    CelulaBaixo[POS_AVIAO] = baixo;
    collide |= (baixoSave == SPRITE_VAZIO) ? false : true;
  }
  
  byte digits = (score > 9999) ? 5 : (score > 999) ? 4 : (score > 99) ? 3 : (score > 9) ? 2 : 1;
  
  // Draw the scene
  CelulaAlto[NUM_CELULAS] = '\0';
  CelulaBaixo[NUM_CELULAS] = '\0';
  char temp = CelulaAlto[16-digits];
  CelulaAlto[16-digits] = '\0';
  lcd.setCursor(0,0);
  lcd.print(CelulaAlto);
  CelulaAlto[16-digits] = temp;  
  lcd.setCursor(0,1);
  lcd.print(CelulaBaixo);
  
  lcd.setCursor(16 - digits,0);
  lcd.print(score);

  CelulaAlto[POS_AVIAO] = altoSave;
  CelulaBaixo[POS_AVIAO] = baixoSave;
  return false;//collide;
}

static byte aviaoPos = AVIAO_POSICAO_1;
static bool atirou = false;

byte NovoTiro(byte pos){
    byte tiro;
	if (atirou){
	  if (aviaoPos == 1 && pos) tiro = SPRITE_TIRO_BAIXO;
	  else if (aviaoPos == 3 && !pos) tiro = SPRITE_TIRO_BAIXO;
	  else if (aviaoPos == 2 && pos) tiro = SPRITE_TIRO_ALTO;
	  else if (aviaoPos == 4 && !pos) tiro = SPRITE_TIRO_ALTO;
	  atirou = false;
	  return tiro;
	}else return SPRITE_VAZIO;
}

static byte novoTipoCelula = SPRITE_VAZIO;
static byte distNovaCelula = 1;

byte CriarInseto(byte dist){ //Falta Completar
	if (--distNovaCelula == 0) {
	  int r = random(100);
	  if (r < 10){
	    novoTipoCelula = (random(2)==0) ? SPRITE_DOIS_INSETOS: SPRITE_DOIS_INSETOS;
	  }else if (r < 55){
	    novoTipoCelula = (random(2)==0) ? SPRITE_INSETO_ALTO: SPRITE_INSETO_ALTO;
	  }else{
	    novoTipoCelula = (random(2)==0) ? SPRITE_INSETO_BAIXO: SPRITE_INSETO_BAIXO;
	  }
	  distNovaCelula = dist; //Dist controla a distancia entre os insetos
    }else{
	  novoTipoCelula = SPRITE_VAZIO;
	 }
  return novoTipoCelula;
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
  
  // Digital pin 2 maps to interrupt 0
  attachInterrupt(digitalPinToInterrupt(2), buttonPush, FALLING);
  
  initializeGraphics();
  
  lcd.begin(16, 2);
  
  Serial.begin(9600);
}

void loop(){
  static bool playing = false;
  static bool blink = false;
  static unsigned int distance = 0;
  static int f = 0;
  
  if (!playing) {
    drawHero((blink) ? POS_AVIAO_NULO : aviaoPos, CelulaAlto, CelulaBaixo, distance >> 3);
    if (blink) {
      lcd.setCursor(0,0);
      lcd.print("Press Start");
    }
    delay(250);
    blink = !blink;
    if (buttonPushed) {
      initializeGraphics();
      aviaoPos = AVIAO_POSICAO_1;
      playing = true;
      buttonPushed = false;
      distance = 0;
    }
    return;
  }

  // Shift the Celula to the left
  ProxFrame(CelulaBaixo, CriarInseto(5), NovoTiro(0), 5, 0);//Falta Completar
  ProxFrame(CelulaAlto, CriarInseto(5), NovoTiro(1), 5, 1);//Falta Completar
    
  if (buttonPushed) {
    if (aviaoPos < AVIAO_POSICAO_4){
	  ++aviaoPos;
      f = 0;
	  atirou = true;
    }
	buttonPushed = false;
  }  

  if (drawHero(aviaoPos, CelulaAlto, CelulaBaixo, distance >> 3)) {
    playing = false; // The hero collided with something. Too bad.
  } else {
    if (aviaoPos > AVIAO_POSICAO_1 && f == 7) {
      --aviaoPos;
      f = 0;
	  atirou = true;
    }
    else if (aviaoPos != AVIAO_POSICAO_1) f++;
    ++distance;
    
  }
  delay(50);
}
