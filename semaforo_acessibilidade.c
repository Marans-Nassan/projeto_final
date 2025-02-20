#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "ssd1306.h"
#include "font.h"

#define led_verde 11
#define led_vermelho 13
#define botao_a 5
#define botao_b 6
#define buzzer_a 21
#define divisor 125.0
#define periodo 999
#define SDA 14
#define SCL 15
#define endereco 0x3c
#define I2C_PORT i2c1

static bool true_false = 1;
static bool contagem_regressiva;
uint8_t pinos[3] = {11, 13, 21};
static uint8_t verde = led_verde;
static uint8_t vermelho = led_vermelho;
static uint8_t slice;
static uint8_t tempo_amarelo = 0;
static uint8_t tempo_vermelho = 0;
ssd1306_t ssd;

void led_e_buz_init();
void botinit();
void gpio_irq_handler(uint gpio, uint32_t events);
int64_t turn_off_leds(alarm_id_t id, void *user_data);
int64_t turn_off_pwm(alarm_id_t id, void *user_data);
int64_t turn_on_pwm(alarm_id_t id, void *user_data);
void alternando_interrupcao();
void alarm_buzzers();
void pwm_setup(uint32_t duty_cycle);
void i2cinit();
void oledinit();
void oledisplay(uint8_t segundos);
void contagem();
void borda();

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
        ssd1306_fill(&ssd, false); 
        ssd1306_send_data(&ssd);
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

void gpio_irq_handler(uint gpio, uint32_t events){
    if(gpio == botao_a || gpio == botao_b) {
        if(gpio == botao_a){
            tempo_amarelo = 15;
            tempo_vermelho = 15;
            gpio_put(led_verde, 1);
            gpio_put(led_vermelho, 1);
            contagem_regressiva = 1;
            alternando_interrupcao();           
            add_alarm_in_ms(15000, turn_off_leds, &verde, false);
            add_alarm_in_ms(30000, turn_off_leds, &vermelho, false);
        }
        if(gpio == botao_b){
            pwm_set_wrap(slice, 999);
            tempo_amarelo = 20;
            tempo_vermelho = 40;
            gpio_put(led_verde, 1);
            gpio_put(led_vermelho, 1);
            alternando_interrupcao();
            alarm_buzzers();
            add_alarm_in_ms(20000, turn_off_leds, &verde, false);
            add_alarm_in_ms(60000, turn_off_leds, &vermelho, false);
        }
    }
}

int64_t turn_off_leds(alarm_id_t id, void *user_data){
    uint8_t led = *(uint8_t*)user_data; 
    gpio_put(led, 0);
        if(gpio_get(vermelho) == 0)alternando_interrupcao();
return 0;
}

int64_t turn_off_pwm(alarm_id_t id, void *user_data){
    pwm_set_gpio_level(buzzer_a, 0);
return 0;
}

int64_t turn_on_pwm(alarm_id_t id, void *user_data){
    if(gpio_get(verde) == 1)pwm_set_gpio_level(buzzer_a, 100);       
        else{
            pwm_set_wrap(slice, 2999);
            pwm_set_gpio_level(buzzer_a, 100);
        }

return 0;    
}

void alternando_interrupcao(){
    true_false = !true_false;
    
    gpio_set_irq_enabled_with_callback(botao_a, GPIO_IRQ_EDGE_FALL, true_false, gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botao_b, GPIO_IRQ_EDGE_FALL, true_false, gpio_irq_handler);
}

void pwm_setup(uint32_t duty_cycle){
    gpio_set_function(buzzer_a, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(buzzer_a);
    pwm_set_clkdiv(slice, divisor);
    pwm_set_wrap(slice, periodo);
    pwm_set_gpio_level(buzzer_a, periodo * duty_cycle / 100);
    pwm_set_enabled(slice, true);
}

void alarm_buzzers(){
    pwm_set_gpio_level(buzzer_a, 50);
    add_alarm_in_ms(200, turn_off_pwm, NULL, false);
    add_alarm_in_ms(15000, turn_on_pwm, NULL, false);
    add_alarm_in_ms(16000, turn_off_pwm, NULL, false);
    add_alarm_in_ms(17000, turn_on_pwm, NULL, false);
    add_alarm_in_ms(18000, turn_off_pwm, NULL, false);
    add_alarm_in_us(20000040, turn_on_pwm, NULL, false);
    add_alarm_in_ms(20600, turn_off_pwm, NULL, false);
    add_alarm_in_ms(50000, turn_on_pwm, NULL, false);
    add_alarm_in_ms(52000, turn_off_pwm, NULL, false);
    add_alarm_in_ms(54000, turn_on_pwm, NULL, false);
    add_alarm_in_ms(56000, turn_off_pwm, NULL, false);
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

void contagem(){
    if(contagem_regressiva){
        if(gpio_get(verde) == 1){ 
            for (uint8_t i = tempo_amarelo ; i > 0; i--){
                oledisplay(i);
                sleep_ms(1000);
            }    
        }  

        if(gpio_get(vermelho) == 1 && gpio_get(verde) == 0){ 
            for (uint8_t i = tempo_vermelho ; i > 0; i--){
                oledisplay(i);
                sleep_ms(1000);
            }    
        }
    } 
    contagem_regressiva = 0;       
}

void borda(){
    ssd1306_rect(&ssd, 4, 4, 124, 60, true, false);
}