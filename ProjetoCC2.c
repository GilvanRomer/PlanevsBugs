#include <LiquidCrystal.h> //Import LiquidCrystal Library

#define PIN_BUTTON 2 //Play button port
#define PIN_READWRITE 10 //LCD R/W
#define PIN_CONTRAST 12 //LCD V0

#define SPRITE_VAZIO ' '      // User the ' ' character
#define SPRITE_AVIAO_BAIXO 1
#define SPRITE_AVIAO_ALTO 2
#define SPRITE_INSETO_BAIXO 3
#define SPRITE_INSETO_ALTO 4
#define SPRITE_DOIS_INSETOS 5
#define SPRITE_TIRO_BAIXO 6
#define SPRITE_TIRO_ALTO 7

#define POS_AVIAO 0    // Horizontal pos of hero on screen

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

static int frames = 0;
static byte aviaoPos = AVIAO_POSICAO_1;
static bool atirou = false;
static unsigned int pontos = 0;

void ProxFrame(char* Celula, byte novaCelula, int vel, byte pos){ //vel controla velocidade dos insetos
  if (frames > vel*2){
	byte tiro = NovoTiro(pos);
	static byte temp = SPRITE_VAZIO;
	Celula[0] = SPRITE_VAZIO;
	for (int i = 1; i < NUM_CELULAS; ++i) {
      char atual = (i == NUM_CELULAS-1) ? novaCelula : Celula[i];
	  char aterior = Celula[i-1];
	  if (temp != SPRITE_VAZIO){
	    if (!(atual == 6 || atual == 7 || atual == ' ')){
		  if ((temp == SPRITE_TIRO_BAIXO && atual == SPRITE_INSETO_BAIXO) || (temp == SPRITE_TIRO_ALTO && atual == SPRITE_INSETO_ALTO)){
			Celula[i] = SPRITE_VAZIO;
		    ++pontos;
		  }
		  else if (temp == SPRITE_TIRO_BAIXO && atual == SPRITE_DOIS_INSETOS){
		    Celula[i-1] = SPRITE_INSETO_ALTO;
		    Celula[i] = SPRITE_VAZIO;
		    ++pontos;
		  }
		  else if (temp == SPRITE_TIRO_ALTO && atual == SPRITE_DOIS_INSETOS){
		    Celula[i-1] = SPRITE_INSETO_BAIXO;
		    Celula[i] = SPRITE_VAZIO;
		    ++pontos;
		  }
		  else{
	        Celula[i-1] = atual;
		    Celula[i] = temp;
		  }
		  temp = SPRITE_VAZIO;
		}
		else if (atual == ' '){
		  Celula[i] = temp;
		  temp = SPRITE_VAZIO;
		}
		else {
		  Celula[i] = temp;
		  temp = atual;
		}
	  }
	  else if (!(atual == 6 || atual == 7 || atual == ' ')){
	    Celula[i-1] = atual;
		Celula[i] = SPRITE_VAZIO;
	  }
	  else if (atual != ' '){
		temp = atual;
		Celula[i] = SPRITE_VAZIO;
	  }
	  frames = (i == NUM_CELULAS-1 && pos) ? 0 : frames; // Reseta frames quando necessario
	if (tiro != ' ' && atirou){ // Atira e mata oponente logo a frente se atirou == true
	  if (Celula[1] == ' '){
	    Celula[1] = tiro;
	  }
	  else{
	    Celula[1] = SPRITE_VAZIO;
		++pontos;
	  }
	  atirou = false;
    }
    }
  }
  ++frames;
}

bool drawHero(byte pos, char* CelulaAlto, char* CelulaBaixo) {
  bool bateu = false;
  char altoSave = CelulaAlto[POS_AVIAO];
  char baixoSave = CelulaBaixo[POS_AVIAO];
  switch (pos) {
    case POS_AVIAO_NULO:
      break;
    case AVIAO_POSICAO_1:
      CelulaBaixo[POS_AVIAO] = SPRITE_AVIAO_BAIXO;
	  bateu = ((baixoSave == SPRITE_INSETO_BAIXO) || (baixoSave == SPRITE_DOIS_INSETOS)) ? true : false;
      break;
    case AVIAO_POSICAO_2:
      CelulaBaixo[POS_AVIAO] = SPRITE_AVIAO_ALTO;
	  bateu = ((baixoSave == SPRITE_INSETO_ALTO) || (baixoSave == SPRITE_DOIS_INSETOS)) ? true : false;
      break;
    case AVIAO_POSICAO_3:
      CelulaAlto[POS_AVIAO] = SPRITE_AVIAO_BAIXO;
	  bateu = ((altoSave == SPRITE_INSETO_BAIXO) || (altoSave == SPRITE_DOIS_INSETOS)) ? true : false;
      break;
    case AVIAO_POSICAO_4:
      CelulaAlto[POS_AVIAO] = SPRITE_AVIAO_ALTO;
	  bateu = ((altoSave == SPRITE_INSETO_ALTO) || (altoSave == SPRITE_DOIS_INSETOS)) ? true : false;
      break;
  }
  
  byte digits = (pontos > 9999) ? 5 : (pontos > 999) ? 4 : (pontos > 99) ? 3 : (pontos > 9) ? 2 : 1;
  
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
  lcd.print(pontos);

  CelulaAlto[POS_AVIAO] = altoSave;
  CelulaBaixo[POS_AVIAO] = baixoSave;
  return bateu;
}

byte NovoTiro(byte pos){
	if (aviaoPos == 1 && !pos) return SPRITE_TIRO_BAIXO;
	else if (aviaoPos == 3 && pos) return SPRITE_TIRO_BAIXO;
	else if (aviaoPos == 2 && !pos) return SPRITE_TIRO_ALTO;
	else if (aviaoPos == 4 && pos) return SPRITE_TIRO_ALTO;
	else return SPRITE_VAZIO;
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
  static int f = 0;
  
  if (!playing) {
    drawHero((blink) ? POS_AVIAO_NULO : aviaoPos, CelulaAlto, CelulaBaixo);
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
      pontos = 0;
    }
    return;
  }

  // Shift the Celula to the left
  ProxFrame(CelulaBaixo, CriarInseto(5), 5, 0);//Falta Completar
  ProxFrame(CelulaAlto, CriarInseto(5), 5, 1);//Falta Completar
    
  if (buttonPushed) {
    if (aviaoPos < AVIAO_POSICAO_4){
	  ++aviaoPos;
      f = 0;
	  atirou = true;
    }
	buttonPushed = false;
  }  

  if (drawHero(aviaoPos, CelulaAlto, CelulaBaixo)) {
    playing = false; // The hero bateud with something. Too bad.
  } else {
    if (aviaoPos > AVIAO_POSICAO_1 && f == 7) {
      --aviaoPos;
      f = 0;
	  atirou = true;
    }
    else if (aviaoPos != AVIAO_POSICAO_1) f++;
  }
  delay(50);
}
