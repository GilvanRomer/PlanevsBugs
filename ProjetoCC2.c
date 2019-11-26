#include <LiquidCrystal.h> //Importa biblioteca LiquidCrystal

#define PIN_BOTAO 2        //Porta do botao
#define PIN_READWRITE 10   //LCD R/W
#define PIN_CONTRASTE 12   //LCD V0

#define SPRITE_VAZIO ' '      //Usa o caracter ' ' como vazio
#define SPRITE_AVIAO_BAIXO 1  //Usa o caracter '1' como o aviao em baixo
#define SPRITE_AVIAO_ALTO 2   //Usa o caracter '2' como o aviao em cima 
#define SPRITE_INSETO_BAIXO 3 //Usa o caracter '3' como o inseto em baixo
#define SPRITE_INSETO_ALTO 4  //Usa o caracter '4' como o inseto em cima
#define SPRITE_DOIS_INSETOS 5 //Usa o caracter '5' como dois insetos
#define SPRITE_TIRO_BAIXO 6   //Usa o caracter '6' como um tiro em baixo
#define SPRITE_TIRO_ALTO 7    //Usa o caracter '7' como um tiro em cima

#define POS_AVIAO 0  //Posicao horizontal do aviao

#define NUM_CELULAS 16  //Numero horizontal total de celulas no lcd

#define AVIAO_POS_NULA 0  //Aviao esta invisivel

#define AVIAO_POSICAO_1 1  //Em baixo na celula de baixo
#define AVIAO_POSICAO_2 2  //Em cima na celula de baixo
#define AVIAO_POSICAO_3 3  //Em baixo na celula de cima
#define AVIAO_POSICAO_4 4  //Em cima na celula de cima

LiquidCrystal lcd(11, 9, 6, 5, 4, 3);                 //Instancia lcd
static char CelulaAlto[NUM_CELULAS + 1];              //Estados de todas as celulas de cima
static char CelulaBaixo[NUM_CELULAS + 1];             //Estados de todas as celulas de baixo
static char TiroSegundoPlano[2 * NUM_CELULAS];        //Estados de todas as celulas de cima
static bool botaoAtivado = false;                     //Estado do botao

void preparaCena(){         //Instancia a cena com todos os elementos no estado inicial
  static byte sprites[] = { //Armazena os sprites
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
  int i;                                       //Variavel contadora
  for (i = 0; i < 7; ++i) {                    //Itera sobre cada sprite em sprites[]
	  lcd.createChar(i + 1, &sprites[i * 8]);  //Cria 7 novos sprites
  }
  for (i = 0; i < NUM_CELULAS; ++i) {          //Itera horizontalmente sobre cada celula do lcd
    CelulaAlto[i] = SPRITE_VAZIO;              //Inicializa todas as celulas de cima com SPRITE_VAZIO
    CelulaBaixo[i] = SPRITE_VAZIO;             //Inicializa todas as celulas de baixo com SPRITE_VAZIO
	TiroSegundoPlano[i] = SPRITE_VAZIO;        //Inicializa todos os tiros de cima em segundo plano com SPRITE_VAZIO
	TiroSegundoPlano[i+16] = SPRITE_VAZIO;     //Inicializa todos os tiros de baixo em segundo plano com SPRITE_VAZIO
  }
}

static int frames = 0;                   //Variavel controla o fluxo na funcao ProxFrame
static byte aviaoPos = AVIAO_POSICAO_1;  //Instancia a posicao vertical inicial do aviao
static bool atirou = false;              //Sinalizador registra se o sistema de tiro foi ativado
static unsigned int pontos = 0;          //Contador de pontos

void ProxFrame(char* Celula, byte novaCelula, byte pos){                        //Calcula o proximo estado de cada celula
	byte tiro = NovoTiro(pos);                                                  //Cria tiro na posicao certa
	byte temp = SPRITE_VAZIO;                                                   //Variavel temporaria inicializada como ' '
	Celula[0] = SPRITE_VAZIO;                                                   //Apaga vestigios do ultimo estado
	Celula[15] = Celula[15] == 6 || Celula[15] == 7 ? SPRITE_VAZIO : Celula[15];//Apaga vestigios do ultimo estado
	int p = pos*16;                                                             //Sinaliza inicio do TiroSegundoPlano
	for (int i = 1; i < NUM_CELULAS; ++i) {                                     //Itera horizontalmente para cada celula do lcd
      char atual = (i == NUM_CELULAS-1) ? novaCelula : Celula[i];               //Armazena o valor atual da celula ou proximo inseto
	  char anterior = Celula[i-1];                                              //Armazena a celula anterior a atual
	  if (TiroSegundoPlano[i+p] != ' '){                                        //Caso haja um tiro em segundo plano
	    if (temp == SPRITE_VAZIO){                                              //Se temp estiver vazio
		  temp = TiroSegundoPlano[i+p];                                         //TiroSegundoPlano volta pro frame
		}
		else{                                                                   //Caso haja um tiro em temp
		  TiroSegundoPlano[i+p+1] = TiroSegundoPlano[i+p];                      //TiroSegundoPlano segue em diante
		}
	    TiroSegundoPlano[i+p] = SPRITE_VAZIO;                                   //Apaga o tiro em segundo plano
	  }
	  if (tiro != ' ' && atirou && i == 1){                                     //Se ha registro de tiro e um tipo de tiro
	    temp = tiro;                                                            //Armazena tiro
	    atirou = false;                                                         //Sinaliza nenhum tiro
      }
	  if (temp != SPRITE_VAZIO){                                                //Se houver um tiro armazenado em temp
	    if (!(atual == 6 || atual == 7 || atual == ' ')){                       //Se atual for um inseto
		  if ((temp == SPRITE_TIRO_BAIXO && atual == SPRITE_INSETO_BAIXO) ||
		     (temp == SPRITE_TIRO_ALTO && atual == SPRITE_INSETO_ALTO)){        //Se o tiro em temp estiver na mesma posicao que o inseto em atual
			Celula[i] = SPRITE_VAZIO;                                           //Inseto morre e o tiro some
		    ++pontos;                                                           //Incrementa os pontos
		  }
		  else if (temp == SPRITE_TIRO_BAIXO && atual == SPRITE_DOIS_INSETOS){  //Se o tiro em temp acertar dois insetos em baixo
		    if (anterior == ' '){                                               //Se anterior estiver vazio
			  Celula[i-1] = SPRITE_INSETO_ALTO;                                 //O inseto de cima prossegue
		      Celula[i] = SPRITE_VAZIO;                                         //O inseto de baixo morre e o tiro some
			}
			else if (anterior == SPRITE_TIRO_ALTO){                             //Se anterior tem um tiro em cima
			  Celula[i-1] = SPRITE_VAZIO;                                       //O inseto de cima prossegue e morre
		      Celula[i] = SPRITE_VAZIO;                                         //O inseto de baixo morre e o tiro some
			  ++pontos;                                                         //Incrementa os pontos por um inseto extra
			}
			else{                                                               //Se anterior tem um tiro em baixo
			  Celula[i-1] = SPRITE_INSETO_ALTO;                                 //O inseto de cima prossegue
			  TiroSegundoPlano[i+p] = anterior;                                 //Armazena tiro em segundo plano
		      Celula[i] = SPRITE_VAZIO;                                         //O inseto de baixo morre e o tiro some
			}
		    ++pontos;                                                           //Incrementa os pontos
		  }
		  else if (temp == SPRITE_TIRO_ALTO && atual == SPRITE_DOIS_INSETOS){   //Se o tiro em temp acertar dois insetos em cima 
		    if (anterior == ' '){                                               //Se anterior estiver vazio
			  Celula[i-1] = SPRITE_INSETO_BAIXO;                                //O inseto de baixo prossegue
		      Celula[i] = SPRITE_VAZIO;                                         //O inseto de cima morre e o tiro some
			}
			else if (anterior == SPRITE_TIRO_BAIXO){                            //Se anterior tem um tiro em baixo
			  Celula[i-1] = SPRITE_VAZIO;                                       //O inseto de baixo prossegue e morre
		      Celula[i] = SPRITE_VAZIO;                                         //O inseto de cima morre e o tiro some
			  ++pontos;                                                         //Incrementa os pontos por um inseto extra
			}
			else{                                                               //Se anterior tem um tiro em cima
			  Celula[i-1] = SPRITE_INSETO_BAIXO;                                //O inseto de baixo prossegue
			  TiroSegundoPlano[i+p] = anterior;                                 //Armazena tiro em segundo plano
		      Celula[i] = SPRITE_VAZIO;                                         //O inseto de cima morre e o tiro some
			}
		    ++pontos;                                                           //Incrementa os pontos
		  }
		  else{                                                                 //Se o tiro acertar nenhum inseto
		    if (anterior == SPRITE_VAZIO){                                      //Se anterior estiver vazio
	          Celula[i-1] = atual;                                              //O inseto prossegue
		      Celula[i] = temp;                                                 //O tiro prossegue
			}
			else if ((anterior == SPRITE_TIRO_BAIXO &&
			         atual == SPRITE_INSETO_BAIXO) ||
		            (anterior == SPRITE_TIRO_ALTO &&
					 atual == SPRITE_INSETO_ALTO)){                             //Se o tiro em anterior estiver na mesma posicao que o inseto em atual
			  Celula[i] = SPRITE_VAZIO;                                         //Inseto morre
			  Celula[i-1] = SPRITE_VAZIO;                                       //E o tiro some
			  TiroSegundoPlano[i+p+1] = temp;                                   //Armazena tiro em segundo plano
		      ++pontos;                                                         //Incrementa os pontos
		    }
			else{                                                               //Se o tiro em anterior nao acertar o inseto
			  Celula[i] = temp;                                                 //Tiro prossegue
			  Celula[i-1] = atual;                                              //Inseto prossegue
			  TiroSegundoPlano[i+p] = anterior;                                 //Armazena tiro em segundo plano
			}
		  }
		  temp = SPRITE_VAZIO;                                                  //Esvazia temp
		}
		else if (atual == ' '){                                                 //Se o atual estiver vazio
		  Celula[i] = temp;                                                     //Tiro prossegue
		  temp = SPRITE_VAZIO;                                                  //Esvazia temp
		}
		else {                                                                  //Se atual tambem e um tiro
		  Celula[i] = temp;                                                     //Tiro em temp prossegue
		  temp = atual;                                                         //Armazena tiro atual em temp
		}
	  }
	  else if (!(atual == 6 || atual == 7 || atual == ' ')){                    //Se atual for um inseto
	    if (anterior == ' '){                                                   //Se anterior estiver vazio
	      Celula[i-1] = atual;                                                  //Inseto prossegue
	      Celula[i] = SPRITE_VAZIO;                                             //Esvazia atual
	    }
	    else if ((anterior == SPRITE_TIRO_BAIXO &&
		         atual == SPRITE_INSETO_BAIXO) ||
		        (anterior == SPRITE_TIRO_ALTO &&
		         atual == SPRITE_INSETO_ALTO)){                                 //Se o tiro em anterior estiver na mesma posicao que o inseto em atual
	      Celula[i] = SPRITE_VAZIO;                                             //Inseto morre
	      Celula[i-1] = SPRITE_VAZIO;                                           //E o tiro some
	      ++pontos;                                                             //Incrementa os pontos
		}
	    else{                                                                   //Se o tiro em anterior nao acertar o inseto
	      Celula[i] = SPRITE_VAZIO;                                             //Esvazia atual
	      Celula[i-1] = atual;                                                  //Inseto prossegue
	      TiroSegundoPlano[i+p] = anterior;                                     //Armazena tiro em segundo plano
	    }
	  }
	  else if (atual != ' '){                                                   //Se atual for um tiro
		temp = atual;                                                           //Armazena o tiro em temp
		Celula[i] = SPRITE_VAZIO;                                               //Esvazia celula atual
	  }
	  frames = (i == NUM_CELULAS-1 && pos) ? 0 : frames;                        //Reseta contador de frames quando necessario
    }
}

bool mostraCena(byte pos, char* CelulaAlto, char* CelulaBaixo) {   //Imprime a cena no lcd e detecta colisao
  bool bateu = false;                                              //Sinalizador de colisao
  char altoSave = CelulaAlto[POS_AVIAO];                           //Salva valor inicial da celula de cima do aviao
  char baixoSave = CelulaBaixo[POS_AVIAO];                         //Salva valor inicial da celula de baixo do aviao
  switch (pos) {                                                   //Escolhe acao para cada estado de pos
    case AVIAO_POSICAO_1:
      CelulaBaixo[POS_AVIAO] = SPRITE_AVIAO_BAIXO;                 //Grava estado do aviao
	  bateu = ((baixoSave == SPRITE_INSETO_BAIXO) ||
	          (baixoSave == SPRITE_DOIS_INSETOS)) ? true : false;  //Detecta colisao com inseto
      break;
    case AVIAO_POSICAO_2:
      CelulaBaixo[POS_AVIAO] = SPRITE_AVIAO_ALTO;                  //Grava estado do aviao
	  bateu = ((baixoSave == SPRITE_INSETO_ALTO) ||
	          (baixoSave == SPRITE_DOIS_INSETOS)) ? true : false;  //Detecta colisao com inseto
      break;
    case AVIAO_POSICAO_3:
      CelulaAlto[POS_AVIAO] = SPRITE_AVIAO_BAIXO;                  //Grava estado do aviao
	  bateu = ((altoSave == SPRITE_INSETO_BAIXO) ||
	          (altoSave == SPRITE_DOIS_INSETOS)) ? true : false;   //Detecta colisao com inseto
      break;
    case AVIAO_POSICAO_4:
      CelulaAlto[POS_AVIAO] = SPRITE_AVIAO_ALTO;                   //Grava estado do aviao
	  bateu = ((altoSave == SPRITE_INSETO_ALTO) ||
	          (altoSave == SPRITE_DOIS_INSETOS)) ? true : false;   //Detecta colisao com inseto
      break;
  }
  
  byte digits = (pontos > 9999) ? 5 : (pontos > 999) ? 4 : (pontos > 99) ? 3 : (pontos > 9) ? 2 : 1;  //Registra numero de digitos da pontuacao
  
  CelulaAlto[NUM_CELULAS] = '\0';    //Insere caractere nulo ao final do vetor. Completa uma string e agiliza o processamento
  CelulaBaixo[NUM_CELULAS] = '\0';   //Insere caractere nulo ao final do vetor.
  char aux = CelulaAlto[16-digits];  //Armazena sprite na posicao inicial dos pontos
  CelulaAlto[16-digits] = '\0';      //Apaga posicao inicial dos pontos
  lcd.setCursor(0,0);                //Posiciona o cursor no canto superior esquerdo do lcd
  lcd.print(CelulaAlto);             //Imprime conteudo superior do ProxFrame
  CelulaAlto[16-digits] = aux;       //Reinsere sprite salvo na sua posicao original
  lcd.setCursor(0,1);                //Posiciona o cursor no canto inferior esquerdo do lcd
  lcd.print(CelulaBaixo);            //Imprime conteudo inferior do ProxFrame
  
  lcd.setCursor(16 - digits,0);  //Posiciona cursor na posicao inicial dos pontos
  lcd.print(pontos);             //Imprime pontuacao

  CelulaAlto[POS_AVIAO] = altoSave;    //Restaura primeira celula a seu estado original
  CelulaBaixo[POS_AVIAO] = baixoSave;  //Restaura primeira celula a seu estado original
  return bateu;                        //Retorna sinalizador de colisao
}

byte NovoTiro(byte pos){                                      //Gera um tiro na posicao correta
	if (aviaoPos == 1 && !pos) return SPRITE_TIRO_BAIXO;      //Retorna tiro caso posicao esteja correta
	else if (aviaoPos == 3 && pos) return SPRITE_TIRO_BAIXO;  //Retorna tiro caso posicao esteja correta
	else if (aviaoPos == 2 && !pos) return SPRITE_TIRO_ALTO;  //Retorna tiro caso posicao esteja correta
	else if (aviaoPos == 4 && pos) return SPRITE_TIRO_ALTO;   //Retorna tiro caso posicao esteja correta
	else return SPRITE_VAZIO;                                 //Retorna vazio caso nao haja posicao correta
}

static byte novoTipoInseto = SPRITE_VAZIO;  //Armazena um novo inseto
static byte distNovoInseto = 1;             //Armazena a distancia cumulativa dos insetos

byte CriarInseto(byte dist){                   //Falta Completar
	if (--distNovoInseto == 0) {               //Decrementa distancia para o proximo inseto e checa se e 0
	  int r = random(100);                     //Armazena um numero aleatorio de 0~99
	  distNovoInseto = dist;                   //Dist controla a distancia entre os insetos
	  if (r < 10){                             //Com 10% de chance
	    return SPRITE_DOIS_INSETOS;            //Retorna dois insetos
	  }else if (r < 45){                       //Com 45% de chance
	    return SPRITE_INSETO_ALTO;             //Retorna um inseto alto
	  }else{                                   //Com 45% de chance
	    return SPRITE_INSETO_BAIXO;            //Retorna um inseto baixo
	  }
    }else{                                     //Se a distancia ainda nao e 0
	  return SPRITE_VAZIO;                     //Retorna um sprite vazio
	 }
}

void ativaBotao() {     //Interrupt para ativar o botao
  botaoAtivado = true;  //Atribui booleano verdadeiro para o sinalizador do botao
}

void setup(){                                                      //Setup
  pinMode(PIN_READWRITE, OUTPUT);                                  //Define PIN_READWRITE como output
  digitalWrite(PIN_READWRITE, LOW);                                //Define estado inicial do PIN_READWRITE como LOW
  pinMode(PIN_CONTRASTE, OUTPUT);                                  //Define PIN_CONTRASTE como output
  digitalWrite(PIN_CONTRASTE, LOW);                                //Define estado inicial do PIN_CONTRASTE como LOW
  pinMode(PIN_BOTAO, INPUT);                                       //Define PIN_BOTAO como input
  digitalWrite(PIN_BOTAO, HIGH);                                   //Define estado inicial do PIN_BOTAO como HIGH
  
  attachInterrupt(digitalPinToInterrupt(2), ativaBotao, FALLING);  //Pino digital 2 e mapeado para o interrupt 0
  
  preparaCena();                                                   //Instancia os graficos
  
  lcd.begin(16, 2);                                                //Inicializa o lcd
  
  Serial.begin(9600);                                              //Inicializa o serial para testes
}

void loop(){                                                                          //Rotina principal
  static bool jogando = false;                                                        //Armazena estado da reproducao do jogo
  static bool pisca = false;                                                          //Sinalizador de pisca-pisca
  static int veloc = 4;                                                               //Controle de fluxo para o fps
  static int f = 0;                                                                   //Controle de fluxo para a movimentacao do aviao
  int dist = 5;                                                                       //Controla a distancia que cada inseto aparece
  
  if (!jogando) {                                                                     //Se jogo nao etiver rodando
    mostraCena((pisca) ? AVIAO_POS_NULA : aviaoPos, CelulaAlto, CelulaBaixo);         //Pisca cena no estado atual
    if (pisca) {                                                                      //Se for pra piscar
      lcd.setCursor(0,0);                                                             //Posiciona cursor no canto superior esquerdo do lcd
      lcd.print("Press Start");                                                       //Escreve no lcd
    }
    delay(250);                                                                       //Delay de 250ms ate piscar de novo
    pisca = !pisca;                                                                   //Inverte o estado de pisca-pisca
    if (botaoAtivado) {                                                               //Se botao foi precionado
      preparaCena();                                                                  //Reinicializa o lcd para seu estado inicial
      aviaoPos = AVIAO_POSICAO_1;                                                     //Reinicializa a posicao do aviao para seu estado inicial
      jogando = true;                                                                 //Atualiza estado de reproducao do jogo
      botaoAtivado = false;                                                           //Desativa sinalizador de ativacao do botao
      pontos = 0;                                                                     //Reseta pontuacao
    }
    return;                                                                           //Enquanto estado de reproducao for false, nao executa os proximos blocos de codigo
  }
  
  if (frames > veloc){                                                                  //Atualiza lcd condicionado a veloc
	veloc = (pontos > 150) ? 2 : (pontos > 75) ? 3 : (pontos > 15) ? 4 : 5;  			//Atualiza veloc de acordo com a pontuacao
	dist =  (pontos > 250) ? 3 : (pontos > 40) ? 4 : 5;  								//Atualiza dist de acordo com a pontuacao
    byte novoInseto = CriarInseto(dist);                                                //Cria, ou nao, um novo inseto
    byte proxInsetoBaixo = (random(2) == 0) ? novoInseto : SPRITE_VAZIO;                //Atribui o novo inseto para a celula inferior com 50% de chance
    byte proxInsetoAlto = (proxInsetoBaixo == novoInseto) ? SPRITE_VAZIO : novoInseto;  //Se inseto nao foi atribuido a celula inferior, atribua a celula superior
  
    ProxFrame(CelulaBaixo, proxInsetoBaixo, 0);  //Avanca o estado das celulas superiores do lcd
    ProxFrame(CelulaAlto, proxInsetoAlto, 1);    //Avanca o estado das celulas inferiores do lcd
  }  
    
  if (botaoAtivado) {                 //Se o botao foi precionado
    if (aviaoPos < AVIAO_POSICAO_4){  //Se o aviao nao estiver no topo da tela do lcd
	  ++aviaoPos;                     //faz o aviao subir um nivel
      f = 0;                          //Reseta controlador de fluxo
	  atirou = true;                  //Atira sempre que o aviao subir
    }
	botaoAtivado = false;             //Desativa sinalizador de botao ativado
  }  

  if (mostraCena(aviaoPos, CelulaAlto, CelulaBaixo)) {  //Mostra a cena e detecta colisao
    jogando = false;                                    //Game over
  } else {                                              //Se aviao nao colidiu
    if (aviaoPos > AVIAO_POSICAO_1 && f == 8) {         //Se aviao nao esta no fundo e controlador de fluxo == veloc
      --aviaoPos;                                       //Faz o aviao descer um nivel
      f = 0;                                            //Reseta controlador de fluxo
	  atirou = true;                                    //Atira sempre que o aviao descer
    }
    else if (aviaoPos != AVIAO_POSICAO_1) f++;          //Incrementa o controlador de fluxo enquanto o aviao nao estiver parado no fundo da tela do lcd
  }
  delay(50);                                            //Espera 50ms para o proximo frame
  ++frames;                                             //Contador de fps
}
