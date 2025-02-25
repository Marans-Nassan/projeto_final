#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/pwm.h"

#define led_verde 11
#define led_vermelho 13
#define botao_a 5
#define botao_b 6
#define buzzer_a 21
#define divisor 125.0
#define periodo_a 499
#define periodo_b 1051

uint8_t pinos[3] = {11, 13, 21};
static uint8_t verde = led_verde;
static uint8_t vermelho = led_vermelho;
static uint8_t slice;
absolute_time_t alarm_a;
absolute_time_t alarm_b;

void led_e_buz_init();
void botinit();
void gpio_irq_handler(uint gpio, uint32_t events);
int64_t turn_off_leds(alarm_id_t id, void *user_data);
int64_t turn_off_pwm(alarm_id_t id, void *user_data);
int64_t turn_on_pwm(alarm_id_t id, void *user_data);
int64_t pressionado_botao_a(alarm_id_t id, void *user_data);
int64_t pressionado_botao_b(alarm_id_t id, void *user_data);
int64_t atraso_botao(alarm_id_t id, void *user_data);
bool alternando_interrupcao(bool true_false);
void alarm_buzzers();
void pwm_setup(uint32_t duty_cycle);

int main(){

led_e_buz_init();
botinit();
pwm_setup(0);
alternando_interrupcao(true);
stdio_init_all();

    while (true) {
        if (gpio_get(botao_a)) cancel_alarm(alarm_a);
        if (gpio_get(botao_b)) cancel_alarm(alarm_b);
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
            if(gpio_get(botao_a) == 0)alarm_a = add_alarm_in_ms(500, pressionado_botao_a, NULL, false);
        }
        if(gpio == botao_b){
            if(gpio_get(botao_b) == 0)alarm_b = add_alarm_in_ms(3000, pressionado_botao_b, NULL, false);
        }    
    }
}

int64_t turn_off_leds(alarm_id_t id, void *user_data){
    uint8_t led = *(uint8_t*)user_data; 
    gpio_put(led, 0);
    if(gpio_get(vermelho) == 0)alternando_interrupcao(false);
    return 0;
}

int64_t turn_off_pwm(alarm_id_t id, void *user_data){
    pwm_set_gpio_level(buzzer_a, 0);
return 0;
}

int64_t turn_on_pwm(alarm_id_t id, void *user_data){
    if(gpio_get(verde) == 1)pwm_set_gpio_level(buzzer_a, (periodo_a * 50) /100);       
        else{
            pwm_set_wrap(slice, periodo_b);
            pwm_set_gpio_level(buzzer_a, (periodo_b * 50) /100);
        }

return 0;    
}

int64_t pressionado_botao_a(alarm_id_t id, void *user_data){
    gpio_put(led_verde, 1);
    gpio_put(led_vermelho, 1);
    alternando_interrupcao(false);           
    add_alarm_in_ms(6000, turn_off_leds, &verde, false);
    add_alarm_in_ms(16000, turn_off_leds, &vermelho, false);
    add_alarm_in_ms(21000, atraso_botao, NULL, false);
}

int64_t pressionado_botao_b(alarm_id_t id, void *user_data){
    pwm_set_wrap(slice, periodo_a);
    gpio_put(led_verde, 1);
    gpio_put(led_vermelho, 1);
    alternando_interrupcao(false);
    alarm_buzzers();
    add_alarm_in_ms(6000, turn_off_leds, &verde, false);
    add_alarm_in_ms(18000, turn_off_leds, &vermelho, false);
    add_alarm_in_ms(23000, atraso_botao, NULL, false);  
}

bool alternando_interrupcao(bool true_false){
    gpio_set_irq_enabled_with_callback(botao_a, GPIO_IRQ_EDGE_FALL, true_false, gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botao_b, GPIO_IRQ_EDGE_FALL, true_false, gpio_irq_handler);
return 0;
}

int64_t atraso_botao(alarm_id_t id, void *user_data){
    alternando_interrupcao(true);
}

void pwm_setup(uint32_t duty_cycle){ 
    gpio_set_function(buzzer_a, GPIO_FUNC_PWM);
    slice = pwm_gpio_to_slice_num(buzzer_a);
    pwm_set_clkdiv(slice, divisor);
    pwm_set_wrap(slice, periodo_a);
    pwm_set_gpio_level(buzzer_a, periodo_a * duty_cycle / 100);
    pwm_set_enabled(slice, true);
}

void alarm_buzzers(){
    pwm_set_gpio_level(buzzer_a, periodo_a * 50 /100);
    add_alarm_in_ms(1000, turn_off_pwm, NULL, false);
    add_alarm_in_ms(4000, turn_on_pwm, NULL, false);
    add_alarm_in_ms(5000, turn_off_pwm, NULL, false);
    add_alarm_in_us(6000040, turn_on_pwm, NULL, false);
    add_alarm_in_ms(7000, turn_off_pwm, NULL, false);
    add_alarm_in_ms(15000, turn_on_pwm, NULL, false);
    add_alarm_in_ms(16000, turn_off_pwm, NULL, false);
}