#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#define ADC_PIN 28 // GPIO para o voltímetro
#define Botao_A 5  // GPIO para botão A

int R_conhecido = 10000;   // Resistor de 10k ohm
float R_x = 2245.8;           // Resistor desconhecido
float ADC_VREF = 3.31;     // Tensão de referência do ADC
int ADC_RESOLUTION = 4095; // Resolução do ADC (12 bits)

void encontrar_faixas(){ //Passa o Rx lido por parâmetro
  
  //MODIFICAÇÃO
  int Rx_int;
  char valor_Rx[9]; //Buffer para armazenar o Rx e identificar os valores separadamente
  int ncasa=0; //vai armazenar a quatidade de casas o número possui
  int faixa1=0; //Ordem: 0 - preto, 1 - marrom, 2 - vermelho, 3 - laranja .....
  int faixa2=0;
  int faixa3=0;
  
  //Código para descobrir as faixas de valores
  Rx_int = 2245.8;
  printf("%d",Rx_int);
  sprintf(valor_Rx, "%d", Rx_int); //converte R_x em string
  puts(valor_Rx);

  ncasa = strlen(valor_Rx); //Armazena a informação do número de casas
  
  
  //Encontrando faixa 3
  if(ncasa < 3){
    faixa3=0;
  }else{
    faixa3 = ncasa-1;
  };

  //Encontrando faixa 1
  faixa1 = valor_Rx[0] - 48; //Converter o valor de char para int

  //Encontrando faixa 2
  faixa2 = valor_Rx[1] - 48;

  printf(",,,");
  printf("faixa 1 - %d; faixa 2 - %d; faixa 3 - %d",faixa1,faixa2,faixa3);

};

int main()
{
  stdio_init_all();
 
  adc_init();
  adc_gpio_init(ADC_PIN); // GPIO 28 como entrada analógica

  float tensao;
  char str_x[5]; // Buffer para armazenar a string
  char str_y[5]; // Buffer para armazenar a string


  bool cor = true;
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

    encontrar_faixas();
    
    printf("..."); 
      
    sprintf(str_x, "%1.0f", media); // Converte o inteiro em string
    sprintf(str_y, "%1.0f", R_x);   // Converte o float em string

  };
};
