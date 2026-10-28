/* Offline encoder example. Prints bytes; does not transmit CAN. */
#include "levo_protocol.h"
#include <stdio.h>
int main(void) {
    const uint8_t turbo[7] = {0x03, 0x5a, 0x64, 0x5a, 0x64, 0, 0x64};
    uint8_t command[8], companion[3];
    if (!levo_command_encode(turbo, 0, command) || !levo_companion_encode(0, companion))
        return 1;
    printf("300 [8]");
    for (unsigned i = 0; i < sizeof command; ++i)
        printf(" %02X", command[i]);
    printf("\n301 [3]");
    for (unsigned i = 0; i < sizeof companion; ++i)
        printf(" %02X", companion[i]);
    puts("");
    return 0;
}
