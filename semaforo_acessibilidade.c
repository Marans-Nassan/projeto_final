#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

#define led_verde 11
#define led_vermelho 13
#define botao_a 5
#define botao_b 6
#define buzzer_a 21
#define buzzer_b 10

static bool interrupcao = 1;
uint8_t pin[4] = {11, 10, 13, 21};

void led_e_buz_init();
void botinit();
void gpio_irq_handler(uint gpio, uint32_t events);
int64_t turn_off_leds(alarm_id_t id, void *user_data);
int64_t turn_on_botoes(alarm_id_t id, void *user_data);
void alternando_interrupcao();

int main(){

led_e_buz_init(led_verde);
led_e_buz_init(led_vermelho);
led_e_buz_init(buzzer_a);
led_e_buz_init(buzzer_b);
botinit();

gpio_set_irq_enabled_with_callback(botao_a, GPIO_IRQ_EDGE_FALL, true, gpio_irq_handler);
gpio_set_irq_enabled_with_callback(botao_b, GPIO_IRQ_EDGE_FALL, true, gpio_irq_handler);

    stdio_init_all();

    while (true) {

    }
}

void led_e_buz_init(){
    for(uint8_t i = 0; i < 4 ;i++){
        gpio_init();
        gpio_set_dir(, GPIO_OUT);
        gpio_put(, 0);
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
    uint64_t current_time = to_us_since_boot(get_absolute_time())* 1000000;
    uint64_t last_time = 0;
    static uint8_t verde = led_verde;
    static uint8_t vermelho = led_vermelho;
    if(gpio == botao_a || gpio == botao_b && current_time - last_time > 3){
        if(gpio == botao_a){
            gpio_put(led_verde, 1);
            gpio_put(led_vermelho, 1);
            alternando_interrupcao();
            add_alarm_in_ms(3000, turn_off_leds, &verde, false);
            add_alarm_in_ms(6000, turn_off_leds, &vermelho, false);
            add_alarm_in_ms(7500, turn_on_botoes, NULL, false);
        }
    }
    last_time = current_time;
}

int64_t turn_off_leds(alarm_id_t id, void *user_data){
uint8_t led = *(uint8_t*)user_data; 
gpio_put(led, 0);
return 0;
}

int64_t turn_on_botoes(alarm_id_t id, void *user_data){
    alternando_interrupcao();
}

void alternando_interrupcao(){
    interrupcao = !interrupcao;
    
    gpio_set_irq_enabled_with_callback(botao_a, GPIO_IRQ_EDGE_FALL, interrupcao, gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(botao_b, GPIO_IRQ_EDGE_FALL, interrupcao, gpio_irq_handler);
}
