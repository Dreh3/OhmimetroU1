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
#define ADC_PIN 28 // GPIO para o voltímetro
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

char tabela_comercial[57][6] = {" 510 "," 560 "," 620 "," 680 "," 750 "," 820 "," 910 ","  1 k","1.1 k",
                                "1.2 k","1.3 k","1.5 k","1.6 k","1.8 k","2.0 k","2.2 k","2.4 k",
                                "2.7 k","3.0 k","3.3 k","3.6 k","3.9 k","4.3 k","4.7 k","5.1 k",
                                "5.6 k","6.2 k","6.8 k","7.5 k","8.2 k","9.1 k"," 10 k"," 11 k",
                                " 12 k"," 13 k"," 15 k"," 16 k"," 18 k"," 20 k"," 22 k"," 24 k"," 27 k",
                                " 30 k"," 33 k"," 36 k"," 39k"," 43 k"," 47 k"," 51 k"," 56 k"," 62 k",
                                " 68 k"," 75 k"," 82 k"," 91 k","100 k"," ---"};

char tabela_cores[13][10] = {"Preto","Marrom","Vermelho","Laranja","Amarelo","Verde","Azul","Violeta","Cinza","Branco","diferente","do valor","comercial"};

uint identificar_tolerancia(){ //Retorna o valor encontrado

  bool encontrado = false;
  uint valor_comercial = 0;
  float faixa_superior = 0;
  float faixa_inferior = 0;

  uint valores_disponiveis[56] = {510,560,620,680,750,820,910,1000,1100,1200,
                              1300,1500,1600,1800,2000,2200,2400,2700,3000,
                              3300,3600,3900,4300,4700,5100,5600,6200,6800,
                              7500,8200,9100,10000,11000,12000,13000,15000,
                              16000,18000,20000,22000,24000,27000,30000,
                              33000,36000,39000,43000,47000,51000,56000,
                              62000,68000,75000,82000,91000,100000};
  
  for(int i=0;i<56 && !encontrado;i++){
    faixa_inferior = 0.95*valores_disponiveis[i];
    faixa_superior = 1.05*valores_disponiveis[i];

    if(R_x>=faixa_inferior && R_x<=faixa_superior){
      valor_comercial = valores_disponiveis[i];
      indice_tabela = i;
      encontrado = true;
    };

  }; 
 
  if(!encontrado){
    printf("\t\n!!Atenção!!\nValor fora da faixa de tolerância dos padrões comerciais\n"); 
    indice_tabela=56;  
  };

  printf("\nValor comercial encontrado: %d",valor_comercial);

  return valor_comercial;
  
};

void encontrar_faixas(uint Rx_int){ 
  
  //MODIFICAÇÃO
  char valor_Rx[9]; //Buffer para armazenar o Rx e identificar os valores separadamente
  int ncasa=0; //vai armazenar a quatidade de casas o número possui
  
  //Código para descobrir as faixas de valores
  //printf("%d\n",Rx_int);
  sprintf(valor_Rx, "%d", Rx_int); //converte R_x em string

  ncasa = strlen(valor_Rx); //Armazena a informação do número de casas

  if(Rx_int==0){
    faixa1=10;
    faixa2=11;
    faixa3=12;
  }else{
    //Encontrando faixa 3
    if(ncasa < 3){
      faixa3=0;
    }else{
      faixa3 = ncasa-2;
    };


    if(ncasa==1){
      faixa1=0;
      faixa2=valor_Rx[0]-48;
    }else{
      //Encontrando faixa 1
      faixa1 = valor_Rx[0] - 48; //Converter o valor de char para int
      //Encontrando faixa 2
      faixa2 = valor_Rx[1] - 48;
    };
  };
  
  printf("\nCódigo de cores:\n\t1º faixa: %s - 2º faixa: %s - 3º faixa: %s",tabela_cores[faixa1],tabela_cores[faixa2],tabela_cores[faixa3]); //Remover essa linha

};

int main()
{
  stdio_init_all();
 
  adc_init();
  adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica

  float tensao;
  char str_x[5]; // Buffer para armazenar a string
  char str_y[5]; // Buffer para armazenar a string
  uint valor_comercial=0;

  bool cor = true;
  // I2C Initialisation. Using it at 400Khz.
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
    adc_select_input(2); // Seleciona o ADC para eixo X. O pino 28 como entrada analógica

    float soma = 0.0f;
    for (int i = 0; i < 500; i++)
    {
      soma += adc_read();
      sleep_ms(1);
    }
    float media = soma / 500.0f;

    // Fórmula simplificada: R_x = R_conhecido * ADC_encontrado /(ADC_RESOLUTION - adc_encontrado)
    R_x = (R_conhecido * media) / (ADC_RESOLUTION - media);
    printf("\nValor encontrado pelo ohmimetro: %f",R_x);
    //sprintf(str_x, "%1.0f", media); // Converte o inteiro em string
    //sprintf(str_y, "%1.0f", R_x);   // Converte o float em string

    //Joga essas funções no loop
    valor_comercial = identificar_tolerancia();
    encontrar_faixas(valor_comercial);
    // cor = !cor;
    //  Atualiza o conteúdo do display com animações
    ssd1306_fill(&ssd, !cor);                          // Limpa o display
    ssd1306_rect(&ssd, 3, 3, 122, 60, cor, !cor);      // Desenha um retângulo ok
    ssd1306_line(&ssd, 3, 16, 123, 16, cor);           // Desenha uma linha ok
    ssd1306_line(&ssd, 3, 28, 123, 28, cor);           // Desenha uma linha
    ssd1306_line(&ssd, 3, 40, 123, 40, cor);           // Desenha uma linha
    ssd1306_line(&ssd, 3, 52, 123, 52, cor);           // Desenha uma linha
    ssd1306_draw_string(&ssd, "OHMIMETRO", 28, 6); // Desenha uma string
    ssd1306_draw_string(&ssd, "Res.:", 10, 19);  // Desenha uma string
    ssd1306_draw_string(&ssd, tabela_comercial[indice_tabela], 55, 19);  // Desenha uma string
    ssd1306_draw_string(&ssd, "ohm", 95, 19);  // Desenha uma string
    ssd1306_draw_string(&ssd, "1", 12, 31);          // Desenha uma string
    ssd1306_draw_string(&ssd, tabela_cores[faixa1], 38, 31);
    ssd1306_draw_string(&ssd, "2", 12, 43);          // Desenha uma string
    ssd1306_draw_string(&ssd, tabela_cores[faixa2], 38, 43); 
    ssd1306_draw_string(&ssd, "3", 12, 54);          // Desenha uma string
    ssd1306_draw_string(&ssd, tabela_cores[faixa3], 38, 54);
    ssd1306_line(&ssd, 28, 29, 28, 62, cor);           // Desenha uma linha vertical
    ssd1306_send_data(&ssd);                           // Atualiza o display
    sleep_ms(700);
  };
};
