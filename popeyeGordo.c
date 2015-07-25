#include <stdio.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mixer.h>
#include <X11/Xlib.h>

#define FPS                 60
#define DECLIVE             0.02967931
#define FORCA_GRAVIDADE     1
#define FORCA_PULO          4.5
#define VELOC_ESCADA        2
#define VELOCX_PERSONAGEM   2
#define VELOCX_HAMBURGER    2.5
#define DX_PERSONAGEM       60
#define DY_PERSONAGEM       520
#define DX_HAMBURGER        50
#define DY_HAMBURGER        24
#define QNT_VIDAS           3

// Flags usadas no controle das frames
enum Flags{
   PULO_DIREITA,
   PULO_ESQUERDA,
   ESCADA,
   CORRENDO,
   PARADO,
   CORRENDO_DIREITA,
   CORRENDO_ESQUERDA,
   PARADO_DIREITA,
   PARADO_ESQUERDA,
   MORTE,
   HAMBURGER,
   ESPINAFRE,
   DUDU_PARADO,
   DUDU_JG_HAMBURGER,
   JG_HAMBURGER
};
/*==============================================================================================================================================================================
=================================VARIÁVEIS GLOBAIS==============================================================================================================================
================================================================================================================================================================================*/

SDL_Event     event;
SDL_Color     color[]               = {{255,255,255}, {255,0,0}};
Mix_Music   * bgTrack               = NULL;
Mix_Chunk   * fxPulo                = NULL;
Mix_Chunk   * fxPonto               = NULL;
Mix_Chunk   * fxMorte               = NULL;
Mix_Chunk   * fxMutacao             = NULL;
Mix_Chunk   * fxBurger              = NULL;
SDL_Thread  * thread[16]            = { NULL };
SDL_Surface * menu                  = NULL; 
SDL_Surface * recordes              = NULL;
SDL_Surface * creditos              = NULL;
SDL_Surface * mododejogo            = NULL;
SDL_Surface * historia              = NULL;
SDL_Surface * comandos              = NULL;
SDL_Surface * imagemTela            = NULL;
SDL_Surface * ship                  = NULL;
SDL_Surface * rotShip[41]           = { NULL }; // vetor com 41 surfaces, cada uma com a surface 'ship' com rotação de -2º a +2º, variando em 0.1º
SDL_Surface * fundo                 = NULL; 
SDL_Surface * plat                  = NULL; // Imagem das plataformas
SDL_Surface * screen                = NULL;
SDL_Surface * scoreWordImage        = NULL;
SDL_Surface * scorePointsImage      = NULL;
SDL_Surface * highScoreWordImage    = NULL;
SDL_Surface * highScorePointsImage  = NULL;
SDL_Surface * bonusScoreWordImage   = NULL;
SDL_Surface * bonusScorePointsImage = NULL;
SDL_Surface * livesImage            = NULL; // Imagem das vidas do personagem
SDL_Surface * aux                   = NULL; // Surface que guardará uma imagem temporariamente se uma troca de imagem for solicitada por outra surface
SDL_Surface * highScoreImage        = NULL;
SDL_Surface * newHighScoreImage     = NULL;
SDL_Surface * youWin                = NULL;
SDL_Surface * gameOver              = NULL;
TTF_Font    * accIngameScore;          //acc = atari classic chunky font
TTF_Font    * accHighScore;
SDL_sem     * threadLock;

int start                = 0,  // Para contar o tempo e controlar FPS
    jogoRodando          = 0,
    teclaEsquerda        = 0,
    teclaDireita         = 0,
    teclaCima            = 0,
    teclaBaixo           = 0,
    teclaEspaco          = 0,
    intervaloHamburger   = 15, // Para o controle da taxa de emissão de hamburgers ( inicialize com números >= 15 ou <= -1, para não começar o jogo já jogando um hamburger )
    burgerDaVez          = 0,  // Índice do próximo hamburger a ser jogado ( vide jogarHamburger() )
    frameShip            = 0,  // Índice da próxima imagem do barco rotacionado a ser blitada
    returnThreadValue    = 0,  // Guarda o valor retornado por uma thread (não usado ainda)
    pontos               = 0,  // Variável que vai guardar a pontuação do jogador
    newHighScore         = 0,
    shiftOn              = 0,  // Variável que indicará se a tecla Shift foi pressionada
    a                    = 0,  // Incremento ou decremento do índice de imagem do barco rotacionado
    v                    = -1, // Incremento ou decremento da posição Y do barco rotacionado
    volumeMusica         = 30,
    pause                = 0,
    hulkTimer            = 0,
    bonusTimer           = 0,
    bonusScoreIntPoints  = 0,
    done                 = 0,
    saudavel             = 0,
    gorduroso            = 0,
    venceu               = 0,
    noMenu               = 0,
    duduJogandoHamburger = 0;

double plataforma0 = 520,
       plataforma1,
       plataforma2,
       plataforma3,
       plataforma4,
       plataforma5,
       freqHamburger = 1;

char highScoreFileDataName[5][11]; // Vetor de strings que guardará o NOME  do TOP5 do high score
char highScoreFileDataPoints[5][10]; // Vetor de strings que guardará os PONTOS do TOP5 do high score
int highScoreIntPoints[5] = { 0, 0, 0, 0, 0 }; // Vetor int que guardará os pontos do vetor acima como int

SDL_Rect mouse              = ( SDL_Rect ) {   0,   0,   0,   0 };
// Rect da Source do barco       
SDL_Rect rSrcShip           = ( SDL_Rect ) {   0,  85, 992, 768 };
// Rect do Destino do barco
SDL_Rect rDstShip           = ( SDL_Rect ) { 125,   0, 992, 768 };
// Rect do Destino da string 'Score: '
SDL_Rect rScoreWord         = ( SDL_Rect ) {  30, 585,   0,   0 };
// Rect do Destino dos pontos do personagem (ficará ao lado da palavra score)
SDL_Rect rScorePoints       = ( SDL_Rect ) { 105, 585,   0,   0 };
// Rect da Source da imagem das vidas do personagem
SDL_Rect rSrcLivesImage     = ( SDL_Rect ) {   0,   0,  54,  13 };
// Rect do Destino da imagem das vidas do personagem
SDL_Rect rDstLivesImage     = ( SDL_Rect ) { 730, 585,   0,   0 };
// Rect do Destino da string 'High Score:'
SDL_Rect rHighScoreWord     = ( SDL_Rect ) { 175, 585,   0,   0 };
// Rect do Destino da maior pontuação
SDL_Rect rHighScorePoints   = ( SDL_Rect ) { 311, 585,   0,   0 };
SDL_Rect rBonusScoreWord    = ( SDL_Rect ) { 400, 585,   0,   0 };
SDL_Rect rBonusScorePoints  = ( SDL_Rect ) { 475, 585,   0,   0 };
// Rects dos Destinos dos nomes dos TOP5 do high score
SDL_Rect rHighScoreName[5]   = {{ 120, 156, 0, 0 }, {  120, 236, 0, 0 }, { 120, 316, 0, 0 },
                                { 120, 396, 0, 0 }, {  120, 476, 0, 0 } };
// Rects dos Destinos das pontuações dos TOP5 do high score
SDL_Rect rHighScoreValues[5] = {{ 524, 146, 0, 0 }, {  524, 236, 0, 0 }, { 524, 316, 0, 0 },
                                { 524, 396, 0, 0 }, {  524, 476, 0, 0 } };      

// Struct que definirá os hamburgers do jogo e também o player
typedef struct
{
   // sX e sY são a posição da fonte (source); dX e dY são a posição destino; velX e velY são as velocidades horizontais (eixo X) e verticais (eixo Y),
   // velEscada é autoexplicativo, chao guarda a informação de onde o objeto está pisando
   float sX, sY, dX, dY, velX, velY, velEscada, chao, hulk;
   
   // ativo indica se o hamburger está ativo ( SEMPRE 0 para o player ) e rota indica qual percurso o hamburger seguirá e 
   // escadagrande é o estado que indica se o hamburger ou player está na escada especial e escorregadia grande
   // regulaFrames vai controlar a frquência para troca de cada frame, mesmo a 60 FPS. morto indica se o player morreu. pontua indica se o hamburger ainda dar pontos. vidas é a quantidade de vidas do player
   int largura, altura, pulo, direita, caindo, naEscada, escadagrande, ativo, rota, plataforma, regulaFrames, morto, pontua, vidas; 

   // cada objeto tem sua própria surface de imagem. a surface aux vai auxiliar na troca de surfaces de cada objeto.imagem   
   SDL_Surface * image, * aux;

}Object;


//                                                           DESATIVE A QUEBRA DE LINHA PARA MELHOR VISUALIZAÇÃO
//------------------------------------------------------------- Criação e inicialização do player e hamburgers--------------------------------------------------------------------- ------------------------------------------------------------------------------------------

                               // sX sY         dX             dY               velX            velY          velEscada    chao  hulk largura altura pulo direita caindo naEscada escadagrande ativo rota plataforma regulaFrames  morto pontua    vidas     image  aux
Object dudu        =  ( Object ) { 0, 0, DX_HAMBURGER-30, DY_HAMBURGER,           0,              0,              0,        5,     0,     44,    50,    0,    0,     0,       0,         0,       0,   -2,     5,          0,         0,    0,        0,       NULL, NULL },
       popeye      =  ( Object ) { 0, 0, DX_HAMBURGER+70, DY_HAMBURGER,           0,              0,              0,        5,     0,      0,     0,    0,    0,     0,       0,         0,       0,   -2,     5,          0,         0,    0,        0,       NULL, NULL },
       player      =  ( Object ) { 0, 0, DX_PERSONAGEM,  DY_PERSONAGEM,  VELOCX_PERSONAGEM,  FORCA_PULO,     VELOC_ESCADA,  0,     0,     37,    50,    0,    0,     0,       0,         0,       0,   -1,     0,          0,         0,    0,    QNT_VIDAS,   NULL, NULL }, // altura padrão é 50
       hamburger[] =           { { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,    0,     0,          0,         0,    0,        0,       NULL, NULL },
                                 { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,    1,     0,          0,         0,    0,        0,       NULL, NULL },
                                 { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,    2,     0,          0,         0,    0,        0,       NULL, NULL },
                                 { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,    3,     0,          0,         0,    0,        0,       NULL, NULL },
                                 { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,    4,     0,          0,         0,    0,        0,       NULL, NULL },
                                 { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,    5,     0,          0,         0,    0,        0,       NULL, NULL },
                                 { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,    6,     0,          0,         0,    0,        0,       NULL, NULL },
                                 { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,    7,     0,          0,         0,    0,        0,       NULL, NULL },
                                 { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,    8,     0,          0,         0,    0,        0,       NULL, NULL },
                                 { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,    9,     0,          0,         0,    0,        0,       NULL, NULL },
                                 { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,   10,     0,          0,         0,    0,        0,       NULL, NULL },
                                 { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,   11,     0,          0,         0,    0,        0,       NULL, NULL },
                                 { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,   12,     0,          0,         0,    0,        0,       NULL, NULL },
                                 { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,   13,     0,          0,         0,    0,        0,       NULL, NULL },
                                 { 0, 0,  DX_HAMBURGER,   DY_HAMBURGER,   VELOCX_HAMBURGER,           0, 1 + VELOC_ESCADA,  0,     0,     31,    27,    0,    0,     0,       0,         0,       0,   14,     0,          0,         0,    0,        0,       NULL, NULL }, },
       espinafre1 =   ( Object ) { 0, 0,       520,           370,                0,                  0,          0,        0,     0,     20,    20,    0,    0,     0,       0,         0,       1,   -2,     0,          0,         0,    0,        0,       NULL, NULL }, 
       espinafre2 =   ( Object ) { 0, 0,        10,           148,                0,                  0,          0,        0,     0,     20,    20,    0,    0,     0,       0,         0,       1,   -2,     0,          0,         0,    0,        0,       NULL, NULL };
//----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


// cada escada é uma struct com pontos definidos de modo a formar um retângulo
typedef struct
{
   float x1, y1, x2, y2;

}Escada;

//                    x1    y1   x2   y2       x1   y1   x2   y2
Escada ladder[] = { { 282,  41, 307, 116 }, { 680,  51, 704, 141 },
                    { 127, 152, 151, 244 }, { 282, 240, 310, 308 },
                    { 440, 247, 465, 310 }, { 647, 245, 671, 349 },
                    {  92, 349, 116, 447 }, { 311, 345, 335, 416 },
                    { 437, 346, 460, 418 }, { 195, 446, 219, 551 },
                    { 685, 454, 711, 553 }, {  10, 233,  35, 450 } };

// Essa struct conterá informações sobre a entrada de dados por texto do usuário
typedef struct 
{
   char str[11];
   SDL_Surface * text;
}StringInput;

StringInput input = (StringInput) { "", NULL };

typedef struct 
{
   float sX, sY, dX, dY, largura, altura;
   int ativo;
   SDL_Surface * image;
}Spinach;

Spinach espinafre[] = { { 0, 0, 520, 420, 20, 20, 1, NULL },
                        { 0, 0,  10, 168, 20, 20, 1, NULL } };

/*==============================================================================================================================================================================
================================================================================================================================================================================
================================================================================================================================================================================*/

// funções pré-declaradas para não ocorrer conflitos na hora de chamá-las
SDL_Surface * loadImage();
void eventoTeclado();
void andar();
void verificaLimites();
void controleFPS();
void desenharObjeto();
void animacao();
void moverObjetos();
void controleFrameSpeed();
void moverHamburger();
void morte();
int cair( void * obj );
int pular( void * obj );
int colisao( Object * objeto, int interacao, float dY );

// Verificar onde o objeto está pisando e retorna o chão do objeto. Tem o argumento float dY como passagem POR VALOR para
// poder verificar qual é a plataforma acima ou a plataforma abaixo
double verificaChao( Object * objeto, float dY )
{
   // plataformas 0 e 5 são retas
   // DECLIVE é o coeficiente angular
   plataforma0 = 520;
   plataforma1 = DECLIVE * objeto->dX + 414;
   plataforma2 = -DECLIVE * objeto->dX + 332;
   plataforma3 = DECLIVE * objeto->dX + 207;
   plataforma4 = -DECLIVE * objeto->dX + 130;
   plataforma5 = 24;
   
   // Aqui começa a verificação do chão pelas comparação de coordenadas
   if( dY <= 52  && objeto->dX <= 740 )
   {
      objeto->chao = plataforma5 - objeto->hulk;
      objeto->plataforma = 5;
   }
   else
   if( dY <= 152 && objeto->dX >=  41 )
   {
      objeto->chao = plataforma4 - objeto->hulk;
      objeto->plataforma = 4;
      // Se o objeto não estiver pulando, nem em uma escada, estiver longe do chao e já não estiver caindo, ele cairá!
      if( !( objeto->pulo ) && !( objeto->naEscada ) && objeto->chao - objeto->dY >= 0.5 && !( objeto->caindo ) )
         // Chamada da função cair como thread, passando o Object objeto para ponteiro para void como parâmetro
         thread[ objeto->rota + 1 ] = SDL_CreateThread( cair, (void *) objeto );
   }
   // O MESMO VAI SE REPETIR NAS PRÓXIMAS PLATAFORMAS!
   else
   if( dY <= 252 && objeto->dX <= 705 )
   {
      objeto->chao = plataforma3 - objeto->hulk;
      objeto->plataforma = 3;
      if( !( objeto->pulo ) && !( objeto->naEscada ) && objeto->chao - objeto->dY >= 0.5 && !( objeto->caindo ) )
         thread[ objeto->rota + 1 ] = SDL_CreateThread( cair, (void *) objeto );
   }
   else
   if( dY <= 352 && objeto->dX >= 43 )
   {
      objeto->chao = plataforma2 - objeto->hulk;
      objeto->plataforma = 2;
      if( !( objeto->pulo ) && !( objeto->naEscada ) && objeto->chao - objeto->dY >= 0.5 && !( objeto->caindo ) )
         thread[ objeto->rota + 1 ] = SDL_CreateThread( cair, (void *) objeto );
   }
   else
   if( dY <= 436 && objeto->dX <= 733 )
   {
      objeto->chao = plataforma1 - objeto->hulk;
      objeto->plataforma = 1;
      if( !( objeto->pulo ) && !( objeto->naEscada ) && objeto->chao - objeto->dY >= 0.5 && !( objeto->caindo ) )
         thread[ objeto->rota + 1 ] = SDL_CreateThread( cair, (void *) objeto );
   }
   else
   {
      objeto->chao = plataforma0 - objeto->hulk;
      objeto->plataforma = 0;
      if( !( objeto->pulo ) && !( objeto->naEscada ) && objeto->chao - objeto->dY >= 0.5 && !( objeto->caindo ) )
         thread[ objeto->rota + 1 ] = SDL_CreateThread( cair, (void *) objeto );
   }

   //SDL_WaitThread( thread, &returnThreadValue );

   // NOTA: Essa função não altera o Y do objeto, apenas verifica aonde ele está pisando, para saber se é necessário cair até o chão
   // o retorno da função é o chão que o objeto estaria pisando
   return objeto->chao;
}

// Função que fará o objeto cair 
int cair( void * obj )
{
   //SDL_SemWait( threadLock );
   // Como recebemos o objeto como ponteiro para void, precisamos fazer um cast e transformá-lo de volta a um ponteiro para Object
   Object * objeto = ( Object * ) obj;

   // Flag para indicar que o objeto está caindo
   objeto->caindo = 1;

   // Gravidade usada nesta função. Note que esta não é usada no pulo
   double gravidade   = FORCA_GRAVIDADE;


   do 
   {
       if( !pause )
       {
          // Verifica o chão a todo momento, pois geralmente o objeto cai de uma plataforma para a outra
          verificaChao( objeto, objeto->dY );
          
          // Começa a contagem do tempo para cada loop do laço
          start         = SDL_GetTicks();
          // o Y do objeto é incrementado pela gravidade
          objeto->dY   += gravidade;
          // A gravidade será mais forte no próximo loop
          gravidade    *= 1.2;
     
          // Se o objeto estiver com a face voltada para a direita
          if( objeto->direita )
          {
          	 // Então será usada a animação do pulo para a direita ( hamburger continua correndo )
             animacao( objeto, PULO_DIREITA );
             // X incrementado para a direita
             objeto->dX += 2;
          }
          else // se não é direita, é esquerda
          {
             animacao( objeto, PULO_ESQUERDA );
             objeto->dX -= 2;
          }

       	  // É necessário por ser uma thread. Quando o player morre, as flags são zeradas, então o objeto tem que parar de cair
          if( !objeto->caindo )
             break;
          
          // É verificado o limite a todo o momento para garantir que o objeto não saia da tela
          verificaLimites( objeto, 1 );
          verificaLimites( objeto, 2 );

          // Se não houver um controle de FPS, o objeto cairá na velocidade da corrupção brasileira
          controleFPS( );
      }

   }while( objeto->chao - objeto->dY > 0.5  && !( player.morto ) ); // tudo isso acontecerá enquanto o objeto estiver longe do chão

   // quando o objeto está no chão, a flag é desativada
   objeto->caindo = 0;

   //SDL_SemPost( threadLock );

   return 1;
}

// Desenha os objetos na tela
void desenharObjeto( Object * objeto, SDL_Surface * destino )
{
   SDL_Rect rFonte, rDestino;

   rFonte   = ( SDL_Rect ) { objeto->sX, objeto->sY, objeto->largura, objeto->altura };
   rDestino = ( SDL_Rect ) { objeto->dX, objeto->dY, objeto->largura, objeto->altura };

   // Se for um hamburger, será blitado um pouco mais abaixo para ficar no chão
   if( objeto->ativo )
      rDestino.y += 50 - objeto->altura;

   SDL_BlitSurface( objeto->image, &rFonte, destino, &rDestino );
}

// Função que vai controlar o FPS... DUH
void controleFPS( )
{
   if( 1000 / FPS > ( SDL_GetTicks() - start ) )
   SDL_Delay( 1000 / FPS - ( SDL_GetTicks() - start) );

}

// Aqui ocorre o controle da frequência de troca de frames para cada objeto
void controleFrameSpeed( Object * objeto )
{
   // O que ocorre: a cada vez que o loop principal é rodado, regulaFrames é incrementado
   objeto->regulaFrames++;
   if( objeto->regulaFrames == 4 )
      objeto->regulaFrames = 0;
  // Então resumindo, essa função altera a flag regulaFrames variando de 0 a 4. Será entendido melhor mais adiante
}

// Verifica se o objeto está dentro da janela, se não estiver, agora estará o/
void verificaLimites( Object * objeto, int acao )
{
// Ação (acao) = 1 verifica se o objeto saiu da tela no eixo Y ( ocorre no pulo parado e pulo andando )
// Ação (acao) = 2 verifica se o objeto saiu da tela no eixo X ( ocorre na corrida e no pulo andando )
   switch( acao )
   {
      case 1:
         // Não pode ser verificado se o objeto estiver na escada, ou então ele cairia ou subiria imediatamente
         // De resto, apenas não permite que o objeto saia da tela ( a não ser até -20 no eixo Y )
         if( objeto->dY > objeto->chao  && !( objeto->naEscada ) && !( player.morto ) )
         {
            objeto->dY   = objeto->chao;
            objeto->velY = FORCA_PULO;
         }
         else
         if( objeto->dY < -20 )
            objeto->dY = -20;
         break;

      case 2:
         if( objeto->dX > 800 - objeto->largura )
            objeto->dX = 800 - objeto->largura;
         else
         // Hamburgers podem ir até -30 no eixo X, porém ele é jogado devolta a plataforma mais alta e é desativado para ser jogado novamente
         if( objeto->ativo )
         {
            if( objeto->dX < -30 )
            {
               objeto->dX = DX_HAMBURGER;
               objeto->dY = DY_HAMBURGER;
               objeto->ativo = 0;
            }
         }
         else // Se for o player
         if( objeto->dX < 0 )
            objeto->dX = 0;
   }
}

// Verifica os frames dentro do spritesheet e o corrige para criar um laço com a sequência de frames, assim será possível animar corretamente
void verificaFrames( Object * objeto, int acao )
{
// Foram definidas flags no início do código para controlar as frames, aqui elas serão usadas dependendo de cada ação
   switch( acao )
   {  // No caso dos pulos, a frame é alterada de acordo com a velocidade do objeto ( só o player, no caso. hamburgers não pulam ._. )
      case PULO_DIREITA:
         if( objeto->velY > FORCA_PULO / 2 )
            objeto->sX = 0;
         else
         if( objeto->velY > FORCA_PULO - FORCA_PULO )
            objeto->sX = objeto->largura;
         else
         if( objeto->velY > (-1) * FORCA_PULO / 2 )
            objeto->sX = 2 * objeto->largura;
         else
            objeto->sX = 3 * objeto->largura; 

         break;

      case PULO_ESQUERDA:
         if( objeto->velY > FORCA_PULO / 2 )
            objeto->sX = 3 * objeto->largura;
         else
         if( objeto->velY > FORCA_PULO - FORCA_PULO )
            objeto->sX = 2 * objeto->largura;
         else
         if( objeto->velY > (-1) * FORCA_PULO / 2 )
            objeto->sX = objeto->largura;
         else
            objeto->sX = 0; 

         break;

      // Como só há 4 frames do player na escada, não é permitido que a sX seja maior que 3 * a largura da frame e nem menor que 0
      case ESCADA:
         if( objeto->sX > 3 * objeto->largura )
            objeto->sX = 0;
         else
         if( objeto->sX < 0 )
            objeto->sX = 3 * objeto->largura;

         break;      

      // Player correndo e parado e o hamburger correndo tem a mesma quantidade de frames, então essa parte serve para os três
      // É a mesma lógica dos frames da escada, só que com 8 frames
      case CORRENDO:
      case PARADO:
         if( objeto->sX > 7 * objeto->largura )
            objeto->sX = 0;
         else
         if( objeto->sX < 0 )
            objeto->sX = 7 * objeto->largura;
         break;

      case JG_HAMBURGER:
         if( objeto->sX > 4 * objeto->largura )
         {
         	objeto->sX = 4 * objeto->largura;
         	duduJogandoHamburger = 0;
         }
         else
         if( objeto->sX < 0 )
         	objeto->sX = 0;
         break;

      case DUDU_PARADO:
         if( objeto->sX > 5 * objeto->largura )
            objeto->sX = 0;
         else
         if( objeto->sX < 0 )
            objeto->sX = 0;
         break;


   }
}

// função que vai animar o objeto de acordo com o estado ( ação ) do mesmo
void animacao( Object * objeto, int acao )
{
   // Aqui é que os frames são controlados
   controleFrameSpeed( objeto );
   /*
      Se regulaFrames for 0, a próxima frame será utilizada. Explicando: Como o jogo roda a X FPS, as animações seriam rodadas a X FPS
      porem, com essa função, apenas a cada 4 loops que a animação é atualizada, ou seja, o jogo roda a X FPS, mas a animação roda a X/4 FPS
      Isso é para manter a movimentação dos objetos suaves na tela, mas também não permitindo que eles sejam animados muuuito rápido
   */
   if( !( objeto->regulaFrames ) )
   {
      // aux vai guardar a imagem do objeto temporariamente, para depois ser destruída
      objeto->aux = objeto->image;
      // dependendo da ação, o objeto terá uma animação diferente
      switch( acao )
      {
         case PULO_DIREITA:
            // ( o hamburger não pula, mas ao cair essa função é chamada, 
            // também pode haver uma mudança para colocar uma animação diferente no hamburger ao cair)
            // Se for um hamburger 
            if( objeto->ativo )
            {
               objeto->image = loadImage( "hamburger.png" );
               objeto->sX  += objeto->largura;
               verificaFrames( objeto, CORRENDO );
               break;
            }
            else
            if( objeto->hulk )
               objeto->image = loadImage( "Hulk Standing Right.png");
            else
               // se chegar até aqui, é por que não é hamburger
               objeto->image = loadImage( "Olive Jumping Right.png" );
            verificaFrames( objeto, PULO_DIREITA );
            break;

         case PULO_ESQUERDA:
            if( objeto->ativo )
            {
               objeto->image = loadImage( "hamburger.png" );
               objeto->sX  -= objeto->largura;
               verificaFrames( objeto, CORRENDO );
               break;
            }
            else
            if( objeto->hulk )
               objeto->image = loadImage( "Hulk Standing Left.png");
            else
               objeto->image = loadImage( "Olive Jumping Left.png" );
            verificaFrames( objeto, PULO_ESQUERDA );
            break;
         
         case ESCADA:
            // Se for hamburger
            if( objeto->ativo )
               objeto->image = loadImage( "hamburger.png" ); //Depois haverá uma animação diferente para o hamburger na escada
            else// Se não
               objeto->image = loadImage( "Olive On Ladder.png" );

            if( teclaCima && !( objeto->ativo )) // Se estiver subindo E (logicamente) não for um hamburger (hamburger só desce)
               objeto->sX += objeto->largura;
            else
            if( teclaBaixo || objeto->ativo ) // Se estiver descendo OU for um hamburger (hamburger sempre tenta descer a escada)
               objeto->sX -= objeto->largura;
            
            // Se for um hamburger, a flag para verificar o frame será CORRENDO ( 8 frames )
            if( objeto->ativo) 
               verificaFrames( objeto, CORRENDO );
            else// Se não, será um player na escada ( 4 frames )
               verificaFrames( objeto, ESCADA );
            break;

         // Mesma lógica
         case CORRENDO_DIREITA:
            if( objeto->ativo )
               objeto->image = loadImage( "hamburger.png" );
            else
            if( objeto->hulk )           
               objeto->image = loadImage( "Hulk Walking Right.png" );
            else
               objeto->image = loadImage( "Olive Running Right.png" );

            objeto->sX  += objeto->largura;
            if( !( objeto->caindo ) )
               objeto->dY   = objeto->chao;

            verificaFrames( objeto, CORRENDO );
            break;
         
         // Um pouco diferente de correr a direita, pois o spritesheet do hamburger serve pra direita e pra esquerda, o do player não
         case CORRENDO_ESQUERDA:
            if( objeto->ativo )
            {
               objeto->image = loadImage( "hamburger.png" );
               objeto->sX   -= objeto->largura;
            }
            else           
            {
               if( objeto->hulk )
                  objeto->image = loadImage( "Hulk Walking Left.png" );
               else
                  objeto->image = loadImage( "Olive Running Left.png" );
               objeto->sX  += objeto->largura;
            }

            if( !( objeto->caindo ) )
               objeto->dY   = objeto->chao;

            verificaFrames( objeto, CORRENDO );
            break;

         // Aqui só é verificado para  o player, por que hamburgers não param
         case PARADO_DIREITA:
            if( objeto->hulk )
               objeto->image = loadImage( "Hulk Standing Right.png" );
            else
               objeto->image = loadImage( "Olive Standing Right.png" );
            objeto->sX  += objeto->largura;
            verificaFrames( objeto, PARADO );
            break;

         case PARADO_ESQUERDA:
            if( objeto->hulk )
                objeto->image = loadImage( "Hulk Standing Left.png" );
            else
                objeto->image = loadImage( "Olive Standing Left.png" );
            objeto->sX  += objeto->largura;
            verificaFrames( objeto, PARADO );
            break;

         case MORTE:
            objeto->sX = 0;
            objeto->image = loadImage( "Olive Death.png");
            break;

         case DUDU_PARADO:
            objeto->image = loadImage( "Wimpy Standing Right.png" );
            objeto->sX += objeto->largura;
            verificaFrames( objeto, DUDU_PARADO );
            break;

         case DUDU_JG_HAMBURGER:
            objeto->image = loadImage( "Wimpy Throwing Burger.png" );
            objeto->sX += objeto->largura;
            verificaFrames( objeto, JG_HAMBURGER );
            break;

      }

      // Se aux já não tiver sido liberado
      if( objeto->aux != NULL )
      {
         SDL_FreeSurface( objeto->aux );
         objeto->aux = NULL;
      }
   }
}

// Define quais frames usar para os pulos parados (virado para a esquerda ou virado para direita)
// Também define quais frames usar quando o personagem está parado no chão ( esquerda / direita ) e o anima
// Funciona apenas para o player
void parado()
{
   int i = 0;
   // Se estiver pulando
   if( player.pulo )
   {  // E estiver com a face para a direita
      if( player.direita )
      	 // Então a animação será de pulo para a direita, mesmo que parado
         animacao( &player, PULO_DIREITA );
      else// Se não é direita, é esquerda
         animacao( &player, PULO_ESQUERDA );
   }
   else// Se não estiver pulando
   {   // mas estiver com a face para a direita
      if( player.direita )
      	 // Então a animação será de player parado para a direita
         animacao( &player, PARADO_DIREITA );
      else// Se não é direita, é esquerda
         animacao( &player, PARADO_ESQUERDA );
   }
   
   i = colisao( &player, HAMBURGER, player.dY );
   if( i ) // Verificar se o player colidiu com um hamburger enquanto estava parado
   {
      if( player.hulk )
      {
         pontos += 500;
         Mix_PlayChannel( -1, fxBurger, 0 );
         pause = 1;
         SDL_Delay( 250 );
         pause = 0;
         hamburger[i - 1].dX      = -31;
         hamburger[i - 1].dY      = plataforma0;
         hamburger[i - 1].direita =   0;

      }
      else
         morte(); // Se sim, então morrerá
   }




   verificaChao( &player, player.dY );
}

void vocePerdeu()
{  
   int terminou = 0;
   SDL_Delay( 100 );
   SDL_BlitSurface( gameOver, NULL, screen, NULL );
   SDL_Flip(screen);

   while( !terminou )
      while( SDL_PollEvent( &event ) )
      {
         switch(event.type)
         {
            case SDL_KEYDOWN:
               terminou = 1;
            break;
         }
      }
  imagemTela = menu;
}
// INCOMPLETO!! FALTA FAZER A ANIMAÇÃO
// Causa a morte do jogador
void morte()
{
   int i;
   double gravidade = FORCA_GRAVIDADE;
   player.morto = 1; // ativa a flag de morte apenas para o player ( hamburgers são imortais )
   player.vidas--;
   player.velY = FORCA_PULO; // A velocidade do pulo é resetada, para o caso de o player ter morrido enquanto pulava
 /*  A jogada aqui é que o jogo todo para enquanto um loop não for concluído, e isso tem que acontecer até que o player decida continuar
   Então é usado o while até que o player volte a vida ( flag de morte desativada ) */
   //while( player.morto && jogoRodando )
   //{
      // A morte do personagem é feita no estilo Sonic/Mario
   player.velY *= 4;

   Mix_PauseMusic();
   SDL_Delay( 250 );
   Mix_PlayChannel( -1, fxMorte, 0 );
   
   while( player.dY < 660)
   {
      // Troca a imagem do personagem
      animacao( &player, MORTE );
      // começa a contagem do tempo para o controle de FPS
      start = SDL_GetTicks();
      // O player vai subir um pouco, enquanto a velocidade Y for positiva
      player.dY -= player.velY ;

      // A velocidade Y vai ser decrementada até ser menor ou igual a -2, quando ela é negativa, o player desce
      if( player.velY > -10 )
         player.velY -= gravidade;

      controleFPS();
      // Atualizar os objetos na tela, para a morte do personagem não passar despercebida
      for( i = 0; i < 16; i++)
         thread[i] = NULL;

      moverObjetos();
   }
   // TODOS os hamburgers são retornados ao início da plataforma mais alta e são desativados
   for( i = 0; i < ( sizeof( hamburger ) / sizeof( hamburger[0] ) ); i++ )
   {
      hamburger[i].naEscada     = 0;
      hamburger[i].escadagrande = 0;
      hamburger[i].ativo        = 0;
      hamburger[i].dX           = DX_HAMBURGER;
      hamburger[i].dY           = plataforma5;
      player.dY = 660;
      verificaChao( &hamburger[i], hamburger[i].dY );       
   }
   if( !player.vidas )
   {
      jogoRodando = 0;
      vocePerdeu();
   }
   

   // Pegar os eventos do teclado
   //eventoTeclado( &player );
   // Note que para o player voltar a vida, o usuário deve pressionar a tecla S
   //}
   // Algumas informações do player são reiniciadas
   SDL_Delay(1000);

   Mix_ResumeMusic();   
   player.chao = plataforma0;
   player.plataforma = 0;
   player.dX = 60;
   player.dY = plataforma0;;
   player.velY = FORCA_PULO;
   player.pulo = 0;
   player.naEscada = 0;
   player.escadagrande = 0;
   player.morto = 0;
}

// Anima o personagem quando ele anda para a direita e esquerda
void andar( Object * objeto )
{
   int i = 0;
   // Se (for correr pra direita E não( for um hamburger ) OU ( for um hamburger E ( estiver na plataforma 5 OU 3 OU 1 ) )
   if( ( teclaDireita && !( objeto->ativo ) ) || ( objeto->ativo && ( objeto->plataforma == 5 || objeto->plataforma == 3 || objeto->plataforma == 1 ) ) )
   {  // Flag pra direita ativada
      objeto->direita  = 1;
      // E se estiver pulando (flag do pulo ativo)      
      if( objeto->pulo )
         animacao( objeto, PULO_DIREITA );
      else // Se não estiver pulando, andará para a direita
         animacao( objeto, CORRENDO_DIREITA );

      // Incremento para andar para a direita (seja pulando ou parado)
      objeto->dX += objeto->velX;
   }
   else // Se não,
   // Se ( for correr pra esquerda E não( for um hamburger ) ) OU ( for um hamburger E ( estiver na plataforma 4 OU 2 OU 0 ) )
   if( ( teclaEsquerda && !( objeto->ativo ) ) || ( objeto->ativo && ( objeto->plataforma == 4 || objeto->plataforma == 2 || objeto->plataforma == 0 ) ) )
   {  // Flag pra direita desativada (então é esquerda)
      objeto->direita  = 0;
      // E se estiver pulando (flag do pulo ativo)
      if( objeto->pulo )
         animacao( objeto, PULO_ESQUERDA );
      else// Se não, está correndo
         animacao( objeto, CORRENDO_ESQUERDA );
      
      // Incremento (decremento, no caso <.<) para andar para a esquerda (pulando ou parado)
      objeto->dX -= objeto->velX;
   }
   
   // Se nem esquerda nem direita forem pressionados, o personagem está parado
   if( !( teclaEsquerda + teclaDireita ) && !objeto->ativo )
      parado();

   i = colisao( &player, HAMBURGER, player.dY );
   if( i ) // Verificar se o player colidiu com um hamburger enquanto estava parado
   {
      if( player.hulk )
      {
         pontos += 500;
         Mix_PlayChannel( -1, fxBurger, 0 );
         pause = 1;
         SDL_Delay( 250 );
         pause = 0;
         hamburger[i - 1].dX      = -31;
         hamburger[i - 1].dY      = plataforma0;
         hamburger[i - 1].direita =   0;

      }
      else
         morte(); // Se sim, então morrerá
   }

   verificaChao( objeto, objeto->dY );
   // Manter o personagem dentro da tela
   verificaLimites( objeto, 2 );
}


int verificarSePontuou()
{
   int i;
   for( i = 0; i < ( sizeof(hamburger) / sizeof( hamburger[0]) ); i++ )
      if(  hamburger[i].pontua && ( player.dX >= hamburger[i].dX && player.dX <= hamburger[i].dX + hamburger[i].largura )
       && !colisao( &player, HAMBURGER, player.dY ) && player.plataforma == hamburger[i].plataforma )
      {
         hamburger[i].pontua = 0;
         Mix_PlayChannel( -1, fxPonto, 0 );
         pontos += 100;
      }
}

// Função thread que executará o pulo!
// O parâmetro ponteiro para void obj aqui não é utilizado, mas é necessário por ser thread (não é usado por que apenas o player pula, e o player é global)
int pular( void * obj )
{

   player.pulo = 1;   // Flag do pulo ativada
   Mix_PlayChannel( -1, fxPulo, 0 );
               

   double gravidade = FORCA_GRAVIDADE;

   // Laço do pulo
   do
   {
      if( !pause )
      {
       	 // É necessário por ser uma thread. Quando o player morre, as flags são zeradas, então o player tem que sair do pulo
       	 if( !player.pulo )
       	 	  break;
         // Aqui é verificado onde o personagem está pisando, para saber como o pulo deve se comportar
         verificaChao( &player, player.dY );
         start = SDL_GetTicks();   // Começa a contagem do tempo para controlar FPS
          /* Ao mesmo tempo que o pulo é incrementado (ou decrementado, já que o eixo Y do SDL é invertido),
            decrementamos a velocidade do pulo pela força da gravidade e, logo após, incrementamos a força da 
            gravidade para dar uma sensação de aceleração */
         player.dY   -= player.velY;
         player.velY -= gravidade * 0.23;   // valores sujeitos a modificações
         gravidade   *= 1.025;
         // manter o player na tela
         verificaLimites( &player, 1 );
         // o player pode se mover enquanto pula
         andar( &player );
         controleFPS();
         eventoTeclado( &player );
         // verificar se o player passou por cima de um hamburger, para ganhar os pontos
         verificarSePontuou();
      }
   // enquanto o player estiver longe do chão
   }while( player.chao - player.dY > 0.5 && !( player.morto ));

   // Desativar a flag do pulo
   player.pulo = 0;
   // Resetar a força da gravidade
   gravidade   = FORCA_GRAVIDADE;
}

// Liberar as imagens da memória e apontar as variáveis das surfaces para NULL, além de esperar as threads para terminá-las
void fechar()
{
   int i;
   
   SDL_RWops* highScoreFile = SDL_RWFromFile( "highScore.bin", "w+b" );
   
   if( highScoreFile != NULL )
   {
     for( i = 0; i < ( sizeof( highScoreFileDataName ) / sizeof( highScoreFileDataName[i]) ); i++ )
     {
       SDL_RWwrite( highScoreFile, &highScoreFileDataName[ i ], sizeof( highScoreFileDataName[i] ), 1 );
       SDL_RWwrite( highScoreFile, &highScoreFileDataPoints[ i ], sizeof( highScoreFileDataPoints[i]), 1);
     }

     SDL_RWclose( highScoreFile );
   }
   else
   {
     printf( "Error: Unable to save file! %s\n", SDL_GetError() );
   }


   //screen = SDL_SetVideoMode( 800, 600, 32, SDL_HWSURFACE );

   if( menu != NULL )  
      SDL_FreeSurface( menu );
   menu = NULL;
   
   if( recordes != NULL )
      SDL_FreeSurface( recordes );
   recordes = NULL;
   
   if( creditos != NULL )
      SDL_FreeSurface( creditos );
   creditos = NULL;
   
   if( mododejogo != NULL )
      SDL_FreeSurface( mododejogo );
   mododejogo = NULL;
   
   if( historia != NULL )
      SDL_FreeSurface( historia );
   historia = NULL;
   
   if( comandos != NULL )
      SDL_FreeSurface( comandos );
   comandos = NULL;

   if( screen != NULL )
       SDL_FreeSurface( screen );
   screen = NULL;

   if( fundo != NULL )
       SDL_FreeSurface( fundo );
   fundo = NULL;

   if( plat != NULL )
       SDL_FreeSurface( plat );
   plat = NULL;

   if( player.image != NULL )
       SDL_FreeSurface( player.image );
   player.image = NULL;

   if( dudu.image != NULL )
       SDL_FreeSurface( dudu.image );
   dudu.image = NULL;

   if( espinafre1.image != NULL )
       SDL_FreeSurface( espinafre1.image );
   espinafre1.image = NULL;

   if( espinafre2.image != NULL )
       SDL_FreeSurface( espinafre2.image );
   espinafre2.image = NULL;

   if( scoreWordImage != NULL )
       SDL_FreeSurface( scoreWordImage );
   scoreWordImage = NULL;

   if( scorePointsImage != NULL )
       SDL_FreeSurface( scorePointsImage );
   scorePointsImage = NULL;

   if( livesImage != NULL )
       SDL_FreeSurface( livesImage );
   livesImage = NULL;
   
   // Liberar os hamburgers
   for( i = 0; i < ( sizeof( hamburger ) / sizeof( hamburger[0] ) ); i++ )
   {
      if( hamburger[i].image != NULL )
          SDL_FreeSurface( hamburger[i].image );
      hamburger[i].image = NULL;
   }
   // Liberar o navio
   for( i = 0; i < 41; i++ )
   {
      if( rotShip[i] != NULL )
   	      SDL_FreeSurface( rotShip[i] );
   	  rotShip[i] = NULL;
   }
   // Esperar as threads e terminá-las
   for( i = 0; i < 16; i++ )
   {
      SDL_WaitThread( thread[i], &returnThreadValue );
      //if( thread[i] != NULL )   
          //SDL_KillThread( thread[i] );
      thread[i] = NULL;
   }

   if( fxBurger != NULL )
     Mix_FreeChunk( fxBurger );
   fxBurger = NULL;

   if( fxMutacao != NULL )
     Mix_FreeChunk( fxMutacao );
   fxMutacao = NULL;

   if( fxPonto != NULL )
   	  Mix_FreeChunk( fxPonto );
   fxPonto = NULL;

   if( fxPulo != NULL )
   	  Mix_FreeChunk( fxPulo );
   fxPulo = NULL;

   if( fxMorte != NULL )
   	  Mix_FreeChunk( fxMorte );
   fxMorte = NULL;

   if( bgTrack != NULL )
   	  Mix_FreeMusic( bgTrack );
   bgTrack = NULL;

   // fecha o TTF, SDL e Mixer
   TTF_Quit();
   SDL_Quit();
   Mix_CloseAudio();
}

// Verificará colisões player X escada, player X hamburger, hamburger X escada
// recebe a flag interação para saber se está lidando com uma escada ou um hamburger
// float dY POR VALOR pois em alguns casos é necessário saber se há uma escada acima e/ou abaixo
int colisao( Object * objeto, int interacao, float dY )
{
   // ponto X Central, ponto Y Central, ponto X1, ponto X2, ponto Y1, ponto Y2
   double  pXC, pYC, pX1, pX2, pY1, pY2;

   // Se não for um hamburgers, a definição dos pontos será um pouco diferente por causa da distancia entre o personagem na frame e o offset da frame
   if( !(objeto->ativo) )
   {
      pXC = objeto->dX + objeto->largura / 2,
      pYC = dY + objeto->altura / 2,
      pX1 = objeto->dX + 12,
      pX2 = objeto->dX + objeto->largura - 10,
      pY1 = dY - 6,
      pY2 = dY + objeto->altura - 2;
   }
   else
   {
      // Se for um hamburger, a alteração dos pontos é diferente
      pXC = objeto->dX + objeto->largura / 2,
      pYC = dY + objeto->altura / 2,
      pX1 = objeto->dX + 14,
      pX2 = objeto->dX + objeto->largura - 17,
      pY1 = dY,
      pY2 = dY + objeto->altura;
   }

   int i;
   
   switch( interacao )
   {  /*
   	    Aqui ocorre a verificação da colisão com cada escada do jogo. Para isso é necessário um FOR percorrendo todas as escadas.
   	    Se o objeto colidir com a escada, o índice da escada será comparado comparado com a rota do objeto da colisão. Cada escada
   	    aceita hamburgers diferentes. As escadas não quebradas ( 11, 10, 6, 5, 2 e 1 ) aceitam o player, que tem rota = -1.
   	  */
      case ESCADA:
         for( i = 0; i < ( sizeof( ladder ) / sizeof( ladder[0] ) ) ; i++ )
            if( ( ( pX2 >= ladder[i].x1 && pX2 <= ladder[i].x2 ) ||  ( pX1 >= ladder[i].x1 && pX1 <= ladder[i].x2 ) )
             && ( ( pY2 >= ladder[i].y1 && pY2 <= ladder[i].y2 ) ||  ( pY1 >= ladder[i].y1 && pY1 <= ladder[i].y2 ) ) )
            {

                switch( i )
                {
                   case 0:
                      switch( objeto->rota )
                      {
                         case  0:
                         case  1:
                         case  2:
                         case  3:
                         case 10:
                         case 11:
//                            objeto->dX = ladder[i].x1;
                            return 1;
                            break;
                      }
                      break;

                   case 1:
                      switch( objeto->rota )
                      {
                         case -1:
                         case  4:
                         case  5:
                         case  6:
                         case 12:
//                            objeto->dX = ladder[i].x1;
                            return 1;
                            break;
                      }
                      break;

                   case 2:
                      switch( objeto->rota )
                      {
                         case -1:
                         case  0:
                         case  1:
                         case  5:
                         case  6:
                         case  4:
                         case 11:
//                            objeto->dX = ladder[i].x1;
                            return 1;
                            break;
                      }
                      break;

                   case 3:
                      switch( objeto->rota )
                      {
                         case  2:
                         case  0:
//                            objeto->dX = ladder[i].x1;
                            return 1;
                            break;
                      }
                      break;

                   case 4:
                      switch( objeto->rota )
                      {
                         case  4:
                         case  3:
                         case  1:
//                            objeto->dX = ladder[i].x1;
                            return 1;
                            break;
                      }
                      break;

                   case 5:
                      switch( objeto->rota )
                      {
                         case -1:
                         case  5:
                         case  7:
//                            objeto->dX = ladder[i].x1;
                            return 1;
                            break;
                      }
                      break;

                   case 6:
                      switch( objeto->rota )
                      {
                         case -1:
                         case  2:
                         case  7:
  //                          objeto->dX = ladder[i].x1;
                            return 1;
                            break;
                      }
                      break;

                   case 7:
                      switch( objeto->rota )
                      {
                         case  3:
                         case  9:
//                            objeto->dX = ladder[i].x1;
                            return 1;
                            break;
                      }
                      break;

                   case 8:
                      switch( objeto->rota )
                      {
                         case  8:
                         case  5:
                         case  1:
                         case  6:
//                            objeto->dX = ladder[i].x1;
                            return 1;
                            break;
                      }
                      break;

                   case 9:
                      switch( objeto->rota )
                      {
                         case  0:
//                            objeto->dX = ladder[i].x1;
                            return 1;
                            break;
                      }
                      break;

                   case 10:
                      switch( objeto->rota )
                      {
                         case -1:
                         case  4:
                         case  2:
                         case  3:
                         case  1:
                         case 11:
                         case 14:
//                            objeto->dX = ladder[i].x1;
                            return 1;
                            break;
                      }
                      break;
/*
 ----------------------------ESCADÃO ESPECIAL!!!---------------------
     Essa escada ativa a flag escadagrande de cada objeto que puder acessá-la
*/
                   case 11:
                      switch( objeto->rota )
                      {
                         case -1:
                         case 10:
                         case 12:
                         case 13:
//                            objeto->dX = ladder[i].x1;
                            objeto->escadagrande = 1;
                            return 1;
                            break;
                      }
                      break;                  
                }
            }

         break;
      // Caso seja um player colidindo com um hamburger, um FOR percorrerá os hamburgers
      case HAMBURGER:
         for( i = 0; i < ( sizeof( hamburger ) / sizeof( hamburger[0] ) ); i++ )
            if( ( ( ( pX2 - 6>= hamburger[i].dX +  3 && pX2 - 6<= hamburger[i].dX+hamburger[i].largura - 3  ) || ( pX1 + 6  >= hamburger[i].dX + 3 && pX1 + 6 <= hamburger[i].dX+hamburger[i].largura - 3 ) )
               && ( ( pY2 >= hamburger[i].dY + 25 && pY2 <= hamburger[i].dY+hamburger[i].altura + 25  ) || ( pY1 + 25 >= hamburger[i].dY+ 25 && pY1 + 25 <= hamburger[i].dY+hamburger[i].altura + 25  ) ) ) && hamburger[i].ativo )
               return i + 1;
         break;
      
      case ESPINAFRE:
         if( ( ( pX2 >= espinafre1.dX && pX2 <= espinafre1.dX + espinafre1.largura ) ||  ( pX1 >= espinafre1.dX && pX1 <= espinafre1.dX + espinafre1.largura ) )
          && ( ( pY2 >= espinafre1.dY && pY2 <= espinafre1.dY + espinafre1.altura  ) ||  ( pY1 + 10 >= espinafre1.dY && pY1 - 10 <= espinafre1.dY + espinafre1.altura ) ) && espinafre1.ativo)
         {
            player.hulk = 10;
            player.largura = 50;
            player.altura = 60;
            animacao( &player, PARADO_ESQUERDA );
            Mix_PlayChannel( -1, fxMutacao, 0 );
            pause = 1;
            SDL_Delay( 500 );
            pause = 0;
            espinafre1.ativo = 0;
            hulkTimer = SDL_GetTicks();
         }
         else
         if( ( ( pX2 >= espinafre2.dX && pX2 <= espinafre2.dX + espinafre2.largura ) ||  ( pX1 >= espinafre2.dX && pX1 <= espinafre2.dX + espinafre2.largura ) )
          && ( ( pY2 >= espinafre2.dY && pY2 <= espinafre2.dY + espinafre2.altura  ) ||  ( pY1 + 10 >= espinafre2.dY && pY1 - 10 <= espinafre2.dY + espinafre2.altura ) ) && espinafre2.ativo )
         {
            player.hulk = 10;
            player.largura = 50;
            player.altura = 60;
            animacao( &player, PARADO_DIREITA );
            Mix_PlayChannel( -1, fxMutacao, 0 );
            pause = 1;
            SDL_Delay( 500 );
            pause = 0;
            espinafre2.ativo = 0;
            hulkTimer = SDL_GetTicks();
         }
   }

   return 0;
}

// Thread que lida com os hamburgers e o player subindo e descendo a escada
int escada( void * obj )
{
   // conversão de ponteiro para void  para  ponteiro para Object
   Object * objeto = ( Object * ) obj;
   
   // Se ( subir e não colidir com uma escada acima (por isso o float dY na colisão) ) OU (ser hamburger E não colidir com escada alguma)
   if( ( teclaCima && !colisao( objeto, ESCADA, objeto->dY - 79 ) ) || ( objeto->ativo && !colisao( objeto , ESCADA, objeto->dY ) ) )
      return 0; // não vai subir ninguém
   else// Se não
   // Se (descer e não colidir com uma escada abaixo)                OU (ser hamburger E não colidir com uma escada abaixo) (dá pra simplificar isso aqui... (A*~B)+(C*~B) = ~B*(A+C) )
   if( ( teclaBaixo && !colisao( objeto, ESCADA, objeto->dY + 79 ) ) || ( objeto->ativo && !colisao( objeto , ESCADA, objeto->dY + 79 ) ) )
      return 0; // retorna sem descer
   else// se não
   // Se (não colidir com escada alguma )
   if( !colisao( objeto, ESCADA, objeto->dY ) )
      return 0;
   else// Se passar por tudo isso, estará finalmente usando a escada
      objeto->naEscada = 1;

   // Aqui começa a treta
   // i, j, e k são contadores
   // j é até onde o loop deve contar, i é o contador até j, k é para ajustes na escada especial

   int i, j, k = 0;
   // plataforma acima e plataforma abaixo
   double platAbaixo, platAcima;

   // Se não estiver na escada grande, verificaremos a plataforma de cima e a de baixo ( para isso serve o float dY no verificaChao )
   if( !( objeto->escadagrande ) )
   {
      platAcima = verificaChao( objeto, objeto->dY - 100 );
      platAbaixo = verificaChao( objeto, objeto->dY + 100 );
   }
   else
   // Se não, estará na escada grande. Então serão verificadas as plataformas ACIMA DA DE CIMA e ABAIXO DA DE BAIXO (escada grande liga duas plataformas distantes)
   {
      platAcima = verificaChao( objeto, objeto->dY - 210 );
      platAbaixo = verificaChao( objeto, objeto->dY + 210 );
   }
   
   // verificado o chão mais uma vez para voltar ao normal
   verificaChao( objeto, objeto->dY );

   // Se for tecla pra baixo vai descer, se for um hamburger também, já que hamburgers sempre querem descer as escadas
   if( teclaBaixo || objeto->ativo )
   {
   	  // então o loop do while contará até j, que é a distancia da plataforma de baixo até o chão dividido pela velocidade da escada somado a 1 para não ter erro
      // desta forma, o player ou hamburger descerá a escada até uma distancia certinha para a plataforma de baixo
   	  // A cada loop ele subirá VELOC_ESCADA de distancia
   	  // Exemplo: se a distância do chão até a plataforma de baixo for 100 e VELOC_ESCADA for 2, o while precisará rodar 50 vezes para chegar até a plataforma de baixo o/
      j = abs( platAbaixo - objeto->chao ) / VELOC_ESCADA + 1;
      i = ( j ) - 1;
   }
   else
   // Se for tecla pra cima vai subir, mas não pode ser um hamburger
   if( teclaCima && !(objeto->ativo) )
   {
      j = abs( platAcima - objeto->chao ) / VELOC_ESCADA + 1;
      i = 1;
   }

   // Enquanto o módulo do contador for menor que o máximo E o contador for maior que zero
   while( abs( i ) < j && i > 0 && !( player.morto ) )
   {
      if( !pause )
      { 
       	  // É necessário por ser uma thread. Quando o player morre, as flags são zeradas, então o objeto tem que sair da escada
       	  if( !objeto->naEscada )
       	     break;
       	  // troca a animação para escada
          animacao( objeto,ESCADA );
          // começa a contagem do tempo
          start = SDL_GetTicks();

          // Se for subir E não for um hamburger ( lógico )
          if( teclaCima && !(objeto->ativo) )
          {
          	 // decrementará Y e incrementará o contador
             objeto->dY -= VELOC_ESCADA;
             i++;
          }
          // Se for descer OU for um hamburger
          if( teclaBaixo || objeto->ativo )
          {
             objeto->dY += VELOC_ESCADA;
             i--;
          }

          // Se a escada for a escadagrande
          if( objeto->escadagrande )
          {  // haverá um incremento no Y para dificultar a subida
             objeto->dY += VELOC_ESCADA * 0.5;
             // como o incremento é de METADE da velocidade da escada, o contador i ficaria desregulado,
             // então temos que decrementá-lo proporcionalmente à velocidade da escada grande
             k++;
             if( k == 2 )
             {
                k = 0;
                i--;
             }
          }
          controleFPS( );

          eventoTeclado( objeto );
      }
   }
   // Desativa a flag da escada
   objeto->naEscada = 0;
   // Desativa flag da escada grande
   objeto->escadagrande = 0;
   verificaChao( objeto, objeto->dY );
   verificaLimites( objeto, 1 );

}
 
// Lidar com os eventos do teclado
void eventoTeclado( Object * objeto )
{
   // Se não for um hamburger
   if( !objeto->ativo )
   // Pega os eventos da pilha de eventos
   if( SDL_PollEvent( &event ) )
   {
      // Se clicarem no X para sair, fechar
      if( event.type == SDL_QUIT ) jogoRodando = 0;

      // Se pressionarem uma tecla
      if( event.type == SDL_KEYDOWN )
      { 
         // Verificar qual tecla foi pressionada para ativar as flags
         switch( event.key.keysym.sym )
         {
            case SDLK_RIGHT:  teclaDireita  = 1;
               break;
            case SDLK_LEFT:   teclaEsquerda = 1;
               break;
            case SDLK_UP:     teclaCima     = ( teclaCima ) ? 0 : 1;
               break;
            case SDLK_DOWN:   teclaBaixo    = ( teclaBaixo ) ? 0 : 1;
               break;
            case SDLK_SPACE:  teclaEspaco   = ( objeto->pulo ) ? : 1;
               break;
            case SDLK_s:
               if( player.vidas > 0 )
                  player.morto = 0;
               else
                  jogoRodando = 0;
               break;
            case SDLK_ESCAPE: jogoRodando   = 0;
               break;
         }
      }
      
      // Se soltarem a tecla
      if( event.type == SDL_KEYUP )
      {
         // Verificar qual tecla foi solta para desativar as flags
         switch( event.key.keysym.sym )
         {
            case SDLK_RIGHT: teclaDireita  = 0;
               break;
            case SDLK_LEFT:  teclaEsquerda = 0;
               break;
            case SDLK_UP:    teclaCima     = 0;
               break;
            case SDLK_DOWN:  teclaBaixo    = 0;
               break;
            case SDLK_SPACE: teclaEspaco   = 0;
               break;
         }
      }
   }
   // Se ( ( ( tecla direita ou esquerda for pressionada ) e ( o personagem não estiver pulando) ) ou ( (f or um hamburger ) e (não estiver caindo) ) ) e ( não estiver na escada) e ( o player não estiver morto ), ande!
   if( ( ( ( teclaDireita || teclaEsquerda ) && !( objeto->pulo ) ) || ( ( objeto->ativo ) && !( objeto->caindo ) ) )  && !( objeto->naEscada ) && !( player.morto ) )
      andar ( objeto );

   // Se a tecla espaço for pressionada e o personagem não estiver pulando  e  não estiver na escada e não for um hamburger e o player não estiver morto, pule!
   if( teclaEspaco  && !( objeto->pulo )  && !( objeto->naEscada ) && !( objeto->ativo ) && !( player.morto ) && !player.hulk )
      thread[ objeto->rota + 1 ] = SDL_CreateThread( pular, ( void * ) objeto ); // chamada da função thread pular
   else
   // Se a tecla para cima ou para baixo forem pressionadas e já não estiver na escada e não estiver pulando e não estiver morto suba ou desça a escada
   if( ( teclaCima    || teclaBaixo ) && !( objeto->naEscada ) && !( objeto->pulo ) && !( player.morto ) && !player.hulk)
      thread[ objeto->rota + 1 ] = SDL_CreateThread( escada, ( void * ) objeto ); // chamada da função thread escada
   else
   // Se nenhuma tecla for pressionada e o personagem não estiver na escada e não estiver na escada e não for um hamburger e não estiver morto, o personagem está parado
   if( !( teclaDireita + teclaEsquerda + teclaCima + teclaBaixo ) && !( objeto->naEscada ) && !( objeto->ativo ) && !( player.morto ) )
      parado( );
}

// move todos os objetos na tela, inclusive o barco do fundo
void moverObjetos()
{
   int i;

   // Aqui v controlará a variação de Y do barco no fundo de 60 a 75
   if( rSrcShip.y == 80 )
   	  v = 1;
   else
   if( rSrcShip.y == 90 )
   	  v = -1;

   // aqui 'a' controlará o ângulo de rotação do barco no fundo de 0 a 360 graus (só de sacanagi)
   if( frameShip == 0 )
      a = 1;
   else
   if( frameShip == 40 )
   	  a = -1;

   // Blitar o fundo primeiro
   SDL_BlitSurface ( fundo, 0, screen, 0 );
   // Aqui é blitada uma das imagens rotacionadas do barco
   SDL_BlitSurface ( rotShip[frameShip], &rSrcShip, screen, &rDstShip );

   // Aqui é usado o regulaFrames de algum objeto (player foi escolhido) para ajudar a controlar a velocidade de atualização do barco no fundo
   // Ou seja, isso é programação orientada à gambiarra
   if( !player.regulaFrames && !player.morto )
   {
      frameShip += a;

      if( frameShip % 2 == 0 )
         rSrcShip.y += v;
   }
   
   // blitando as plataformas
   SDL_BlitSurface ( plat, 0, screen, 0 );
   desenharObjeto ( &dudu, screen );
   if( espinafre1.ativo )
      desenharObjeto ( &espinafre1, screen );
   if( espinafre2.ativo )
      desenharObjeto ( &espinafre2, screen );
   // blitando o player
   desenharObjeto  ( &player, screen );
   // blitando a palavra 'Score:'
   SDL_BlitSurface ( scoreWordImage, 0, screen, &rScoreWord);
   // blitando a pontuação do jogador
   SDL_BlitSurface ( scorePointsImage, 0, screen, &rScorePoints);
   // blitando as vidas do jogador
   SDL_BlitSurface ( livesImage, &rSrcLivesImage, screen, &rDstLivesImage );
   // blitar as palavras 'High Score:'
   SDL_BlitSurface ( highScoreWordImage, 0, screen, &rHighScoreWord );
   // blitar a maior pontuação até o momento
   SDL_BlitSurface ( highScorePointsImage, 0, screen, &rHighScorePoints );
   // blitar todos os hamburgers na tela
   SDL_BlitSurface ( bonusScoreWordImage, 0, screen, &rBonusScoreWord );
   SDL_BlitSurface ( bonusScorePointsImage, 0, screen, &rBonusScorePoints );
   if( !player.morto )
   for( i = 0; i < ( sizeof( hamburger ) / sizeof( hamburger[0] ) ); i++ )
      {
         if( hamburger[i].ativo )
            desenharObjeto  ( &hamburger[i], screen );
         else
         {
            hamburger[i].dX = DX_HAMBURGER;
            hamburger[i].dY = plataforma5;
         }
      }
   // atualizar a tela
   SDL_Flip( screen );
}

// INCOMPLETO!! AINDA FALTA A ANIMAÇÃO DO DUDU
// Função para jogar os hamburgers BEEEM aleatoriamente
void jogarHamburger()
{
   // Incremento da variável que auxiliará na frequência de emissão de hamburgers
   if( !duduJogandoHamburger )
   {
      intervaloHamburger++;
      animacao( &dudu, DUDU_PARADO );
   }
   // O IntervaloHamburger será incrementado até ser igual a FPS * a taxa de frequência de hamburgers
   // Ex: Se FPS = 60 e FREQ_HAMBURGER = 1.5, 1.5*60 = 90, intervaloHamburger terá de ser incrementado 90 vezes e
   // como só é incrementado uma vez por loop, serão necessários 90 loops a 60 fps
   if( intervaloHamburger == FPS * freqHamburger )
   {
      intervaloHamburger = 0; // quando atinge a meta, é zerado
   }

   if( !intervaloHamburger ) // se intervaloHamburger for = a zero
      if( burgerDaVez != -1 && !hamburger[burgerDaVez].ativo && !player.morto ) // Se o burgerDaVez foi escolhido( != -1 ) e o hamburger da vez estiver desativado e o player estiver vivo
      {
         if( !duduJogandoHamburger )
            dudu.sX = 0 - dudu.largura;

      	 duduJogandoHamburger = 1;
      	 animacao( &dudu, DUDU_JG_HAMBURGER );

         if( !duduJogandoHamburger )
         {
             hamburger[burgerDaVez].chao       = plataforma5;
             hamburger[burgerDaVez].dY         = plataforma5;
             hamburger[burgerDaVez].plataforma = 5;
             hamburger[burgerDaVez].ativo      = 1;// o hamburger da vez é ativado
             hamburger[burgerDaVez].pontua     = 1;// Será capaz de pontuar o jogador novamente
             burgerDaVez = -1;// logo depois o burger da vez é desselecionado
         }
      }
      else
      {
      	 // Se o burger da vez não foi escolhido, será agora com um índice aleatório baseado no tempo de jogo
         srand( SDL_GetTicks() );
         // esse indice será sempre entre 0 e 14
         burgerDaVez = rand() % 15;
      }
}

// Move a posição dos hamburgers ATIVOS pela tela
void moverHamburger()
{
   int i;
   // Se o player morrer, os hamburgers param!
   if( !( player.morto ) )
   	  // verifica-se todos os hamburgers
      for( i = 0; i < ( sizeof( hamburger ) / sizeof( hamburger[0] ) ); i++ )
      	 // Se o hamburger não estiver caindo e estiver ativo e não estiver na escada
         if( !( hamburger[i].caindo ) && hamburger[i].ativo && !( hamburger[i].naEscada ))
         {
         	// Ele anda!
            andar( &hamburger[i] );
            // Se ele estiver andando e esbarrar numa escada
            if( colisao( &hamburger[i], ESCADA, hamburger[i].dY + 50) )
               // Ele desce!                
               thread[ i + 1 ] = SDL_CreateThread( escada, (void *) &hamburger[i] );
         }
}

// Carrega a imagem no formato da screen
// O parâmetro é o caminho para a imagem
SDL_Surface * loadImage( char* caminho )
{
   SDL_Surface * imagemCarregada = NULL;
   SDL_Surface * imagemOtimizada = NULL;

   imagemCarregada = IMG_Load( caminho );

   if( imagemCarregada != NULL )
   {
      imagemOtimizada = SDL_DisplayFormat( imagemCarregada );
    	SDL_FreeSurface( imagemCarregada );
      imagemCarregada = NULL;
   }

   return imagemOtimizada;
}

// Atualiza a pontuação do jogador em tempo real
void updateScore( )
{
   // Variável que vai guardar a pontuações de int para string
   char pontosString[10];
   // Converter os pontos em int para string
   snprintf( pontosString, 10, "%d", pontos);
   
   // aux guardará a imagem temporariamente antes de ser liberada da memória
   // pontos atuais
   aux = scorePointsImage;   
   scorePointsImage = TTF_RenderText_Solid( accIngameScore, pontosString, color[0] );
   SDL_FreeSurface( aux );
   aux = NULL;

   snprintf( pontosString, 10, "%d", highScoreIntPoints[0]);
   // maior pontuação
   aux = highScorePointsImage;
   highScorePointsImage = TTF_RenderText_Solid( accIngameScore, pontosString, color[0] );
   SDL_FreeSurface( aux );
   aux = NULL;

   snprintf( pontosString, 10, "%d", bonusScoreIntPoints );
   aux = bonusScorePointsImage;
   bonusScorePointsImage = TTF_RenderText_Solid( accIngameScore, pontosString, color[0] );
   SDL_FreeSurface( aux );
   aux = NULL;

   // Atualização da quantidade de vidas do player
   rSrcLivesImage.x = 18 * ( QNT_VIDAS - player.vidas ); 

   aux = NULL;
   
}

int loadHighScore()
{
   int success = 1, i; 

   SDL_RWops * highScoreFile = SDL_RWFromFile( "highScore.bin", "r+b" );

   if( highScoreFile == NULL )
   {
      printf( "Warning! Unable to open high score file! ERROR: %s\n", SDL_GetError() );
      highScoreFile = SDL_RWFromFile( "highScore.bin", "w+b" );

      if( highScoreFile != NULL )
      {
         printf( "New high score file created!\n" );
         for( i = 0; i < ( sizeof( highScoreFileDataName ) / sizeof( highScoreFileDataName[0] )); i++ )
         {
            strcpy( highScoreFileDataName[i], "" );
            strcpy( highScoreFileDataPoints[i], "" );
            SDL_RWwrite( highScoreFile, &highScoreFileDataName[i], sizeof( highScoreFileDataName[i] ), 1 );
            SDL_RWwrite( highScoreFile, &highScoreFileDataPoints[i], sizeof( highScoreFileDataPoints[i] ), 1 );
         }

         SDL_RWclose( highScoreFile );
      }
      else
      {
         printf( "Error: Unable to create high score file! ERROR: %s\n", SDL_GetError() );
      }  success = 0;
   }
   else
   {
      puts( "Reading high score... ");
      for( i = 0; i < ( sizeof( highScoreFileDataName ) / sizeof( highScoreFileDataName[0] ) ); i++ )
      {
         SDL_RWread( highScoreFile, &highScoreFileDataName[i], sizeof( highScoreFileDataName[i]), 1 );  
         SDL_RWread( highScoreFile, &highScoreFileDataPoints[i], sizeof( highScoreFileDataPoints[i] ), 1 );
      }

      SDL_RWclose( highScoreFile );  
   }

   for( i = 0; i < sizeof( highScoreFileDataPoints) / sizeof( highScoreFileDataPoints[i]); i++ )
   {
      highScoreIntPoints[i] = atoi( highScoreFileDataPoints[i] );
   }

}

void showHighScore()
{
   int i;

   SDL_Surface * highScoreNameImage[5] = { NULL }, * highScorePointsImage[5] = { NULL };
 
   SDL_BlitSurface( highScoreImage, 0, screen, 0 );

   for( i = 0; i < ( sizeof( highScoreFileDataPoints ) / sizeof( highScoreFileDataPoints[i]) ); i++ )
   {
      aux = highScoreNameImage[i];
      highScoreNameImage[i] = TTF_RenderText_Solid( accHighScore, highScoreFileDataName[i], color[1] );
      SDL_FreeSurface( aux );
     
      aux = highScorePointsImage[i];
      highScorePointsImage[i] = TTF_RenderText_Solid( accHighScore, highScoreFileDataPoints[i], color[0] );
      SDL_FreeSurface( aux );
 
      SDL_BlitSurface( highScoreNameImage[i], 0, screen, &rHighScoreName[i] );
      SDL_BlitSurface( highScorePointsImage[i], 0, screen, &rHighScoreValues[i] );
   }

   SDL_Flip( screen );
}

void loadShipAnimation()
{
   int i;

   char rotShipPath[41][25]; // vetor de strings que vai armazenar o caminho de cada imagem do barco
   FILE * arq; // variável que vai apontar para o arquivo de texto com os nomes das imagens dos barcos
   
   arq = fopen( "rotShip.txt", "r"); // abertura do arquivo em modo de leitura
   for(i = 0; i < 41; i++ )
   {
      fgets( rotShipPath[i], 25, arq); // pegando uma string do arquivo, cada string é um caminho pra imagem
      // os 20 primeiros caminhos tem um tamanho diferente dos 21 caminhos restantes, então o fim de string fica em lugares diferentes
      if(i >= 20)
         rotShipPath[i][22] = '\0';
      else
         rotShipPath[i][23] = '\0';

      rotShip[i] = loadImage( rotShipPath[i] );
   }

   fclose( arq );

}

int updateHighScore()
{
   int i, j, newHighScore = -1;

   char pontosString[10];

   for( i = 0; i < ( sizeof( highScoreIntPoints ) / sizeof( highScoreIntPoints[i]) ); i++ )
      if( pontos > highScoreIntPoints[i] && ( venceu || noMenu ) )
      {
         newHighScore = i;
         for( j = ( sizeof( highScoreIntPoints ) / sizeof( highScoreIntPoints[i]) ); j > i; j--)
         {
            if( j - 1 == i )
            {
               highScoreIntPoints[j-1] = pontos;
               snprintf( pontosString, 10, "%d", pontos );
               strcpy( highScoreFileDataPoints[j-1], pontosString );
               strcpy( highScoreFileDataName[j-1], " ");
            }
            else
            {
               highScoreIntPoints[j-1] = highScoreIntPoints[j-2];
               snprintf( pontosString, 10, "%d", highScoreIntPoints[j-2] );
               strcpy( highScoreFileDataPoints[j-1], pontosString );
               strcpy( highScoreFileDataName[j-1], highScoreFileDataName[j-2]);
            }
         }
         break;        
      }

   return newHighScore;
}

char* eraseLast(char *str)
{
    size_t nullIndex = strlen( str );
    str[nullIndex - 1] = '\0';
    return str;
}

void handleUserInput( int highScorePosition )
{
   SDL_Rect rNewHighScoreName = ( SDL_Rect ){ 100, 156 + highScorePosition * 80, 0, 0 };
   //Se uma tecla foi pressionada
   if( event.type == SDL_KEYDOWN )
   {
      //Faz uma cópia temporária da string atual
      char temp[11], tempChar;
      strcpy( temp, input.str );
      //Se a string for menor que o tamanho máximo
      if( strlen(input.str) <= 10 )
      {
         if( event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT )
            shiftOn = 1;
         // Se a tecla for espaço
         else if( event.key.keysym.sym == (Uint16)' ' )
         {
            //A string é concatenada com o novo caractere
            tempChar = (char) event.key.keysym.sym;
            strncat( input.str, &tempChar , 1 );
         }
         // Se a tecla for um número
         else if( ( event.key.keysym.sym >= (Uint16)'0' ) && ( event.key.keysym.sym <= (Uint16)'9' ) )
         {
            //A string é concatenada com o novo caractere
            tempChar = (char) event.key.keysym.sym;
            strncat( input.str, &tempChar , 1 );
         }
         // Se a tecla for uma letra minúscula
         else if( ( event.key.keysym.sym >= (Uint16)'a' ) && ( event.key.keysym.unicode <= (Uint16)'z' ) )
         {
            
            // Se o Shift estiver pressionado, o char é transformado em maiúsculo
            if( shiftOn )
               tempChar = (char) ( event.key.keysym.sym - 32 ); 
            else
               tempChar = (char) event.key.keysym.sym;
            //A string é concatenada com o novo caractere
            strncat( input.str, &tempChar , 1 );
         }
          
      }
      // Se o backspace foi pressionado e a string não está vazia
      if( ( event.key.keysym.sym == SDLK_BACKSPACE ) && ( strlen(input.str) != 0 ) )
      {
         //Um caractere é removido do final da string
         strcpy(input.str, eraseLast( input.str ));
      }
      //Se a string foi modificada
      if( input.str != temp )
      {
         // A surface antiga é liberada
         SDL_FreeSurface( input.text );
         // A nova surface é renderizada
         input.text = TTF_RenderText_Solid( accHighScore, input.str, color[1] );
      }
  }
  else // Se a tecla shift foi solta
  if( event.type == SDL_KEYUP )
     if( event.key.keysym.sym == SDLK_LSHIFT || event.key.keysym.sym == SDLK_RSHIFT )
        shiftOn = 0;

  showHighScore();
  SDL_BlitSurface( input.text, 0, screen, &rNewHighScoreName);
  SDL_Flip(screen);
}

void getUserInput( int newHighScorePosition )
{
    int quit = 0, nameEntered = 0;

    while( quit == 0 )
    {
        // Enquanto houver eventos para lidar
        while( SDL_PollEvent( &event ) )
        {    
            // Se o usuário não entrou com o nome ainda
            if( nameEntered == 0 )
            {
                // Então pegamos a entrada do usuário
                handleUserInput( newHighScorePosition );

                // Se a tecla Enter foi pressionada
                if( ( event.type == SDL_KEYDOWN ) && ( event.key.keysym.sym == SDLK_RETURN ) )
                {
                    // Mudamos a flag para 1, indicando que o usuário entrou com o nome
                    nameEntered = 1;
                    quit = 1;
                }
            }

            //Se o usuário deseja sair
            if( event.type == SDL_QUIT )
            {
                //Sair do loop
                quit = 1;
            }
        }
   }

    strcpy( highScoreFileDataName[ newHighScorePosition ], input.str );
    strcpy( input.str, "" );
    input.text = TTF_RenderText_Solid( accHighScore, input.str, color[1] );

}

void eventoMouse()
{
   while (SDL_PollEvent(&event)) // Loop de eventos
   {
      // Verifica se o evento mais antigo é do tipo SDL_QUIT
 
      switch(event.type)
      {
         case SDL_QUIT:
            done = 1;
            break;

         case SDL_MOUSEBUTTONDOWN:
            mouse.x = event.motion.x;
            mouse.y = event.motion.y;
            break;
      }
   }
}

void menuRodando()
{
      if( imagemTela == menu )
      {
        if(mouse.x > 291 && mouse.x < 511 && mouse.y > 259 && mouse.y < 320)
        {
           mouse.x    = 0;
           mouse.y    = 0;
           imagemTela = mododejogo;
        }
        else
        if(mouse.x > 225 && mouse.x < 578 && mouse.y > 349 && mouse.y < 409)
        {
           mouse.x    = 0;
           mouse.y    = 0;
           imagemTela = highScoreImage;
           
        }
        else
        if(mouse.x > 230 && mouse.x < 571 && mouse.y > 436 && mouse.y < 499)
        {
           mouse.x    = 0;
           mouse.y    = 0;
           imagemTela = creditos;
        }
        else
        if(mouse.x > 332 && mouse.x < 472 && mouse.y > 525 && mouse.y < 588)
          done = 1;
      }
      else 
      if(imagemTela == mododejogo)
      {
          if(mouse.x < 93 && mouse.y > 580)
          {        
             mouse.x    = 0;
             mouse.y    = 0;
             imagemTela = menu;
          }
          else
          if(mouse.x > 315 && mouse.x < 511 && mouse.y > 388 && mouse.y < 427)
          {         
             mouse.x    = 0;
             mouse.y    = 0;
             imagemTela = historia;
             saudavel   = 1;
          }
          else
          if(mouse.x > 279 && mouse.x < 546 && mouse.y > 500 && mouse.y < 537)
          {
             mouse.x    = 0;
             mouse.y    = 0;
             imagemTela = historia;
             gorduroso  = 1;
          }
      }
      else
      if(imagemTela == comandos)
      {
            if(mouse.x > 684 && mouse.x < 790 && mouse.y > 565 && mouse.y < 585)
            {
               mouse.x = 0;
               mouse.y = 0;
               imagemTela  = menu;
               jogoRodando = 1;
            }  
            
      }     
      else
      if(imagemTela == historia)
      {
            if( mouse.x > 684 && mouse.x < 790 && mouse.y > 565 && mouse.y < 585 )
            {             
              mouse.x = 0;
              mouse.y = 0;
              imagemTela  = comandos;
            }
            else
            if( mouse.x < 94 && mouse.y > 580 )
            {
               mouse.x = 0;
               mouse.y = 0;
               imagemTela = mododejogo;
            }
            
      }
      else
      if(imagemTela == highScoreImage)
      {
                SDL_BlitSurface( imagemTela, NULL, screen, NULL);
                SDL_Flip( screen );
                //updateHighScore();
                showHighScore();

            while( !( mouse.x < 94 && mouse.y > 580) )
            {
                eventoMouse();
            }
               mouse.x = 0;
               mouse.y = 0;
               imagemTela = menu;
      }  
      else 
      if(imagemTela == creditos)
          if(mouse.x < 105 && mouse.y > 580)
          {
             mouse.x = 0;
             mouse.y = 0;
             imagemTela = menu;
          }

      
      SDL_BlitSurface(imagemTela, NULL, screen, NULL);
      SDL_Flip(screen);
      SDL_Delay(30);     

}

void voceVenceu()
{  
   int terminou = 0;
   SDL_Delay( 100 );
   SDL_BlitSurface( youWin, NULL, screen, NULL );
   SDL_Flip(screen);
   SDL_Delay( 1000 );

   while( !terminou )
      while( SDL_PollEvent( &event ) )
      {
         switch(event.type)
         {
            case SDL_KEYDOWN:
               terminou = 1;
            break;
         }
      }
}

void verificarSeGanhou()
{
   if( player.dX <= 260 && player.chao == plataforma5 && !player.morto )
   {
      venceu = 1;
      jogoRodando = 0;
   }
}

void carregarDados( )
{
   XInitThreads();
   TTF_Init();
   Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 );
   SDL_Init( SDL_INIT_EVERYTHING );
   SDL_WM_SetCaption( "Popeye Gordo", NULL );
   srand( SDL_GetTicks() );
 
   burgerDaVez         = rand() % 15;
   screen              = SDL_SetVideoMode( 800, 600, 32, SDL_SWSURFACE | SDL_DOUBLEBUF );
   plat                = loadImage( "plataformas.png" );
   fundo               = loadImage( "background.png" );
   livesImage          = loadImage( "Olive Lives.png" );
   youWin              = loadImage( "You Win.png" );
   gameOver            = loadImage( "Game Over.png" );
   highScoreImage      = loadImage( "Recordes.png");
   espinafre1.image    = loadImage( "Spinach Can.png" );
   espinafre2.image    = loadImage( "Spinach Can.png" );
   menu                = loadImage( "Menu.png" );
   recordes            = loadImage( "Novo Recorde.jpg" );
   creditos            = loadImage( "Creditos.png" );
   mododejogo          = loadImage( "Mododejogo.png" );
   historia            = loadImage( "História.jpg" );
   comandos            = loadImage( "Comandos.jpg" );
   imagemTela          = menu;
   threadLock          = SDL_CreateSemaphore( 1 );
   accIngameScore      = TTF_OpenFont( "ATARCC__.TTF", 12 );
   accHighScore        = TTF_OpenFont( "ATARCC__.TTF", 35 );
   scoreWordImage      = TTF_RenderText_Solid( accIngameScore, "Score:", color[1] );
   highScoreWordImage  = TTF_RenderText_Solid( accIngameScore, "High Score:", color[1] );
   bonusScoreWordImage = TTF_RenderText_Solid( accIngameScore, "Bonus:", color[1] );
   bgTrack             = Mix_LoadMUS( "WagonWheelElectronic.wav" );
   fxMutacao           = Mix_LoadWAV( "SFX_Mutate.wav" );
   fxBurger            = Mix_LoadWAV( "SFX_DestroyBurger.wav" );
   fxPulo              = Mix_LoadWAV( "SFX_Jump.wav" );
   fxPonto             = Mix_LoadWAV( "SFX_Score.wav" );
   fxMorte             = Mix_LoadWAV( "SFX_Death.wav" );

   loadHighScore();
   loadShipAnimation();

}

void resetarInfo()
{
   int i;
   bonusScoreIntPoints = 10000;
   pontos = 0;
   bonusTimer = SDL_GetTicks();
   if( saudavel )
      freqHamburger = 1;
   else
   if( gorduroso )
      freqHamburger = 0.5;

   for( i = 0; i < sizeof( hamburger ) / sizeof( hamburger[0] ); i++ )
   {
      hamburger[i].ativo        = 0;
      hamburger[i].dX           = DX_HAMBURGER;
      hamburger[i].dY           = plataforma5;
      hamburger[i].chao         = plataforma5;
      hamburger[i].plataforma   = 5;
      hamburger[i].naEscada     = 0;
      hamburger[i].escadagrande = 0;
      hamburger[i].caindo       = 0;
      if( saudavel )
      {
         hamburger[i].velX = VELOCX_HAMBURGER;
      }
      else
      if( gorduroso )
      {
         hamburger[i].velX = VELOCX_HAMBURGER + 1.5;
      }
   }

   player.dX           = DX_PERSONAGEM;
   player.dY           = 520;
   player.chao         = plataforma0;
   player.plataforma   = 0;
   player.hulk         = 0;
   player.vidas        = QNT_VIDAS;
   player.pulo         = 0;
   player.naEscada     = 0;
   player.escadagrande = 0;
   player.caindo       = 0;
   player.morto        = 0;

   espinafre1.ativo = 1;
   espinafre2.ativo = 1;

}

int main( int argc, const char* argv[] )
{
   int newHighScore, i; // contador

   carregarDados();   

   while( !done )
   {
      eventoMouse();
      menuRodando();

      if( jogoRodando )
      {
         Mix_PlayMusic( bgTrack, -1 ); 
         Mix_VolumeMusic( volumeMusica );

         resetarInfo();
         noMenu = 0;
         while( jogoRodando )
         {
            start = SDL_GetTicks();
            jogarHamburger();
            moverHamburger();
            updateScore();
            moverObjetos();
            eventoTeclado( &player );
            controleFPS();
            if( player.hulk && start - hulkTimer >= 10000 )
            {
               player.hulk = 0;
               player.altura = 50;
               player.largura = 37;
            }
            if( start - bonusTimer >= 1500 && bonusScoreIntPoints > 0 )
            {
               bonusScoreIntPoints -= 100;
               bonusTimer = SDL_GetTicks();
            }
            colisao( &player, ESPINAFRE, player.dY );
            verificarSeGanhou();

         }
         

         Mix_HaltMusic();
         pontos += bonusScoreIntPoints;
         newHighScore = updateHighScore();
         if( venceu )
            voceVenceu();

         if( newHighScore != -1 && venceu )
         {
            getUserInput( newHighScore );
            newHighScore = -1;
            showHighScore();
            SDL_Delay( 5000 );
         }

         venceu    = 0;
         saudavel  = 0;
         gorduroso = 0;
      }
      noMenu = 1;     

   }

   fechar();
   return 0;
}
