# Ohmimetro com Sinalização de Cores e Padrão Comercial
__Tarefa 2 - Aula Síncrona 22/04__<br>
Repositório destinando ao desenvolvimento da Tarefa Eletrônica da Unidade 1 da Fase 2 da Residência em Software Embarco - Embarca Tech TIC 37

__Responsável pelo desenvolvimento:__
Andressa Sousa Fonseca

## Descrição Da Tarefa 
__Aplicando conceitos de Eletrônica com o uso da BitDogLab__  <br>

__Os Componentes necessários para a execução da atividade são:__
1) Matriz 5x5 de LEDs (endereçáveis) WS2812, conectada à GPIO 7;
2) Botão A conectado à GPIO 6;
3) Display SSD1306 conectado via I2C (GPIO 14 e GPIO15);
4) Protoboard;
5) Jumpers MXF;
6) Resistores da série E24.

__Uma breve explicação do desenvolvimento e resultados obtidos podem ser vistos no vídeo endereçado no seguine link: [Aplicação de Interaces de Comunicação Serial](https://youtu.be/67-9-NYzABU?feature=shared).__

## Funcionalidades 

### 1. Calcula o valor de um resistor desconhecido

O projeto permite identificar o valor de um resistor desconhecido, com base em um resistor de referência, uma tensão fornecida e as leituras do conversor analógico digital. Assim, tenho uma resistência conhecida e a tensão total que passa pelos resistores, utilizando-se a fórmula de dividão de tensão, o código consegue calcular o valor do resistor desconhecido a apartir da tensão lida sobre ele. A relação entre os valores do conversor e a tensão é a de que a tensão máxima de 3.3V corresponde ao valor máximo de 4095. Assim, outras leituras podem ter um valor correspondente de tensão relacionado. A fórmula usado no código foi:
<div align="center">
  <img src="![image](https://github.com/user-attachments/assets/7aa8629e-c27b-4fcd-9467-bb6d4c6d9b34)" alt="line ending" width="300"/>
</div>
A monatagem do ohmímetro foi feita com os pinos GPIO 28, GND  e 3.3V, presentes nabit DogLab, com os resistores dispostos em série. A alimentação de 3.3V foi conectada à ponta de um resistor, e o GND à outra extremidade da série. Já o GPIO 38 foi conectado no encontro entre os dois resistores, paa assim medir a tesnão em cima do resistor desconhecido. A configuração das conexões pode ser vista abaixo.
<div align="center">
  <img src="![image](https://github.com/user-attachments/assets/a59dfc9b-d5f4-403e-8d4c-dbc861c57757)" alt="line ending" width="300"/>
</div>

### 2. Identifica o Valor Comercial
Os valores encontrados no cálculo são tratados para identificar o valor comercial correspondente na série E24. Assim, se o valor estiver dentro da tolerância de algum dos valores de mercado, o valor comercial correspondente é exibido.

### 2. Código de Cores
Com base no valor comercial identificado, o display e a matriz de leds informam a cores do resistor identificado. O display apresenta o nome da cor de cada faixa e a matriz de leds exibe um figura retangular, símbolo dos resistores, com três cores internas que representam as três faixas.
<div align="center">
  <img src="![image](https://github.com/user-attachments/assets/b4261f0f-c201-4c60-b070-b55b07412163)" alt="line ending" width="300"/>
</div>
