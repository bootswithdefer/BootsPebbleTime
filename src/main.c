#include <pebble.h>
#include <locale.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static TextLayer *s_unixtime_layer;
static TextLayer *s_battery_layer;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // *** time ***
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  
  text_layer_set_text(s_time_layer, s_buffer);

  // *** date ***
  static char s_datebuf[12];
  strftime(s_datebuf, sizeof(s_datebuf), "%F", tick_time); 
  
  text_layer_set_text(s_date_layer, s_datebuf);
  
  // *** unix time ***
  char s_unixbuf[14];
  snprintf(s_unixbuf, sizeof(s_unixbuf), "%lu", temp);

  int c, i;
  static char s_unixbuf_f[14];
  char *p;

  c = 2 - strlen(s_unixbuf) % 3;
  i = 0;
  for (p = s_unixbuf; *p != 0; p++) {
     s_unixbuf_f[i++] = *p;
     if (c == 1 && *(p+1) != 0) {
         s_unixbuf_f[i++] = ',';
     }
     c = (c + 1) % 3;
  }
  s_unixbuf_f[i] = '\0';
  
  text_layer_set_text(s_unixtime_layer, s_unixbuf_f);
}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100% charged";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% charged", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // *** time ***
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(8, 0), bounds.size.w, 50));

  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // *** date ***
  s_date_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 50), bounds.size.w, 28));

  text_layer_set_background_color(s_date_layer, GColorClear);
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_text(s_date_layer, "00/00/0000");
  text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

  // *** unix time ***
  s_unixtime_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(86, 78), bounds.size.w, 28));

  text_layer_set_background_color(s_unixtime_layer, GColorClear);
  text_layer_set_text_color(s_unixtime_layer, GColorBlack);
  text_layer_set_text(s_unixtime_layer, "0,000,000,000");
  text_layer_set_font(s_unixtime_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_unixtime_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_unixtime_layer));

  // *** battery ***
  s_battery_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(114, 106), bounds.size.w, 32));

  text_layer_set_background_color(s_battery_layer, GColorClear);
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  text_layer_set_text(s_battery_layer, "100% charging");
  text_layer_set_font(s_battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentCenter);

  layer_add_child(window_layer, text_layer_get_layer(s_battery_layer));
  handle_battery(battery_state_service_peek());
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_date_layer);
  text_layer_destroy(s_unixtime_layer);
  text_layer_destroy(s_battery_layer);
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}