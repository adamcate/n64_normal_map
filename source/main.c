#include <libdragon.h>
#include <math.h>

static sprite_t *n_map;
static sprite_t *base;
// static sprite_t *base;

// surface_t base_surf;
surface_t normal_surf;
surface_t base_surf;


/*void draw_normal_mapped_sprite(sprite_t *normal, sprite_t *base, int x_pos, int y_pos, int light_x0,
							   int light_y0, int light_x1, int light_y1, color_t color) {
	int dir_x = light_x1 - light_x0, dir_y = light_y1 - light_y0;

	for (int x = 0; x < base->width; ++x) {
		for (int y = 0; y < base->height; ++y) {
		}
	}
}*/

const int pix_stride = TEX_FORMAT_BYTES2PIX(FMT_RGBA32, 128);

#define __get_buffer( disp ) ((disp)->buffer)

#define __get_pixel( buffer, x, y ) \
    (buffer)[(x) + ((y) * pix_stride)]

#define __set_pixel( buffer, x, y, color ) \
    (buffer)[(x) + ((y) * pix_stride)] = color

int main() {
	display_init(RESOLUTION_320x240, DEPTH_32_BPP, 3, GAMMA_NONE, ANTIALIAS_RESAMPLE);

	dfs_init(DFS_DEFAULT_LOCATION);

	rdpq_init();

	n_map = sprite_load("rom:/normal.sprite");
	base = sprite_load("rom:/base.sprite");
	// base = sprite_load("rom:/base.sprite");

	normal_surf = sprite_get_pixels(n_map);
	base_surf = sprite_get_pixels(base);
	// base_surf = sprite_get_pixels(base);
	// surface_t output_surf = surface_alloc(FMT_RGBA32, 32, 32);
	float angle = 0;

	color_t bk_color = {.r = 0, .g = 0, .b = 0, .a = 0};
	rdpq_mode_antialias(false);
	rdpq_mode_dithering(DITHER_NONE_NONE);

	while (1) {
		surface_t *disp;
		if (!(disp = display_lock()))
			continue;

		rdpq_attach(disp, NULL);
		graphics_fill_screen(disp, color_to_packed32(bk_color));

		//rdpq_fill_rectangle(0,0,320,240);


		color_t curr_pixel;
		color_t output_pix;
		int light_level = 127+127*cosf(angle);
		float l_x = -2*cosf(2*angle), l_y = -2*sinf(2*angle), l_z = 0.5;

		graphics_draw_sprite_trans(disp, 100, 100, base);
		graphics_draw_sprite_trans(disp, 132, 100, base);
		for(int i = 0; i < 32; ++i){
			for(int j = 0; j < 64; ++j){
				curr_pixel = color_from_packed32(__get_pixel((uint32_t *)__get_buffer(&normal_surf),i,j));
				if(curr_pixel.a <= 5) continue;
				float dot_product = l_x * ((float)curr_pixel.r / 255 - 0.5f) + l_y * ((float)curr_pixel.g / 255 - 0.5f) + l_z * ((float)curr_pixel.b / 255 - 0.5f);
				//dot_product = fabsf(dot_product);
				if(dot_product > 1.f) dot_product = 1.f;
				if(dot_product < 0.f) dot_product = 0.f;

				output_pix.r = 255;
				output_pix.g = 0;
				output_pix.b = 0;
				output_pix.a = dot_product * light_level;

				graphics_draw_pixel_trans(disp, 100 + i, 100+j, color_to_packed32(output_pix));
				output_pix.r = 0;
				output_pix.g = 255;
				output_pix.b = 0;
				graphics_draw_pixel_trans(disp, 132 + i, 100+j, color_to_packed32(output_pix));
				
			}
		}

		angle += 0.05;

		if(angle >= 62.8f) angle = 0.f;
		rdpq_detach_show();
	}
	return 0;
}