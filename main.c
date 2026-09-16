#include "main.h"
#include "tm1637.h"

volatile uint32_t tickCount;
uint32_t last_display_update;
uint16_t counter;
char lastKey;
uint32_t lastScanTime;

uint8_t isTimerWorking;

uint16_t firstNum;
uint16_t secondNum;

char op;


void osSystickHandler(void) {
  tickCount++;
}


void initGPIO() {
  // Включаем тактирование GPIOA и GPIOB
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;

  // Настраиваем PA5 как выход
  GPIOA->MODER = (GPIOA->MODER & ~(3 << 10)) | (1 << 10);
  GPIOA->OTYPER &= ~(1 << 5);
  GPIOA->OSPEEDR |= (1 << 10);
}

void initUSART2() {
  // Включаем тактирование USART2
  RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

  // Настраиваем PA2 и PA3 в альтернативный режим
  GPIOA->MODER = (GPIOA->MODER & ~(0xF << 4)) | (0xA << 4);
  GPIOA->AFR[0] = (GPIOA->AFR[0] & ~(0xFF << 8)) | (1 << 8) | (1 << 12);

  // Настраиваем USART2
  USART2->BRR = 417; // 48MHz/115200
  USART2->CR1 = USART_CR1_TE | USART_CR1_UE;
}

void initSysTick() {
  SysTick->LOAD = 47999; // 1ms при 48MHz
  SysTick->VAL = 0;
  SysTick->CTRL = (1 << 2) | (1 << 1) | (1 << 0);
}

int _write(int file, uint8_t *ptr, int len) {
  for (int i = 0; i < len; i++) {
    while (!(USART2->ISR & USART_ISR_TXE));
    USART2->TDR = ptr[i];
  }
  return len;
}

void checkTickCount() {
  if ((tickCount % 1000) == 0) {
    printf("tickCount = %d!\n", tickCount++);
  }
}

void ledOn() {
  GPIOA->ODR |= (1 << 5);
}

void ledOff() {
  GPIOA->ODR &= ~(1 << 5);
}

int main(void) {
  firstNum = 0;
  secondNum = 0;
  counter = 0;

  initGPIO();
  initUSART2();
  initSysTick();
  initKeyboard();
  tm1637_init();

  printf("Hello, %s!\n", "Wokwi Simulation");

  op = ' ';

  while (1) {
    checkTickCount();
    tm1637_update();
    scanKeyboard();
  }

  return 0;
}