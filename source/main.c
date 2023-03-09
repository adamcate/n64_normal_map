#include <libdragon.h>
#include <math.h>
#include <malloc.h>

static sprite_t *n_map;
static sprite_t *base;
// static sprite_t *base;

// surface_t base_surf;
surface_t normal_surf;
surface_t base_surf;


const int pix_stride = TEX_FORMAT_BYTES2PIX(FMT_RGBA32, 4*128);
	
#define __get_buffer( disp ) ((disp)->buffer)

#define __get_pixel( buffer, x, y ) \
    (buffer)[(x) + ((y) * pix_stride)]

#define __set_pixel( buffer, x, y, color ) \
    (buffer)[(x) + ((y) * pix_stride)] = color

static int __is_transparent( int bitdepth, uint32_t color )
{
    if( bitdepth == 2 )
    {
        /* Is alpha bit set? */
        if( (color & 0x1) == 0x0 ) { return 1; }
    }
    else
    {
        /* Is alpha byte set? */
        if( (color & 0xFF) == 0x00 ) { return 1; }
    }

    return 0;
}

void graphics_draw_pixel_trans_opt( surface_t* disp, int x, int y, uint32_t color )
{
    if( disp == 0 ) { return; }
    int pix_stride = TEX_FORMAT_BYTES2PIX(surface_get_format(disp), disp->stride);

    if( TEX_FORMAT_BITDEPTH(surface_get_format( disp )) == 16 )
    {
        /* Only display the pixel if alpha bit is set */
        if( !__is_transparent( 2, color ) )
        {
            __set_pixel( (uint16_t *)__get_buffer( disp ), x, y, color );
        }
    }
    else
    {
        /* Get 32bit representations */
        uint32_t cur_color = __get_pixel( (uint32_t *)__get_buffer( disp ), x, y );

        /* Get current color */
        uint32_t cr = (cur_color >> 24) & 0xFF;
        uint32_t cg = (cur_color >> 16) & 0xFF;
        uint32_t cb = (cur_color >> 8) & 0xFF;

        /* Get new color */
        uint32_t sr = (color >> 24) & 0xFF;
        uint32_t sg = (color >> 16) & 0xFF;
        uint32_t sb = (color >> 8) & 0xFF;

        /* Transparencies */
        uint32_t st = color & 0xFF;
        uint32_t ct = 255 - st;

        /* Mixed color */
        uint32_t mixed_color;
        uint8_t *final_color = (uint8_t *)(&mixed_color);

        final_color[0] = ((cr * ct) + (sr * st)) >> 8;
        final_color[1] = ((cg * ct) + (sg * st)) >> 8;
        final_color[2] = ((cb * ct) + (sb * st)) >> 8;

        /* Since we are doing mixing anyway */
        final_color[3] = 255;

        __set_pixel( (uint32_t *)__get_buffer( disp ), x, y, mixed_color );
    }

}

void draw_normal_point_light_tex_float(surface_t *disp, sprite_t *normal, int x_pos, int y_pos, int light_x, int light_y, int light_z, color_t color, float intensity) {
    surface_t n_surf = sprite_get_pixels(normal);

    color_t curr_pixel;
    uint32_t output_pix = color_to_packed32(color);
	uint32_t *buf = (uint32_t *)__get_buffer(&n_surf);

    //int light_level = 255;

    float l_x = 0, l_y = 0, l_z = 0;

	const int width = normal->width;
	const int height = normal->height;

    for(int y = 0; y < height; ++y){
            for(int x = 0; x < width; ++x){

                curr_pixel = color_from_packed32(__get_pixel(buf,x, y));

                if(!curr_pixel.a) continue;

                l_x = light_x - x, l_y = y - light_y, l_z = 0 - light_z;


                float dot_product = l_x * (curr_pixel.r - 127) + l_y * (curr_pixel.g - 127) + l_z * (curr_pixel.b - 127);


				float mag = l_x * l_x + l_y * l_y + l_z * l_z;
				
				dot_product *= intensity;
                if(mag != 0.f) dot_product /= mag;
				

				if(dot_product > 255.f) dot_product = 255.f;
                if(dot_product < 0.f) dot_product = 0.f;

                graphics_draw_pixel_trans_opt(disp, x+x_pos, y+y_pos, (output_pix ^ 0xFF) | (uint32_t)(dot_product));
                
            }
        }
}

void draw_normal_point_light_tex_fast(surface_t *disp, sprite_t *normal, int x_pos, int y_pos, int light_x,
							   int light_y, int light_z, color_t color, int intensity) {
	surface_t n_surf = sprite_get_pixels(normal);

	color_t curr_pixel;

	uint32_t col_pixel = color_to_packed32(color);

	//int light_level = 255;


	// float inv_magnitude;
	for(int y = y_pos; y < normal->height + y_pos; ++y){
			for(int x = x_pos; x < normal->width + x_pos; ++x){

				curr_pixel = color_from_packed32(__get_pixel((uint32_t *)__get_buffer(&n_surf),x - x_pos,y - y_pos));

				if(!curr_pixel.a) continue;

				int l_x = light_x - x, l_y = y - light_y, l_z = 0 - light_z;
				
				int mag = ((l_x * l_x + l_y * l_y + l_z * l_z) >> intensity);
				if (!mag) mag = 1;
				// if(!mag) inv_magnitude = 1;
				// else inv_magnitude = intensity / mag;

				int dot_product = (l_x * (curr_pixel.r - 127) + l_y * (curr_pixel.g - 127) + l_z * (curr_pixel.b - 127));
				//dot_product = fabsf(dot_product);
				if(dot_product < 0) dot_product = 0;

				
				int alpha = (dot_product) / mag;
				if(alpha > 255) alpha = 255;

				// if(x) continue;
				graphics_draw_pixel_trans(disp, x, y, (col_pixel ^ 0xFF) | alpha);
				
			}
		}
}


int main() {
	display_init(RESOLUTION_320x240, DEPTH_32_BPP, 3, GAMMA_NONE, ANTIALIAS_RESAMPLE);

	dfs_init(DFS_DEFAULT_LOCATION);

	rdpq_init();
	timer_init();

	debug_init(DEBUG_FEATURE_ALL);

	controller_init();

	n_map = sprite_load("rom:/normal3.sprite");
	base = sprite_load("rom:/base.sprite");
	// base = sprite_load("rom:/base.sprite");

	normal_surf = sprite_get_pixels(n_map);
	base_surf = sprite_get_pixels(base);
	// base_surf = sprite_get_pixels(base);
	// surface_t output_surf = surface_alloc(FMT_RGBA32, 32, 32);

	color_t bk_color = {.r = 0, .g = 0, .b = 0, .a = 255};
	color_t light_color = {.r = 255, .g = 0, .b = 0, .a = 255};

	rdpq_mode_antialias(false);
	rdpq_mode_dithering(DITHER_NONE_NONE);

	
	int l_pos_x = 128, l_pos_y = 64, l_pos_z = -20;
	float intensity = 64;
	
	while (1) {
		surface_t *disp;
		if (!(disp = display_lock()))
			continue;
		
		struct controller_data control = get_keys_held();

		controller_scan();

		if(control.c[0].left) l_pos_x -= 5;
		if(control.c[0].right) l_pos_x += 5;
		if(control.c[0].up) l_pos_y -= 5;
		if(control.c[0].down) l_pos_y += 5;
		if(control.c[0].C_up) l_pos_z -= 5;
		if(control.c[0].C_down) l_pos_z += 5;
		if(control.c[0].C_left) intensity-=8;
		if(control.c[0].C_right) intensity+=8;

		rdpq_attach(disp, NULL);
		graphics_fill_screen(disp, color_to_packed32(bk_color));

		//graphics_draw_sprite_trans(disp, 100, 100, base);
		//rdpq_fill_rectangle(0,0,320,240);
	
		//graphics_draw_sprite_trans(disp, 160, 120, base);
		
		uint64_t initial = TIMER_MICROS_LL(timer_ticks());

		//draw_normal_point_light_tex_fast(disp, n_map, 160, 120, l_pos_x, l_pos_y, l_pos_z, light_color, 6);
		//draw_normal_point_light_tex_float(disp, n_map, 0, 0, l_pos_x, l_pos_y, l_pos_z, light_color, intensity);
		draw_normal_point_light_tex_float(disp, n_map, 128, 0, l_pos_x-128, l_pos_y, l_pos_z, light_color, intensity);

		debugf("%llu us\n", TIMER_MICROS(timer_ticks()) - initial);

		
		rdpq_detach_show();
	}
	return 0;
}