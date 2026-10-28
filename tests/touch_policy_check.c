#include "touch_controls.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    assert(ui_hit(86, 140) == UI_PLUS);
    assert(ui_hit(86, 250) == UI_MINUS);
    assert(ui_hit(86, 50) == UI_SHUTDOWN);
    assert(ui_hit(86, 199) == UI_NONE);
    assert(ui_hit(-1, 115) == UI_NONE);
    assert(ui_hit(172, 115) == UI_NONE);
    assert(ui_hit(40, 319) == UI_NONE);
    assert(ui_hit(8, 88) == UI_PLUS);
    assert(ui_hit(163, 193) == UI_PLUS);
    assert(ui_hit(8, 204) == UI_MINUS);
    assert(ui_hit(163, 311) == UI_MINUS);
    for (unsigned m = 0; m < 4; m++) {
        assert(ui_requested(UI_PLUS, m) == (m < 3 ? m + 1 : 3));
        assert(ui_requested(UI_MINUS, m) == (m ? m - 1 : 0));
        for (int a = 0; a < 4; a++)
            assert(ui_requested(a, m) == (unsigned)a);
    }
    puts("PASS: touch hit zones, gaps, bounds, explicit modes, and +/- limits.");
}
