#include "Menu.h"
#include <time.h>

int main(int argc, char** argv) {
    srand(time(NULL));

    display_menu();
    int number;
    scanf("%d", &number);
    handle_menu_option(number);

    return 0;
}