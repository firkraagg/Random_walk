#include "Menu.h"

void display_menu() {
    printf("---------Hlavne menu---------\n");
    printf("1. Nova simulacia\n");
    printf("2. Pripojenie k simulacii\n");
    printf("3. Opatovne spustenie simulacie\n");
    printf("4. Koniec\n");
    printf("Zvolte moznost: ");
}

void handle_menu_option(int number) {
    switch (number)
    {
    case 1:
        run_simulation(create_simulation());
        break;

    case 2:
        printf("Pripajam sa k simulacii..");
        break;

    case 3:
        run_simulation(recreate_simulation());
        break;

    case 4:
        printf("Ukoncujem aplikaciu..");
        break;
    
    default:
        printf("Ukoncujem aplikaciu..");
        break;
    }
}