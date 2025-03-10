#include <stdint.h>

#define VIDEO_MEMORY((uint16_t * ) 0xb8000)
#define KEYBOARD_PORT 0x60

static uint8_t cursor_x = 0, cursor_y = 0;

static inline uint8_t inb(uint16_t port) {
  uint8_t result;
  __asm__ volatile("inb %1, %0": "=a"(result): "Nd"(port));
  return result;
}

void printf(const char * str) {
  for (int i = 0; str[i] != '\0'; ++i) {
    if (str[i] == '\n') {
      cursor_x = 0;
      cursor_y++;
    } else {
      VIDEO_MEMORY[80 * cursor_y + cursor_x] = (VIDEO_MEMORY[80 * cursor_y + cursor_x] & 0xFF00) | str[i];
      cursor_x++;
    }

    if (cursor_x >= 80) {
      cursor_x = 0;
      cursor_y++;
    }

    if (cursor_y >= 25) {
      for (cursor_y = 0; cursor_y < 25; ++cursor_y)
        for (cursor_x = 0; cursor_x < 80; ++cursor_x)
          VIDEO_MEMORY[80 * cursor_y + cursor_x] = (VIDEO_MEMORY[80 * cursor_y + cursor_x] & 0xFF00) | ' ';
      cursor_x = 0;
      cursor_y = 0;
    }
  }
}

void clear_screen() {
  for (cursor_y = 0; cursor_y < 25; ++cursor_y)
    for (cursor_x = 0; cursor_x < 80; ++cursor_x)
      VIDEO_MEMORY[80 * cursor_y + cursor_x] = (VIDEO_MEMORY[80 * cursor_y + cursor_x] & 0xFF00) | ' ';
  cursor_x = 0;
  cursor_y = 0;
}

void putchar(char c) {
  char str[2] = {
    c,
    '\0'
  };
  printf(str);
}

void print_number(int64_t num) {
  char buffer[21];
  int i = 0, isNegative = 0;

  if (num == 0) {
    putchar('0');
    return;
  }

  if (num < 0) {
    isNegative = 1;
    num = -num;
  }

  while (num > 0) {
    buffer[i++] = (num % 10) + '0';
    num /= 10;
  }

  if (isNegative) buffer[i++] = '-';

  while (i--) putchar(buffer[i]);
}

static char keymap[128] = {
  0,
  27,
  '1',
  '2',
  '3',
  '4',
  '5',
  '6',
  '7',
  '8',
  '9',
  '0',
  '-',
  '=',
  '\b',
  '\t',
  'q',
  'w',
  'e',
  'r',
  't',
  'y',
  'u',
  'i',
  'o',
  'p',
  '[',
  ']',
  '\n',
  0,
  'a',
  's',
  'd',
  'f',
  'g',
  'h',
  'j',
  'k',
  'l',
  ';',
  '\'',
  '`',
  0,
  '\\',
  'z',
  'x',
  'c',
  'v',
  'b',
  'n',
  'm',
  ',',
  '.',
  '/',
  0,
  '*',
  0,
  ' ',
  0
};

char keyboard_getchar() {
  char key = 0;
  while (key == 0) {
    uint8_t scancode = inb(KEYBOARD_PORT);
    if (scancode < 128) {
      key = keymap[scancode];

      while (inb(KEYBOARD_PORT) == scancode);
    }
  }
  return key;
}

int64_t get_number() {
  char buffer[21];
  int index = 0;
  int isNegative = 0;

  while (1) {
    char c = keyboard_getchar();
    if (c == '\n') {
      buffer[index] = '\0';
      break;
    }
    if (c >= '0' && c <= '9') {
      putchar(c);
      buffer[index++] = c;
    } else if (c == '-' && index == 0) {
      putchar(c);
      isNegative = 1;
    }
  }

  int64_t num = 0;
  for (int i = isNegative; buffer[i] != '\0'; i++) {
    num = num * 10 + (buffer[i] - '0');
  }
  return isNegative ? -num : num;
}

void pause() {
  printf("\nPress Any Key To Continue...");
  keyboard_getchar();
  clear_screen();
}

void print_menu() {
  printf("HDH-N12 Terminal Calculator\n");
  printf("---------------------------\n");
  printf("1. Addition\n");
  printf("2. Subtraction\n");
  printf("3. Multiplication\n");
  printf("4. Division\n\n");
  printf("Choose An Option: ");
}

void kernel_main(void) {
  while (1) {
    print_menu();
    char choice = keyboard_getchar();
    putchar(choice);
    printf("\n");

    if (choice != '1' && choice != '2' && choice != '3' && choice != '4') {
      printf("Invalid Option! Try Again!\n");
      pause();
      continue;
    }

    printf("Enter First Number: ");
    int64_t num1 = get_number();
    printf("\nEnter Second Number: ");
    int64_t num2 = get_number();
    printf("\n");

    int64_t result = 0;
    switch (choice) {
    case '1':
      result = num1 + num2;
      printf("Result: ");
      print_number(result);
      printf("\n");
      break;
    case '2':
      result = num1 - num2;
      printf("Result: ");
      print_number(result);
      printf("\n");
      break;
    case '3':
      result = num1 * num2;
      printf("Result: ");
      print_number(result);
      printf("\n");
      break;
    case '4':
      if (num2 == 0) {
        printf("Error: Division By Zero!\n");
      } else {
        result = num1 / num2;
        printf("Result: ");
        print_number(result);
        printf("\n");
      }
      break;
    }

    pause();
  }
}