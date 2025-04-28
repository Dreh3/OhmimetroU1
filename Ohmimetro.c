#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define ADC_PIN 28 // GPIO para o ohmimetro
#define Botao_A 5  // GPIO para botão A

//Arquivo .pio
#include "pio_matrix.pio.h"
#define IS_RGBW false 
#define MatrizLeds 7 //Pino para matriz de leds
PIO pio = pio0;
int sm =0;

int R_conhecido = 10080;   // Resistor de 10k ohm
float R_x = 0.0;           // Resistor desconhecido
float ADC_VREF = 3.31;     // Tensão de referência do ADC
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)
uint indice_tabela = 56;
uint faixa1=0; //Ordem: 0 - preto, 1 - marrom, 2 - vermelho, 3 - laranja .....
uint faixa2=0;
uint faixa3=0;

// Trecho para modo BOOTSEL com botão B
#include "pico/bootrom.h"
#define botaoB 6
void gpio_irq_handler(uint gpio, uint32_t events)
{
  reset_usb_boot(0, 0);
};

//Tabela com os valores comerciais para mostrar na matriz de leds
char tabela_comercial[57][6] = {" 510 "," 560 "," 620 "," 680 "," 750 "," 820 "," 910 ","  1 k","1.1 k",
                                "1.2 k","1.3 k","1.5 k","1.6 k","1.8 k","2.0 k","2.2 k","2.4 k",
                                "2.7 k","3.0 k","3.3 k","3.6 k","3.9 k","4.3 k","4.7 k","5.1 k",
                                "5.6 k","6.2 k","6.8 k","7.5 k","8.2 k","9.1 k"," 10 k"," 11 k",
                                " 12 k"," 13 k"," 15 k"," 16 k"," 18 k"," 20 k"," 22 k"," 24 k"," 27 k",
                                " 30 k"," 33 k"," 36 k"," 39k"," 43 k"," 47 k"," 51 k"," 56 k"," 62 k",
                                " 68 k"," 75 k"," 82 k"," 91 k","100 k"," ---"};
//Tabela com as cores das faixas
char tabela_cores[10][10] = {"Preto","Marrom","Vermelho","Laranja","Amarelo","Verde","Azul","Violeta","Cinza","Branco"};

//Configurações para a matriz de Leds

//Definindo struct para cores personalizadas
typedef struct {
  double red;
  double green;
  double blue;
}Led_RGB;

//Definindo tipo Cor
typedef Led_RGB COR_RGB;

// Definição de tipo da matriz de leds
typedef Led_RGB Matriz_leds[5][5];

//Retorno o valor binário para a cor passada por parâmetro
uint32_t cor_binario (double b, double r, double g)
{
unsigned char R, G, B;
R = r * 255; 
G = g * 255;  
B = b * 255;
return (G << 24) | (R << 16) | (B << 8);
};

//Função responsável por acender os leds desejados 
void acender_leds(Matriz_leds matriz){
  //Primeiro for para percorrer cada linha
  for (int linha =4;linha>=0;linha--){
      /*
      Devido à ordem de disposição dos leds na matriz de leds 5X5, é necessário
      ter o cuidado para imprimir o desenho na orientação correta. Assim, o if abaixo permite o 
      desenho saia extamente como projetado.
      */

      if(linha%2){                             //Se verdadeiro, a numeração das colunas começa em 4 e decrementam
          for(int coluna=0;coluna<5;coluna++){
              uint32_t cor = cor_binario(matriz[linha][coluna].blue,matriz[linha][coluna].red,matriz[linha][coluna].green);
              pio_sm_put_blocking(pio, sm, cor);
          };
      }else{                                      //Se falso, a numeração das colunas começa em 0 e incrementam
          for(int coluna=4;coluna>=0;coluna--){
              uint32_t cor = cor_binario(matriz[linha][coluna].blue,matriz[linha][coluna].red,matriz[linha][coluna].green);
              pio_sm_put_blocking(pio, sm, cor);
          };
      };
  };
};

//Função para mostrar na matriz de leds a faixa dos resistores
void resistor_faixas(){

  //Definindo cores e tonalidades
  COR_RGB apagado = {0.0,0.0,0.0};
  COR_RGB bordas = {0.004,0.004,0.001};
  
                              // PRETO - MARROM - VERMELHO - LARANJA - AMARELO - VERDE - AZUL - VIOLETA - CINZA - BRANCO (ordem das cores no vetor)
  COR_RGB cores_faixas[10] = {{0.0,0.0,0.0},{0.015,0.007,0.003},{0.1,0.0,0.0},{0.2,0.03,0},{0.125,0.068627451,0},
                              {0.0,0.08,0.0},{0.0,0.0,0.1},{0.04,0.0,0.06},{0.008,0.008,0.004},{0.05,0.05,0.05}};
  
  Matriz_leds resistor = {{apagado,apagado,apagado,apagado,apagado},
                          {bordas,bordas, bordas,bordas, bordas},
                          {bordas, cores_faixas[faixa1], cores_faixas[faixa2],cores_faixas[faixa3], bordas},
                          {bordas, bordas, bordas,bordas, bordas},
                          {apagado,apagado,apagado,apagado,apagado}};
  
  //Se o valor encontrado estiver fora da faixa de tolerância, a matriz mostrará todas as faixas apagadas
  Matriz_leds fora_da_faixa = {{apagado,apagado,apagado,apagado,apagado},{bordas,bordas,bordas,bordas, bordas},{bordas, apagado,apagado,apagado, bordas},{bordas, bordas,bordas,bordas, bordas},{apagado,apagado,apagado,apagado,apagado}};

  
  acender_leds(resistor);     //Mostra na matriz de leds as faixas 
  

};


uint identificar_tolerancia(){ //Retorna o valor encontrado

  bool encontrado = false;    //Variável para saber se o valor foi encontrado ou não
  uint valor_comercial = 0;  
  float faixa_superior = 0;   //Armazena 95% do valor de uma posição na tabela
  float faixa_inferior = 0;   //Armazena 105% do valor de uma posição na tabela

  //Tabela com os valores comerciais disponíveis na série E24
  uint valores_disponiveis[56] = {510,560,620,680,750,820,910,1000,1100,1200,
                              1300,1500,1600,1800,2000,2200,2400,2700,3000,
                              3300,3600,3900,4300,4700,5100,5600,6200,6800,
                              7500,8200,9100,10000,11000,12000,13000,15000,
                              16000,18000,20000,22000,24000,27000,30000,
                              33000,36000,39000,43000,47000,51000,56000,
                              62000,68000,75000,82000,91000,100000};
  
  for(int i=0;i<56 && !encontrado;i++){             //Percorre a tabela de valores disponíveis
    faixa_inferior = 0.95*valores_disponiveis[i];   //Calcula os valores de tolerância para a posição na tabela
    faixa_superior = 1.05*valores_disponiveis[i];   //Sendo 5% pra mais e pra menos

    if(R_x>=faixa_inferior && R_x<=faixa_superior){ //Verifica se o valor encontrado está dentro dos limites
      valor_comercial = valores_disponiveis[i];     
      indice_tabela = i;                            //Posição na tabela de strings para imprimir o valor no display
      encontrado = true;                            //Controle para saber se o vetor foi todo percorrido
    };

  }; 
 
  if(!encontrado){      //Se encontrado for falso, significa que o resistor está fora dos limites 
    printf("\t\n!!Atenção!!\nValor fora da faixa de tolerância dos padrões comerciais\n"); 
    indice_tabela=56;   //Posição na tebal de string que mostra "---" no display
  };

  printf("\nValor comercial encontrado: %d",valor_comercial);   //Mensagem informativa

  return valor_comercial;
  
};

//Função para identificar as faixas
void encontrar_faixas(uint Rx_int){ //Tem como paraâmetro o valor de R_x depois de identificada a tolerência
  
  //MODIFICAÇÃO
  char valor_Rx[9];               //Buffer para armazenar o Rx e identificar os valores separadamente
  int ncasa=0;                    //Armazena a quatidade de casas o número possui
  
  if(Rx_int==0){    //Se o valor comercial não for identificado, o display e as matriz vão msotrar as faixas sobre o valor calculado inicialmente
    Rx_int = R_x;
  };
  sprintf(valor_Rx, "%d", Rx_int);//Converte R_x em string

  ncasa = strlen(valor_Rx);       //Armazena a informação do número de casas

  //if(Rx_int==0){                  //Sgnifica que o valor encontrado está fora da faixa de tolerância
    //faixa1=10;
    //faixa2=11;
    //faixa3=12;
  //}else{
    
    if(ncasa < 3){                //Encontrando faixa 3
      faixa3=0;
    }else{
      faixa3 = ncasa-2;
    };

    if(ncasa==1){                 //Encontrando as outras duas faixas
      faixa1=0;               
      faixa2=valor_Rx[0]-48;      //Nessa caso só tem um algarismo significativo
    }else{
      faixa1 = valor_Rx[0] - 48;  //Converte o valor de char para int
      faixa2 = valor_Rx[1] - 48;
    };
  //};

  //Mensagem sobre o código de cores
  printf("\nCódigo de cores:\n\t1º faixa: %s - 2º faixa: %s - 3º faixa: %s",tabela_cores[faixa1],tabela_cores[faixa2],tabela_cores[faixa3]);

};


int main()
{
  stdio_init_all();
 
  adc_init();
  adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica

  char str_y[5];          // Buffer para armazenar a string
  uint valor_comercial=0; //Armazena o valor comercial identificado

  // Para ser utilizado o modo BOOTSEL com botão B
  gpio_init(botaoB);
  gpio_set_dir(botaoB, GPIO_IN);
  gpio_pull_up(botaoB);
  gpio_set_irq_enabled_with_callback(botaoB, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  
  //Configurações para matriz de leds
  uint offset = pio_add_program(pio, &pio_matrix_program);
  pio_matrix_program_init(pio, sm, offset, MatrizLeds, 800000, IS_RGBW);

  // Configurações paa o uso do display
  bool cor = true;
  i2c_init(I2C_PORT, 400 * 1000);
  gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
  gpio_pull_up(I2C_SDA);                                        // Pull up the data line
  gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
  ssd1306_t ssd;                                                // Inicializa a estrutura do display
  ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
  ssd1306_config(&ssd);                                         // Configura o display
  ssd1306_send_data(&ssd);                                      // Envia os dados para o display
 
  // Limpa o display. O display inicia com todos os pixels apagados.
  ssd1306_fill(&ssd, false);
  ssd1306_send_data(&ssd);


  while (true)
  {
    adc_select_input(2);                // Seleciona o ADC para eixo X. O pino 28 como entrada analógica

    float soma = 0.0f;
    for (int i = 0; i < 500; i++)
    {
      soma += adc_read();
      sleep_ms(1);
    }
    float media = soma / 500.0f;        //Coleta uma quantidade de dados grande para maior precisão

    R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);   //Fórmula para descobrir o valor desconhecido
    printf("\nValor encontrado pelo ohmimetro: %f",R_x);      //Mostra o valor encontrado
    sprintf(str_y, "%1.0f", R_x);                             // Converte o float em string para mostrar no display
    
    
    valor_comercial = identificar_tolerancia();       //Funções para identificar a tolerância e as faixas do resistor
    encontrar_faixas(valor_comercial);

    // cor = !cor;
    //  Atualiza o conteúdo do display com as informações necessárias
    ssd1306_fill(&ssd, !cor);                          // Limpa o display
    ssd1306_rect(&ssd, 3, 1, 122, 61, cor, !cor);      // Desenha um retângulo ok
    ssd1306_line(&ssd, 3, 13, 123, 13, cor);           // Desenha uma linha ok
    ssd1306_line(&ssd, 3, 23, 123, 23, cor);           // Desenha uma linha
    ssd1306_line(&ssd, 3, 33, 123, 33, cor);           // Desenha uma linha
    ssd1306_line(&ssd, 3, 43, 123, 43, cor);           // Desenha uma linha
    ssd1306_line(&ssd, 3, 53, 123, 53, cor);           // Desenha uma linha
    ssd1306_draw_string(&ssd, "OHMIMETRO", 28, 5); // Desenha uma string
    ssd1306_draw_string(&ssd, "Res.:", 10, 15);  // Desenha uma string
    ssd1306_draw_string(&ssd, tabela_comercial[indice_tabela], 55, 15);  // Desenha uma string
    ssd1306_draw_string(&ssd, "ohm", 95, 15);  // Desenha uma string
    ssd1306_draw_string(&ssd, "1", 12, 25);          // Desenha uma string
    ssd1306_draw_string(&ssd, tabela_cores[faixa1], 38, 25);
    ssd1306_draw_string(&ssd, "2", 12, 35);          // Desenha uma string
    ssd1306_draw_string(&ssd, tabela_cores[faixa2], 38, 35); 
    ssd1306_draw_string(&ssd, "3", 12, 45);          // Desenha uma string
    ssd1306_draw_string(&ssd, tabela_cores[faixa3], 38, 45);
    ssd1306_draw_string(&ssd, "ADC:", 10, 55);  // Desenha uma string
    ssd1306_draw_string(&ssd, str_y, 55, 55);
    ssd1306_line(&ssd, 28, 23, 28, 53, cor);           // Desenha uma linha vertical
    ssd1306_send_data(&ssd);                           // Atualiza o display
    sleep_ms(700);
    
    resistor_faixas();                                //Chama a função que atualiza a matriz de leds
  };
};
