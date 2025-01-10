#include "client.h"
#include "server.h"
#include <time.h>

int main(int argc, char** argv) {
    srand(time(NULL));
    start_client();
    return 0;
}