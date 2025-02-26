# Semáforo Reativo de Suplementação

## Descrição

Projeto para semáforo suplementar em faixas de pedestres utilizando o Raspberry Pi Pico W (RP2040).  
O sistema é reativo – acionado apenas quando necessário – e possui duas versões:

- **Com Display:** Exibe a contagem regressiva em um display OLED (SSD1306) via I2C.
- **Sem Display:** Opera apenas com sinais luminosos (LEDs) e sonoros (buzzer via PWM).

## Objetivos

- Melhorar a segurança nas travessias de pedestres.
- Oferecer uma solução de baixo custo e eficiente.
- Garantir acessibilidade com sinalização sonora.
- Operar de forma reativa, acionando o sistema apenas quando necessário.
- Utilizar temporizadores de até 30 segundos e aplicação ideal em distâncias de até 12 metros.

## Componentes Utilizados

- **Raspberry Pi Pico W (RP2040)**
- **LEDs:**
  - LED Verde (GPIO 11)
  - LED Vermelho (GPIO 13)
- **Botões:**
  - Botão A (GPIO 5) – ativa o sistema sem sinal sonoro.
  - Botão B (GPIO 6) – ativa o sistema com sinal sonoro.
- **Buzzer:**  
  - (GPIO 21) – controlado via PWM.
- **Display OLED (apenas na versão com display):**
  - SDA (GPIO 14)
  - SCL (GPIO 15) – comunicação I2C.

## Configuração de Hardware

**Com Display:**  
- **RP2040:** Processador central.  
- **LEDs:** Acionam os sinais (verde e vermelho).  
- **Botões:** Iniciam o ciclo (com debounce via alarmes).  
- **Buzzer:** Emite sinal sonoro via PWM.  
- **Display OLED:** Exibe a contagem regressiva (I2C).

**Sem Display:**  
- Mesmo esquema, porém sem o módulo OLED.

### Conexões

- **LED Verde:** GPIO 11.
- **LED Vermelho:** GPIO 13.
- **Botão A:** GPIO 5 (entrada com pull-up).
- **Botão B:** GPIO 6 (entrada com pull-up).
- **Buzzer:** GPIO 21 (PWM).
- **Display OLED (versão com display):**
  - SDA: GPIO 14.
  - SCL: GPIO 15.

## Instalação e Compilação

1. Configure o ambiente de desenvolvimento do Raspberry Pi Pico (SDK do Pico e CMake).
2. Clone este repositório.
3. Compile o código utilizando CMake e o SDK do Pico.
4. Carregue o arquivo .uf2 gerado para o Raspberry Pi Pico.

## Autor
Hugo Martins Santana (TIC370101267)