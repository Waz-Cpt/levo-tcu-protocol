idf_component_register(
  SRCS "bringup.c" "display.c" "touch_controls.c" "touch_policy.c"
       "remote_buttons.c" "walk_test.c" "standalone_control.c" "async_log.c" "control_fault.c"
       "../../protocol/tcu_sequence.c" "../../protocol/tcu_diagnostics.c" "../../protocol/levo_protocol.c"
  INCLUDE_DIRS "." "../../protocol"
  REQUIRES driver esp_timer esp_lcd esp_lcd_jd9853 esp_app_format)
