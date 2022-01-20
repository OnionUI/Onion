#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <png.h>

//
//	Rumble ON(1) / OFF(0)
//
void rumble(uint32_t val) {
	int fd;
	const char str_export[] = "48";
	const char str_direction[] = "out";
	char value[1];
	value[0] = ((val&1)^1) + 0x30;

	fd = open("/sys/class/gpio/export",O_WRONLY);
		if (fd > 0) {
			write(fd, str_export, 2);
			close(fd);
		}
	fd = open("/sys/class/gpio/gpio48/direction",O_WRONLY);
		if (fd > 0) {
			write(fd, str_direction, 3);
			close(fd);
		}
	fd = open("/sys/class/gpio/gpio48/value",O_WRONLY);
		if (fd > 0) {
			write(fd, value, 1);
			close(fd);
		}
}

//
//	Screenshot (640x480, rotate180, png)
//
void screenshot(void) {
	char		screenshotname[32];
	uint32_t	linebuffer[640], i, x, y, pix1, pix2;
	FILE		*fp;
	int		fd_fb;
	struct		fb_var_screeninfo vinfo;
	png_structp	png_ptr;
	png_infop	info_ptr;

	for (i=0; i<100; i++) {
		snprintf(screenshotname,sizeof(screenshotname),"/mnt/SDCARD/screenshot%02d.png",i);
		if ( access(screenshotname, F_OK) != 0 ) break;
	} if (i > 99) return;

	fd_fb = open("/dev/fb0", O_RDWR);
	ioctl(fd_fb, FBIOGET_VSCREENINFO, &vinfo);

	fp = fopen(screenshotname, "wb");
	if (fp > 0) {
		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		info_ptr = png_create_info_struct(png_ptr);
		png_init_io(png_ptr, fp);
		png_set_IHDR(png_ptr, info_ptr, 640, 480, 8, PNG_COLOR_TYPE_RGBA,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png_ptr, info_ptr);
		for (y=480; y>0; y--) {
			lseek(fd_fb, 640*(vinfo.yoffset+y-1)*4, SEEK_SET);
			read(fd_fb,linebuffer,sizeof(linebuffer));
			for (x=320; x>0; x--){
				pix1 = linebuffer[320-x] | 0xFF000000;
				pix2 = linebuffer[320+x-1]  | 0xFF000000;
				pix1 = (pix1 & 0xFF00FF00) | (pix1 & 0x00FF0000)>>16 | (pix1 & 0x000000FF)<<16;
				pix2 = (pix2 & 0xFF00FF00) | (pix2 & 0x00FF0000)>>16 | (pix2 & 0x000000FF)<<16;
				linebuffer[320+x-1] = pix1;
				linebuffer[320-x] = pix2;
			}
			png_write_row(png_ptr, (png_bytep)linebuffer);
		}
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fclose(fp);
		sync();
	}
	close(fd_fb);
}

//
//	Screenshot Sample Main
//
#define	BUTTON_MENU	KEY_ESC
#define	BUTTON_POWER	KEY_POWER
#define	BUTTON_L2	KEY_TAB
#define	BUTTON_R2	KEY_BACKSPACE
int main() {
	int			input_fd;
	struct input_event	ev;
	uint32_t		val;
	uint32_t		l2_pressed = 0;
	uint32_t		r2_pressed = 0;

	// Prepare for Poll button input
	input_fd = open("/dev/input/event0", O_RDONLY);

	while( read(input_fd, &ev, sizeof(ev)) == sizeof(ev) ) {
		val = ev.value;
		if (( ev.type != EV_KEY ) || ( val > 1 )) continue;
		if (( ev.code == BUTTON_L2 )||( ev.code == BUTTON_R2 )) {
			if ( ev.code == BUTTON_L2 ) {
				l2_pressed = val;
			} else if ( ev.code == BUTTON_R2 ) {
				r2_pressed = val;
			}
			if (l2_pressed & r2_pressed) {
				rumble(1);
				screenshot();
				usleep(100000);	//0.1s
				rumble(0);
				l2_pressed = r2_pressed = 0;
			}
		}
	}

	// Quit
	close(input_fd);

	return 0;
}
