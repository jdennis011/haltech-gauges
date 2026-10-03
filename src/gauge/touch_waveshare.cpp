#include "touch.h"

#include <TouchDrvCSTXXX.hpp>
#include <Wire.h>
#include <lvgl.h>

#include "board_pins.h"
#include "display.h"

namespace touch {

namespace {

TouchDrvCST92xx driver;
bool started = false;

void readCb(lv_indev_t*, lv_indev_data_t* data) {
    int16_t x[1], y[1];
    if (started && driver.getPoint(x, y, 1) > 0) {
        data->point.x = x[0];
        data->point.y = y[0];
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

}  // namespace

bool begin() {
    driver.setPins(PIN_TOUCH_RST, PIN_TOUCH_INT);
    started = driver.begin(Wire, 0x5A, PIN_I2C_SDA, PIN_I2C_SCL);
    if (started) {
        driver.setMaxCoordinates(display::kSize, display::kSize);
        driver.setMirrorXY(true, true);
    }

    lv_indev_t* indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, readCb);
    return started;
}

}  // namespace touch
