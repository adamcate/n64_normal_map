#include <libdragon.h>
#include <math.h>
#include <malloc.h>

static sprite_t *n_map;
static sprite_t *base;
// static sprite_t *base;

// surface_t base_surf;
surface_t normal_surf;
surface_t base_surf;


const int pix_stride = TEX_FORMAT_BYTES2PIX(FMT_RGBA32, 4*32);
	
#define __get_buffer( disp ) ((disp)->buffer)

#define __get_pixel( buffer, x, y ) \
    (buffer)[(x) + ((y) * pix_stride)]

#define __set_pixel( buffer, x, y, color ) \
    (buffer)[(x) + ((y) * pix_stride)] = color

float inv_sqrt_from_lookup(float input, int num_vals, float* lut, int max, int min) {
	int step = (max - min) / num_vals;
	if (input >= max) input = max;

	if(input < 0.f) input = 0.f;

	return(lut[(int)(input/step)]);
}

void draw_normal_point_light_tex(surface_t *disp, sprite_t *normal, int x_pos, int y_pos, int light_x,
							   int light_y, int light_z, color_t color, float intensity) {
	surface_t n_surf = sprite_get_pixels(normal);

	color_t curr_pixel;
	color_t output_pix;

	//int light_level = 255;

	float l_x = 0, l_y = 0, l_z = 0;
	float inv_magnitude;
	for(int x = x_pos; x < normal->width + x_pos; ++x){
			for(int y = y_pos; y < normal->height + y_pos; ++y){

				curr_pixel = color_from_packed32(__get_pixel((uint32_t *)__get_buffer(&n_surf),x - x_pos,y - y_pos));

				if(curr_pixel.a <= 5) continue;

				l_x = -1*x + light_x, l_y = y - light_y, l_z = -1*light_z;

				if((l_x * l_x + l_y * l_y + l_z * l_z) == 0) inv_magnitude = 1;
				else inv_magnitude = intensity / ((l_x * l_x + l_y * l_y + l_z * l_z));
				l_x *= inv_magnitude;
				l_y *= inv_magnitude;
				l_z *= inv_magnitude;

				float dot_product = l_x * ((float)curr_pixel.r / 255 - 0.5f) + l_y * ((float)curr_pixel.g / 255 - 0.5f) + l_z * ((float)curr_pixel.b / 255 - 0.5f);
				//dot_product = fabsf(dot_product);
				if(dot_product > 1.f) dot_product = 1.f;
				if(dot_product < 0.f) dot_product = 0.f;

				
				output_pix.r = color.r;
				output_pix.g = color.g;
				output_pix.b = color.b;
				output_pix.a = color.a * dot_product;

				graphics_draw_pixel_trans(disp, x, y, color_to_packed32(output_pix));
				
			}
		}
}

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
	float theta = 0;
	color_t bk_color = {.r = 0, .g = 0, .b = 0, .a = 255};
	color_t light_color = {.r = 255, .g = 255, .b = 0, .a = 255};
	color_t light_color_2 = {.r = 0, .g = 255, .b = 0, .a = 255};
	color_t light_color_3 = {.r = 255, .g = 0, .b = 0, .a = 127};

	rdpq_mode_antialias(false);
	rdpq_mode_dithering(DITHER_NONE_NONE);

	while (1) {
		surface_t *disp;
		if (!(disp = display_lock()))
			continue;

		rdpq_attach(disp, NULL);
		graphics_fill_screen(disp, color_to_packed32(bk_color));

		//graphics_draw_sprite_trans(disp, 100, 100, base);
		//rdpq_fill_rectangle(0,0,320,240);

		int l_pos_x = 160, l_pos_y = 240 - angle;
		int l_pos_x2 = 160 + 64*cosf(theta), l_pos_y2 = 120 + 64*sinf(theta);
		int l_pos_x3 = 160 - 64*cosf(0.25*theta), l_pos_y3 = 120;
		graphics_draw_sprite_trans(disp, 160-16, 120-32, base);
		draw_normal_point_light_tex(disp, n_map, 160-16, 120-32, l_pos_x-7, l_pos_y, -2, light_color, 50);
		draw_normal_point_light_tex(disp, n_map, 160-16, 120-32, l_pos_x2, l_pos_y2, -2, light_color_2, 100);
		draw_normal_point_light_tex(disp, n_map, 160-16, 120-32, l_pos_x3, l_pos_y3, -2, light_color_3, 50);

		angle += 10;
		theta += 0.2f;

		if(angle >= 240) angle = 0.f;
		if(theta >= 4*6.28f) theta = 0.f;


		rdpq_detach_show();
	}
	return 0;
}