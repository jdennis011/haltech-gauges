#include <M5Dial.h>
#include <lvgl.h>

#include "touch.h"

namespace touch {

namespace {

// M5Dial.update() (called from board::update()) refreshes the touch state.
void readCb(lv_indev_t*, lv_indev_data_t* data) {
    const auto t = M5Dial.Touch.getDetail();
    if (t.isPressed()) {
        data->point.x = t.x;
        data->point.y = t.y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

}  // namespace

bool begin() {
    lv_indev_t* indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, readCb);
    return true;
}

}  // namespace touch
