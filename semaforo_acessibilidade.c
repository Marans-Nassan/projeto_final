#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"
// Macros.
#define led_verde 11
#define led_vermelho 13
#define botao_a 5
#define botao_b 6
#define buzzer_a 21
#define divisor 125.0
#define periodo_a 500
#define periodo_b 1052
#define SDA 14
#define SCL 15
#define endereco 0x3c
#define I2C_PORT i2c1
ssd1306_t ssd;
// Variáveis.
static volatile bool contagem_regressiva = 0;
static volatile bool som_estado;
uint8_t pinos[3] = {11, 13, 21};
static uint8_t verde = led_verde;
static uint8_t vermelho = led_vermelho;
static uint8_t slice;
static uint8_t tempo_amarelo = 0;
static uint8_t tempo_vermelho = 0;
absolute_time_t alarm;
// Protótipos.
void led_e_buz_init(); // Responsável por iniciar os leds e buzzer.
void botinit(); // Responsável por iniciar os botões.
void gpio_irq_handler(uint gpio, uint32_t events); // Administra as ações quando a interrupção é ativada.
bool alternando_interrupcao(bool true_false); // Responsável por garantir que o botão só será acionado quando terminar a contagem.
void pwm_setup(uint32_t duty_cycle); // Responsável por iniciar e configurar o PWM.
void i2cinit(); // Responsável por iniciar o i2c.
void oledinit(); // Responsável por iniciar o OLED ssd1306.
void oledisplay(uint8_t segundos); // Responsável por apresentar a contagem no display.
void contagem(); // Responsável por toda operação dentro do loop principal.
void limpar_tela(); // Responsável por limpar a tela do display ao concluir toda a operação.
int64_t pressionado_botao_a (alarm_id_t id, void *user_data);
int64_t pressionado_botao_b (alarm_id_t id, void *user_data);
// Função principal.
int main(){

    led_e_buz_init();
    botinit();
    pwm_setup(0);
    i2cinit();
    oledinit();
    gpio_set_irq_enabled_with_callback(botao_a, GPIO_IRQ_EDGE_FALL, true, gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botao_b, GPIO_IRQ_EDGE_FALL, true, gpio_irq_handler);
    stdio_init_all();

    while (true) {
        contagem();
        limpar_tela();
    }
}

void led_e_buz_init(){
    for(uint8_t i = 0; i < 3 ;i++){
        gpio_init(pinos[i]);
        gpio_set_dir(pinos[i], GPIO_OUT);
        gpio_put(pinos[i], 0);
    }
}

void botinit(){
    for(uint8_t botoes = 5 ; botoes < 7 ; botoes++){
        gpio_init(botoes);
        gpio_set_dir(botoes, GPIO_IN);
        gpio_pull_up(botoes);
    }
}
// Altera-se o tempo_amarelo e o vermelho para definir de quando vai começar a contagem do display para cada cor do sinal.
void gpio_irq_handler(uint gpio, uint32_t events){
    if(gpio == botao_a || gpio == botao_b) {
        if(gpio == botao_a){ // Este botão é o de uso comum. Sem som.
            if(gpio_get(botao_a)== 0) alarm = add_alarm_in_ms(300, pressionado_botao_a, NULL, false);
            else cancel_alarm(alarm);
        }
        
        if(gpio == botao_b){ // Este botão é o de acessibilidade. Com som.
            if(gpio_get(botao_b) == 0) alarm = add_alarm_in_ms(3000, pressionado_botao_b, NULL, false);
            else cancel_alarm(alarm);
        }
    }
}

bool alternando_interrupcao(bool true_false){    
    gpio_set_irq_enabled_with_callback(botao_a, GPIO_IRQ_EDGE_FALL, true_false, gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botao_b, GPIO_IRQ_EDGE_FALL, true_false, gpio_irq_handler);
return 0;
}

void pwm_setup(uint32_t duty_cycle){
    gpio_set_function(buzzer_a, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(buzzer_a);
    pwm_set_clkdiv(slice, divisor);
    pwm_set_wrap(slice, periodo_a);
    pwm_set_gpio_level(buzzer_a, periodo_a * duty_cycle / 100);
    pwm_set_enabled(slice, true);
}

void i2cinit(){
    i2c_init(I2C_PORT, 400*1000);
        for(uint8_t i = 14; i < 16; i++){
            gpio_set_function(i, GPIO_FUNC_I2C);
            gpio_pull_up(i);
        }  
}

void oledinit(){
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
}

void oledisplay(uint8_t segundos){
    char tempo[3];
    tempo[0] = '0' + (segundos / 10);
    tempo[1] = '0' + (segundos % 10);
    tempo[2] = '\0';
    ssd1306_rect(&ssd, 1, 1, WIDTH - 2, HEIGHT - 2, true, false);
    ssd1306_draw_string(&ssd, tempo, 58, 25 );
    ssd1306_send_data(&ssd);
}
// Os valores de 0 ~ 100  no segundo argumento de pwm_set_gpio_level definem o volume do som. (0 = sem som. 100 = volume máximo)
void contagem(){

    if(contagem_regressiva && som_estado){ // Verifica se o botao_b foi pressionado.
        if(gpio_get(verde) == 1){ 
            for (uint8_t i = tempo_amarelo ; i > 0; i--){
                oledisplay(i);
                if(i == 5)pwm_set_gpio_level(buzzer_a, periodo_a * 10 /100);
                if(i == 4)pwm_set_gpio_level(buzzer_a, 0);
                if(i == 2)pwm_set_gpio_level(buzzer_a, periodo_a * 40 /100);
                if(i == 1)pwm_set_gpio_level(buzzer_a, 0);
                sleep_ms(1000);
            } 
                gpio_put(verde, 0);   
        }  

        if(gpio_get(vermelho) == 1 && gpio_get(verde) == 0){ 
            pwm_set_wrap(slice, periodo_b);
            for (uint8_t i = tempo_vermelho ; i > 0; i--){
                oledisplay(i);
                if(i == 12)pwm_set_gpio_level(buzzer_a, periodo_b * 65 /100);
                if(i == 11)pwm_set_gpio_level(buzzer_a, 0);
                if(i == 5)pwm_set_gpio_level(buzzer_a, periodo_b * 90 /100);
                if(i == 3)pwm_set_gpio_level(buzzer_a, 0);
                sleep_ms(1000);
            }
                gpio_put(vermelho, 0);
                som_estado = 0; 
        }
    }
        else if (contagem_regressiva){ // Verifica se o botao_a foi pressionado.
            if(gpio_get(verde) == 1){ 
                for (uint8_t i = tempo_amarelo ; i > 0; i--){
                    oledisplay(i);
                    sleep_ms(1000);
                } 
                gpio_put(verde, 0);   
            }  

            if(gpio_get(vermelho) == 1 && gpio_get(verde) == 0){ 
                for (uint8_t i = tempo_vermelho ; i > 0; i--){
                    oledisplay(i);
                    sleep_ms(1000);
                }
                gpio_put(vermelho, 0); 
            }
        }
    contagem_regressiva = 0;  
    alternando_interrupcao(true);      
}

void limpar_tela(){
    ssd1306_fill(&ssd, false); 
    ssd1306_send_data(&ssd);
}

int64_t pressionado_botao_a (alarm_id_t id, void *user_data){
    if(gpio_get(botao_a))
    return 0;

    tempo_amarelo = 5;
    tempo_vermelho = 10;
    gpio_put(led_verde, 1);
    gpio_put(led_vermelho, 1);
    contagem_regressiva = 1;
    alternando_interrupcao(false); 
return 0; 
}

int64_t pressionado_botao_b (alarm_id_t id, void *user_data){
    if(gpio_get(botao_b))
    return 0;

    pwm_set_wrap(slice, periodo_a);
    tempo_amarelo = 5;
    tempo_vermelho = 12;
    gpio_put(led_verde, 1);
    gpio_put(led_vermelho, 1);
    contagem_regressiva = 1;
    alternando_interrupcao(false);
    som_estado = 1;
return 0;
}