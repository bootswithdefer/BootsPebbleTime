#include <pebble.h>
#include <locale.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_unixtime_layer;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);

  static char buf[14];
  snprintf(buf, sizeof(buf), "%lu", temp);

  int c, i;
  static char out[14];
  char *p;

  c = 2 - strlen(buf) % 3;
  i = 0;
  for (p = buf; *p != 0; p++) {
     out[i++] = *p;
     if (c == 1 && *(p+1) != 0) {
         out[i++] = ',';
     }
     c = (c + 1) % 3;
  }
  out[i] = '\0';
  
  text_layer_set_text(s_unixtime_layer, out);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(8, 0), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  s_unixtime_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(58, 50), bounds.size.w, 28));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_unixtime_layer, GColorClear);
  text_layer_set_text_color(s_unixtime_layer, GColorBlack);
  text_layer_set_text(s_unixtime_layer, "0,000,000,000");
  text_layer_set_font(s_unixtime_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
  text_layer_set_text_alignment(s_unixtime_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_unixtime_layer));

}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_unixtime_layer);
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