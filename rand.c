#include <stdlib.h>
#include <stdio.h>
#include <time.h>


void main() {
srand(time(NULL));
int led = rand() % 2;
printf("Rand: %d\n", led);
}
