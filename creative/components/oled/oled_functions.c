#include "oled_init.h"
#include "font6x8.h"

void clear_pixels(t_oled *oled)
{
    if (!oled)
        ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
    memset(oled->pixels, 0, sizeof(oled->pixels));
}

void display_pixels(t_oled *oled)
{
    if (!oled)
        ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    ESP_ERROR_CHECK(i2c_master_start(cmd));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd,
                                          (oled->addr << 1) | I2C_MASTER_WRITE, true));
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x80, true)); // single command
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x00, true)); // page number
    ESP_ERROR_CHECK(i2c_master_write_byte(cmd, 0x40, true)); // data stream
    ESP_ERROR_CHECK(i2c_master_write(cmd, oled->pixels,
                                     sizeof(oled->pixels), true));
    ESP_ERROR_CHECK(i2c_master_stop(cmd));
    print_error(i2c_master_cmd_begin(
            oled->port, cmd, 10 / portTICK_PERIOD_MS),
                __func__, __LINE__, "master_cmd_begin failed");
    i2c_cmd_link_delete(cmd);
}

void put_pixel(uint8_t *pixels, int16_t x, uint16_t y, _Bool color)
{
    if (!pixels)
        ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
    if (color == true)
        pixels[x + ((y >> 3U) * LCDWIDTH)] |=  (1U << (y & 7U));
    else
        pixels[x + ((y >> 3U) * LCDWIDTH)] &= ~(1U << (y & 7U));
}

static void char_to_pixels(uint8_t *pixels, int *pixels_index, int char_index)
{
    pixels[(*pixels_index)++] = font6x8[char_index++];
    pixels[(*pixels_index)++] = font6x8[char_index++];
    pixels[(*pixels_index)++] = font6x8[char_index++];
    pixels[(*pixels_index)++] = font6x8[char_index++];
    pixels[(*pixels_index)++] = font6x8[char_index++];
    pixels[(*pixels_index)] = font6x8[char_index];
}

void str_to_oled(t_oled *oled, char *str)
{
    if (!oled || !str)
        ESP_ERROR_CHECK(ESP_ERR_INVALID_ARG);
    int len = strlen(str);
    int calc = LCDWIDTH - (len * 6);
    int x = (calc > 0) ? (calc / 2) : 0;
    int y = 3;
    for (int i = 0; (i < len) && (i < 168); ++i) {
        int pixels_index = (y * LCDWIDTH) + x;
        int char_index = (str[i] - 32) * 6;
        char_to_pixels(oled->pixels, &pixels_index, char_index);
        x += 6;
        if (x >= 126)
        {
            x = 0;
            y++;
        }
    }
    display_pixels(oled);
}