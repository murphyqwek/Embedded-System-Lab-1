#include "keyboard.h"

void initKeyboard() {
    GPIOB->MODER = (GPIOB->MODER & ~(3U << (7 * 2))) | (1U << (7 * 2));
    GPIOB->MODER = (GPIOB->MODER & ~(3U << (6 * 2))) | (1U << (6 * 2));
    GPIOB->MODER = (GPIOB->MODER & ~(3U << (3 * 2))) | (1U << (3 * 2));
    GPIOA->MODER = (GPIOA->MODER & ~(3U << (10 * 2))) | (1U << (10 * 2));

    GPIOB->OTYPER |= (1U << 7) | (1U << 6) | (1U << 3);
    GPIOA->OTYPER |= (1U << 10);

    GPIOB->BSRR = (1U << 7) | (1U << 6) | (1U << 3);
    GPIOA->BSRR = (1U << 10);

    GPIOB->MODER &= ~(
        (3U << (10 * 2)) |
        (3U << (4 * 2)) |
        (3U << (5 * 2)) |
        (3U << (15 * 2))
    );

    GPIOB->PUPDR = (GPIOB->PUPDR & ~(3U << (10 * 2))) | (1U << (10 * 2));
    GPIOB->PUPDR = (GPIOB->PUPDR & ~(3U << (4 * 2))) | (1U << (4 * 2));
    GPIOB->PUPDR = (GPIOB->PUPDR & ~(3U << (5 * 2))) | (1U << (5 * 2));
    GPIOB->PUPDR = (GPIOB->PUPDR & ~(3U << (15 * 2))) | (1U << (15 * 2));

    lastKey = '\0';
    lastScanTime = 0;
}

char readKey() {
  const uint8_t rows[] = {7, 6, 10, 3};    // Пины строк
  const uint8_t cols[] = {10, 4, 5, 15};       // Пины столбцов
  const char keymap[4][4] = {
    {'1', '2', '3', '+'},
    {'4', '5', '6', '-'},
    {'7', '8', '9', '/'},
    {'D', '0', '#', '*'}
  };

  for (uint8_t i = 0; i < 4; i++) {
    // Активируем текущую строку
    if (rows[i] != 10) {
      GPIOB->BSRR = (1 << (rows[i] + 16));
    } else {
      GPIOA->BSRR = (1 << (rows[i] + 16));
    }

    // Небольшая задержка для стабилизации
    for (volatile int d = 0; d < 100; d++);

    // Проверяем столбцы
    for (uint8_t j = 0; j < 4; j++) {
      if ((GPIOB->IDR & (1 << cols[j])) == 0) {
        // Деактивируем строку перед возвратом
        if (rows[i] != 10) {
          GPIOB->BSRR = (1 << rows[i]);
        } else {
          GPIOA->BSRR = (1 << rows[i]);
        }
        return keymap[i][j];
      }
    }

    // Деактивируем строку
    if (rows[i] != 10) {
      GPIOB->BSRR = (1 << rows[i]);
    } else {
      GPIOA->BSRR = (1 << rows[i]);
    }
  }

  return '\0';
}

void scanKeyboard() {
  // Сканируем клавиатуру каждые 100мс
  if(tickCount - lastScanTime <= 100) {
    return;
  }

  lastScanTime = tickCount;
  char currentKey = readKey();


  if(currentKey == '#') {
    printf("Calculations\n");
    
    if(op == '+') {
      counter = firstNum + secondNum;
    }

    if(op == '-') {
      counter = firstNum - secondNum;
    }

    if(op == '*') {
      counter = firstNum * secondNum;
    }

    if(op == '/') {
      counter = firstNum / secondNum;
    }

    op = ' ';

    printf("Result value: %d\n", counter);
  }

  if(currentKey >= '0' && currentKey <= '9') {
    uint16_t newNum = currentKey - '0';

    if(op == ' ') {
      firstNum = newNum;
      printf("First number: %d\n", firstNum);
    }
    else {
      secondNum = newNum;
      printf("Second number: %d\n", secondNum);
    }

    counter = newNum;
  }
  
  if(currentKey == '+' ||
     currentKey == '-' ||
     currentKey == '/' ||
     currentKey == '*') {

      op = currentKey;

      printf("Selected op: %c\n", op);
     }
}
