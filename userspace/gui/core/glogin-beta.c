/*
 * glogin
 *
 * Graphical Login screen
 */
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

#include <sys/utsname.h>

#include "lib/sha2.h"
#include "lib/graphics.h"
#include "lib/window.h"
#include "lib/shmemfonts.h"
#include "lib/kbd.h"
#include "lib/yutani.h"

sprite_t * sprites[128];
sprite_t alpha_tmp;

gfx_context_t * ctx;

uint16_t win_width;
uint16_t win_height;

int uid = 0;

/* Timing type */
struct timeval {
	unsigned int tv_sec;
	unsigned int tv_usec;
};

static window_t * _window_create(int x, int y, int w, int h) {
	window_t * win = malloc(sizeof(window_t));
	win->wid = 0;
	win->width = w;
	win->height = h;
	win->buffer = malloc(w * h * 4);
}

#define LOGO_FINAL_OFFSET 100

int checkUserPass(char * user, char * pass) {

	/* Generate SHA512 */
	char hash[SHA512_DIGEST_STRING_LENGTH];
	SHA512_Data(pass, strlen(pass), hash);

	/* Open up /etc/master.passwd */

	FILE * passwd = fopen("/etc/master.passwd", "r");
	char line[2048];

	while (fgets(line, 2048, passwd) != NULL) {

		line[strlen(line)-1] = '\0';

		char *p, *tokens[4], *last;
		int i = 0;
		for ((p = strtok_r(line, ":", &last)); p;
				(p = strtok_r(NULL, ":", &last)), i++) {
			if (i < 511) tokens[i] = p;
		}
		tokens[i] = NULL;
		
		if (strcmp(tokens[0],user) != 0) {
			continue;
		}
		if (!strcmp(tokens[1],hash)) {
			fclose(passwd);
			return atoi(tokens[2]);
		}
		}
	fclose(passwd);
	return -1;

}

int center_x(int x) {
	return (win_width - x) / 2;
}

int center_y(int y) {
	return (win_height - y) / 2;
}

void init_sprite_png(int id, char * path) {
	sprites[id] = malloc(sizeof(sprite_t));
	load_sprite_png(sprites[id], path);
}

#define INPUT_SIZE 1024
int buffer_put(char * input_buffer, char c) {
	int input_collected = strlen(input_buffer);
	if (c == 8) {
		/* Backspace */
		if (input_collected > 0) {
			input_collected--;
			input_buffer[input_collected] = '\0';
		}
		return 0;
	}
	if (c < 10 || (c > 10 && c < 32) || c > 126) {
		return 0;
	}
	input_buffer[input_collected] = c;
	input_collected++;
	input_buffer[input_collected] = '\0';
	if (input_collected == INPUT_SIZE - 1) {
		return 1;
	}
	return 0;
}

void * process_input(void * arg) {
	while (1) {
	}
}

int32_t min(int32_t a, int32_t b) {
	return (a < b) ? a : b;
}

int32_t max(int32_t a, int32_t b) {
	return (a > b) ? a : b;
}

void draw_box(gfx_context_t * ctx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
	int32_t _min_x = max(x, 0);
	int32_t _min_y = max(y,  0);
	int32_t _max_x = min(x + w - 1, ctx->width  - 1);
	int32_t _max_y = min(y + h - 1, ctx->height - 1);

	for (int i = _min_y; i < _max_y; ++i) {
		draw_line(ctx, _min_x, _max_x, i, i, color);
	}
}

void draw_box_border(gfx_context_t * ctx, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) {
	int32_t _min_x = max(x, 0);
	int32_t _min_y = max(y,  0);
	int32_t _max_x = min(x + w - 1, ctx->width  - 1);
	int32_t _max_y = min(y + h - 1, ctx->height - 1);

	draw_line(ctx, _min_x, _max_x, _min_y, _min_y, color);
	draw_line(ctx, _min_x, _max_x, _max_y, _max_y, color);
	draw_line(ctx, _min_x, _min_x, _min_y, _max_y, color);
	draw_line(ctx, _max_x, _max_x, _min_y, _max_y, color);
}

int main (int argc, char ** argv) {
	init_sprite_png(0, "/usr/share/logo_login.png");
	init_sprite_png(1, "/usr/share/wallpaper.png");
	init_shmemfonts();

	fprintf(stderr, "glogin-beta here, hello world.\n");

	yutani_t * y = yutani_init();

	if (!y) {
		fprintf(stderr, "[demo-client] Connection to server failed.\n");
		return 1;
	}

	while (1) {

		int width  = y->display_width;
		int height = y->display_height;

		win_width = width;
		win_height = height;

		if (!sprites[2]) {
			float x = (float)width  / (float)sprites[1]->width;
			float y = (float)height / (float)sprites[1]->height;

			int nh = (int)(x * (float)sprites[1]->height);
			int nw = (int)(y * (float)sprites[1]->width);;

			sprite_t * tmp = create_sprite(width, height, ALPHA_OPAQUE);
			gfx_context_t * bg_tmp = init_graphics_sprite(tmp);

			sprites[2] = create_sprite(width, height, ALPHA_OPAQUE);
			gfx_context_t * bg = init_graphics_sprite(sprites[2]);

			if (nw > width) {
				draw_sprite_scaled(bg_tmp, sprites[1], (width - nw) / 2, 0, nw, height);
			} else {
				draw_sprite_scaled(bg_tmp, sprites[1], 0, (height - nh) / 2, width, nh);
			}

			blur_context_no_vignette(bg, bg_tmp, 80.0);
			blur_context_no_vignette(bg_tmp, bg, 400.0);
			blur_context_no_vignette(bg, bg_tmp, 200.0);

			sprite_free(tmp);

			free(bg_tmp);
			free(bg);
		}


		/* Do something with a window */
		yutani_window_t * wina = yutani_window_create(y, width, height);
		assert(wina);
		//window_reorder (wina, 0); /* Disables movement */
		ctx = init_graphics_yutani_double_buffer(wina);

		for (int i = 0; i < LOGO_FINAL_OFFSET; ++i) {
			draw_fill(ctx, rgb(0,0,0));
			draw_sprite(ctx, sprites[2], center_x(width), center_y(height));
			draw_sprite(ctx, sprites[0], center_x(sprites[0]->width), center_y(sprites[0]->height) - i);
			flip(ctx);
			yutani_flip(y, wina);
		}

		size_t buf_size = wina->width * wina->height * sizeof(uint32_t);
		char * buf = malloc(buf_size);

		uint32_t i = 0;

		uint32_t black = rgb(0,0,0);
		uint32_t white = rgb(255,255,255);
		uint32_t red   = rgb(240, 20, 20);

		int x_offset = 65;
		int y_offset = 64;

		int fuzz = 3;

		char username[INPUT_SIZE] = {0};
		char password[INPUT_SIZE] = {0};
		char hostname[512];

		{
			char _hostname[256];
			syscall_gethostname(_hostname);

			struct tm * timeinfo;
			struct timeval now;
			syscall_gettimeofday(&now, NULL); //time(NULL);
			timeinfo = localtime((time_t *)&now.tv_sec);

			char _date[256];
			strftime(_date, 256, "%a %B %d %Y", timeinfo);

			sprintf(hostname, "%s // %s", _hostname, _date);
		}

		char kernel_v[512];

		{
			struct utsname u;
			uname(&u);
			/* UTF-8 Strings FTW! */
			uint8_t * os_name_ = "とあるOS";
			uint32_t l = snprintf(kernel_v, 512, "%s %s", os_name_, u.release);
		}

		uid = 0;

#define BOX_WIDTH  272
#define BOX_HEIGHT 104
#define USERNAME_BOX 1
#define PASSWORD_BOX 2
#define EXTRA_TEXT_OFFSET 12
#define TEXTBOX_INTERIOR_LEFT 4
#define LEFT_OFFSET 80
		int box_x = center_x(BOX_WIDTH);
		int box_y = center_y(0) + 8;

		int focus = USERNAME_BOX;

		set_font_size(11);

		int username_label_left = LEFT_OFFSET - 2 - draw_string_width("Username:");
		int password_label_left = LEFT_OFFSET - 2 - draw_string_width("Password:");
		int hostname_label_left = width - 10 - draw_string_width(hostname);
		int kernel_v_label_left = 10;

		char password_circles[INPUT_SIZE * 3];

		int show_error = 0;

		while (1) {
			focus = USERNAME_BOX;
			memset(username, 0x0, INPUT_SIZE);
			memset(password, 0x0, INPUT_SIZE);
			memset(password_circles, 0x0, INPUT_SIZE * 3);

			while (1) {

				strcpy(password_circles, "");
				for (int i = 0; i < strlen(password); ++i) {
					strcat(password_circles, "●");
				}

				/* Redraw the background */
				draw_fill(ctx, rgb(0,0,0));

				draw_sprite(ctx, sprites[2], center_x(width), center_y(height));
				draw_sprite(ctx, sprites[0], center_x(sprites[0]->width), center_y(sprites[0]->height) - LOGO_FINAL_OFFSET);

				draw_string_shadow(ctx, hostname_label_left, height - 12, white, hostname, rgb(0,0,0), 2, 1, 1, 3.0);
				draw_string_shadow(ctx, kernel_v_label_left, height - 12, white, kernel_v, rgb(0,0,0), 2, 1, 1, 3.0);

				/* Draw backdrops */
				draw_box(ctx, box_x, box_y, BOX_WIDTH, BOX_HEIGHT, rgb(20,20,20));
				draw_box(ctx, box_x + LEFT_OFFSET, box_y + 32, 168, 16, rgb(255,255,255));
				draw_box(ctx, box_x + LEFT_OFFSET, box_y + 56, 168, 16, rgb(255,255,255));

				/* Draw labels */
				draw_string(ctx, box_x + username_label_left, box_y + 32 + EXTRA_TEXT_OFFSET, white, "Username:");
				draw_string(ctx, box_x + password_label_left, box_y + 56 + EXTRA_TEXT_OFFSET, white, "Password:");

				/* Draw box entries */
				draw_string(ctx, box_x + LEFT_OFFSET + TEXTBOX_INTERIOR_LEFT, box_y + 32 + EXTRA_TEXT_OFFSET, black, username);
				draw_string(ctx, box_x + LEFT_OFFSET + TEXTBOX_INTERIOR_LEFT, box_y + 56 + EXTRA_TEXT_OFFSET, black, password_circles);

				if (show_error) {
					char * error_message = "Incorrect username or password.";
					
					draw_string(ctx, box_x + (BOX_WIDTH - draw_string_width(error_message)) / 2, box_y + 8 + EXTRA_TEXT_OFFSET, red, error_message);
				}

				if (focus == USERNAME_BOX) {
					draw_box_border(ctx, box_x + LEFT_OFFSET, box_y + 32, 168, 16, rgb(8, 193, 236));
				} else if (focus == PASSWORD_BOX) {
					draw_box_border(ctx, box_x + LEFT_OFFSET, box_y + 56, 168, 16, rgb(8, 193, 236));
				}

				flip(ctx);
				yutani_flip(y, wina);

				w_keyboard_t * kbd = NULL;
				do {
					kbd = NULL;
					yutani_msg_t * msg = yutani_poll(y);
					if (msg->type == YUTANI_MSG_KEY_EVENT) {
						struct yutani_msg_key_event * ke = (void*)msg->data;
						if (ke->event.action == KEY_ACTION_DOWN) {
							kbd = malloc(sizeof(w_keyboard_t));
							kbd->key = ke->event.key;
						}
					}
					free(msg);
				} while (!kbd);

				if (kbd->key == '\n') {
					if (focus == USERNAME_BOX) {
						free(kbd);
						focus = PASSWORD_BOX;
						continue;
					} else if (focus == PASSWORD_BOX) {
						free(kbd);
						break;
					}
				}

				if (kbd->key == '\t') {
					if (focus == USERNAME_BOX) {
						focus = PASSWORD_BOX;
					} else if (focus == PASSWORD_BOX) {
						focus = USERNAME_BOX;
					}
					free(kbd);
					continue;
				}

				if (focus == USERNAME_BOX) {
					buffer_put(username, kbd->key);
				} else if (focus == PASSWORD_BOX) {
					buffer_put(password, kbd->key);
				}
				free(kbd);

			}

			uid = checkUserPass(username, password);

			if (uid >= 0) {
				break;
			}
			show_error = 1;
		}

		draw_fill(ctx, rgb(0,0,0));
		draw_sprite(ctx, sprites[2], center_x(width), center_y(height));
		draw_sprite(ctx, sprites[0], center_x(sprites[0]->width), center_y(sprites[0]->height) - LOGO_FINAL_OFFSET);
		flip(ctx);
		yutani_flip(y, wina);

		//teardown_windowing();
		yutani_close(y, wina);

		pid_t _session_pid = fork();
		if (!_session_pid) {
			setenv("PATH", "/usr/bin:/bin", 0);
			syscall_setuid(uid);
			char * args[] = {"/bin/gsession-beta", NULL};
			execvp(args[0], args);
		}

		free(buf);
		free(ctx->backbuffer);
		free(ctx);

		syscall_wait(_session_pid);
	}

	return 0;
}