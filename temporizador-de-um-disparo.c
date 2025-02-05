#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"

// Definição dos pinos dos LEDs
#define LED_AZUL 11
#define LED_VERMELHO 12
#define LED_VERDE 13

// Definição do pino do botão
#define BOTAO 5

// Definição do período do temporizador em milissegundos
#define PERIODO_MS 3000 

// Variável de controle para evitar múltiplas ativações do botão
volatile bool processo_em_execucao = false;

// Protótipos das funções
void inicializar_hardware();
void ligar_leds();
int64_t desligar_led_azul_callback(alarm_id_t id, void *user_data);
int64_t desligar_led_vermelho_callback(alarm_id_t id, void *user_data);
int64_t desligar_led_verde_callback(alarm_id_t id, void *user_data);
void botao_callback(uint gpio, uint32_t events);

int main() {
    stdio_init_all();
    
    // Inicializa LEDs e Botão
    inicializar_hardware();

    printf("Sistema inicializado. Pressione o botão para iniciar a sequência.\n");

    // Loop principal apenas para manter o programa rodando
    while (true) {
        sleep_ms(1000);
    }
}

// Inicializa os LEDs e o botão
void inicializar_hardware() {
    int leds[] = {LED_AZUL, LED_VERMELHO, LED_VERDE};

    for (int i = 0; i < 3; i++) {
        gpio_init(leds[i]);
        gpio_set_dir(leds[i], GPIO_OUT);
        gpio_put(leds[i], 0);  // Inicializa desligado
    }

    gpio_init(BOTAO);
    gpio_set_dir(BOTAO, GPIO_IN);
    gpio_pull_up(BOTAO);  // Ativa pull-up interno

    // Configura o botão para detecção de borda de descida
    gpio_set_irq_enabled_with_callback(BOTAO, GPIO_IRQ_EDGE_FALL, true, &botao_callback);
}

// Callback acionado quando o botão é pressionado (com debounce)
void botao_callback(uint gpio, uint32_t events) {
    static uint32_t ultimo_tempo = 0;
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());

    // Verifica se passaram pelo menos 200ms desde o último clique para evitar bouncing
    if (tempo_atual - ultimo_tempo > 200) {
        if (!processo_em_execucao) {
            processo_em_execucao = true;
            printf("Botão pressionado! Iniciando sequência dos LEDs...\n");
            ligar_leds();
        }
    }
    ultimo_tempo = tempo_atual;
}


// Liga todos os LEDs e inicia o temporizador para desligar o azul após 3s
void ligar_leds() {
    gpio_put(LED_AZUL, 1);
    gpio_put(LED_VERMELHO, 1);
    gpio_put(LED_VERDE, 1);

    add_alarm_in_ms(PERIODO_MS, desligar_led_azul_callback, NULL, false);
}

// Callback para desligar o LED azul e iniciar o timer para desligar o LED vermelho
int64_t desligar_led_azul_callback(alarm_id_t id, void *user_data) {
    gpio_put(LED_AZUL, 0);
    printf("LED azul desligado.\n");

    add_alarm_in_ms(PERIODO_MS, desligar_led_vermelho_callback, NULL, false);
    return 0;
}

// Callback para desligar o LED vermelho e iniciar o timer para desligar o LED verde
int64_t desligar_led_vermelho_callback(alarm_id_t id, void *user_data) {
    gpio_put(LED_VERMELHO, 0);
    printf("LED vermelho desligado.\n");

    add_alarm_in_ms(PERIODO_MS, desligar_led_verde_callback, NULL, false);
    return 0;
}

// Callback para desligar o LED verde e finalizar a sequência
int64_t desligar_led_verde_callback(alarm_id_t id, void *user_data) {
    gpio_put(LED_VERDE, 0);
    printf("LED verde desligado. Sequência finalizada.\n");

    // Habilita novamente a detecção do botão
    processo_em_execucao = false;
    return 0;
}
