#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"

#define MY_UUID { 0x9B, 0xD4, 0xF2, 0xB6, 0x63, 0xDB, 0x4E, 0x09, 0xB3, 0x27, 0x81, 0x42, 0xAE, 0xBD, 0xAA, 0xE1 }
PBL_APP_INFO(MY_UUID,
             "Beer O' Clock Deluxe", "ThomW / LMNOpc.com",
             1, 0, /* App version */
             RESOURCE_ID_IMAGE_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;

BmpContainer imageMug;
BmpContainer imageBeer;
RotBmpPairContainer imageBOC;

Layer beerContainer;

TextLayer timeLayer;

int beer_offset = -1;

const int BEER_STARTING_Y = 125;

const int ANIMATION_START_TIME = 9;
const int ANIMATION_END_TIME = 17;

void update_display(PblTm *current_time) {

  // Write the time (yawn)
  static char timeText[] = "00:00 00";
  char *timeFormat;
  if (clock_is_24h_style()) {
    timeFormat = " %R";
  } else {
    timeFormat = "%l:%M %p";
  }
  string_format_time(timeText, sizeof(timeText), timeFormat, current_time);
  text_layer_set_text(&timeLayer, timeText);

  // At 5pm, show the BEER O CLOCK graphic, and hide the actual time
  bool showText = (current_time->tm_hour == 17) && (current_time->tm_min >= 0) && (current_time->tm_min <= 5);
  layer_set_hidden(&imageBOC.layer.layer, !showText);
  layer_set_hidden(&timeLayer.layer, showText);

  // Calculate 
  int offset = beer_offset;

  // Reset the beer before 9am
  if (current_time->tm_hour < ANIMATION_START_TIME) {
    
    offset = BEER_STARTING_Y;

  // Animate the beer between 9-5
  } else if ((current_time->tm_hour >= ANIMATION_START_TIME) && (current_time->tm_hour <= ANIMATION_END_TIME)) {

    const int totalSeconds = (ANIMATION_END_TIME - ANIMATION_START_TIME) * 60 * 60;

    int currentSeconds = ((current_time->tm_hour - ANIMATION_START_TIME) * 3600)
                        + (current_time->tm_min * 60)
                        + current_time->tm_sec;

    offset = (int)((float)BEER_STARTING_Y - (((float)currentSeconds / (float)totalSeconds) * (float)BEER_STARTING_Y));
  }

  // Update the position of imageBeer if necessary
  if (offset < 0) {
    offset = 0;
  }
  if (offset != beer_offset) {
    imageBeer.layer.layer.frame.origin.y = offset;
    beer_offset = offset;
  }

}

void handle_init(AppContextRef ctx) {
  (void)ctx;

  window_init(&window, "Beer O Clock");
  window_stack_push(&window, true);

  window_set_background_color(&window, GColorBlack);

  resource_init_current_app(&APP_RESOURCES);

  // Mug
  bmp_init_container(RESOURCE_ID_IMAGE_MUG, &imageMug);
  imageMug.layer.layer.frame.origin.x = 25;
  imageMug.layer.layer.frame.origin.y = 11;  
  layer_add_child(&window.layer, &imageMug.layer.layer);

  // Beer and its container
  layer_init(&beerContainer, GRect(25, 0, 66, 125));
  bmp_init_container(RESOURCE_ID_IMAGE_BEER, &imageBeer);
  bitmap_layer_set_compositing_mode(&imageBeer.layer, GCompOpOr);
  imageBeer.layer.layer.frame.origin.x = 0;
  imageBeer.layer.layer.frame.origin.y = BEER_STARTING_Y;  
  layer_add_child(&window.layer, &beerContainer);
  layer_add_child(&beerContainer, &imageBeer.layer.layer);

  // BOC
  rotbmp_pair_init_container(RESOURCE_ID_IMAGE_TEXT_WHITE, RESOURCE_ID_IMAGE_TEXT_BLACK, &imageBOC);
  imageBOC.layer.layer.frame.origin.x = 3;
  imageBOC.layer.layer.frame.origin.y = 65;
  layer_add_child(&window.layer, &imageBOC.layer.layer);

  // Time display
  text_layer_init(&timeLayer, GRect(40, 140, 80, 40));
  text_layer_set_text_color(&timeLayer, GColorWhite);
  text_layer_set_background_color(&timeLayer, GColorClear);
  text_layer_set_font(&timeLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(&window.layer, &timeLayer.layer);

  // Avoid a blank screen on watch start
  PblTm tick_time;
  get_time(&tick_time);
  update_display(&tick_time);
}

void handle_deinit(AppContextRef ctx) {
  (void)ctx;

  bmp_deinit_container(&imageMug);
  bmp_deinit_container(&imageBeer);
  rotbmp_pair_deinit_container(&imageBOC);
}

void handle_minute_tick(AppContextRef ctx, PebbleTickEvent *t) {
  (void)ctx;
  update_display(t->tick_time);
}

void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
    .deinit_handler = &handle_deinit,
    .tick_info = {
      .tick_handler = &handle_minute_tick,
      .tick_units = MINUTE_UNIT     /* DEBUG: Change this to minutes when I'm done ... duh */
    }
  };
  app_event_loop(params, &handlers);
}

