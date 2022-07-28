
//
//	Stock compatible custom keymon for miyoomini
//

#include <stdlib.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <linux/fb.h> 
#include <linux/input.h>
#include <linux/rtc.h>
#include <png.h>

   
//	Button Defines
#define BUTTON_UP	KEY_UP
#define BUTTON_DOWN	KEY_DOWN
#define BUTTON_LEFT	KEY_LEFT
#define BUTTON_RIGHT	KEY_RIGHT
#define BUTTON_A	KEY_SPACE
#define BUTTON_B	KEY_LEFTCTRL
#define BUTTON_X	KEY_LEFTSHIFT
#define BUTTON_Y	KEY_LEFTALT
#define BUTTON_L1	KEY_E
#define BUTTON_R1	KEY_T
#define	BUTTON_L2	KEY_TAB
#define	BUTTON_R2	KEY_BACKSPACE
#define	BUTTON_SELECT	KEY_RIGHTCTRL
#define	BUTTON_START	KEY_ENTER
#define	BUTTON_POWER	KEY_POWER
#define	BUTTON_MENU	KEY_ESC

//	for ev.value
#define RELEASED	0
#define PRESSED		1
#define REPEAT		2

//	for button_flag
#define SELECT_BIT	0
#define START_BIT	1
#define L2_BIT		2
#define R2_BIT		3
#define SELECT		(1<<SELECT_BIT)
#define START		(1<<START_BIT)
#define L2		(1<<L2_BIT)
#define R2		(1<<R2_BIT)

//	for proc_stat flags
#define PF_KTHREAD	0x00200000

//	for JSON
#define JSONKEYSMAX	14
#define JSONKEYSIZE	16
#define JSONDEFSIZE	20
#define JSONSTRSIZE	128
#define VOLMAX		20
#define BRIMAX		10
typedef enum {
	JSON_VOLUME,		// vol
	JSON_KEYMAP,		// keymap (unused)
	JSON_MUTE,		// mute
	JSON_BGM_VOLUME,	// bgmvol
	JSON_BRIGHTNESS,	// brightness
	JSON_LANGUAGE,		// language
	JSON_HIBERNATE_DELAY,	// hibernate
	JSON_LUMINATION,	// lumination
	JSON_HUE,		// hue
	JSON_SATURATION,	// saturation
	JSON_CONTRAST,		// contrast
	JSON_THEME,		// theme
	JSON_FONTSIZE,		// fontsize
	JSON_AUDIOFIX,		// audiofix
	JSON_VALUE_MAX,
} JsonValue;
const char jsonname[] = "/appconfigs/system.json";
const char jsonkeyname[JSONKEYSMAX][JSONKEYSIZE] = {
	"\"vol\"","\"keymap\"","\"mute\"","\"bgmvol\"","\"brightness\"","\"language\"","\"hibernate\"",
	"\"lumination\"","\"hue\"","\"saturation\"","\"contrast\"","\"theme\"","\"fontsize\"","\"audiofix\""};
char	jsonvalue[JSONKEYSMAX][JSONSTRSIZE];

//	for read battery
#define SARADC_IOC_MAGIC                     'a'
#define IOCTL_SAR_INIT                       _IO(SARADC_IOC_MAGIC, 0)
#define IOCTL_SAR_SET_CHANNEL_READ_VALUE     _IO(SARADC_IOC_MAGIC, 1)
typedef struct {
	int channel_value;
	int adc_value;
} SAR_ADC_CONFIG_READ;

//	system directory
#define	GPIO_DIR1	"/sys/class/gpio/"
#define	GPIO_DIR2	"/sys/devices/gpiochip0/gpio/"
#define	PWM_DIR		"/sys/devices/soc0/soc/1f003400.pwm/pwm/pwmchip0/"
#define	CPU_DIR		"/sys/devices/system/cpu/cpufreq/policy0/"
#define concat(ptr,str1,str2)	{ strcpy(ptr, str1); strcat(ptr, str2); }

//	libshmvar header
typedef enum {
	MONITOR_VOLUME,		// vol
	MONITOR_BRIGHTNESS,	// brightness
	MONITOR_KEYMAP,		// keymap (unused)
	MONITOR_MUTE,		// mute
	MONITOR_VOLUME_CHANGED,	// volume change (internal use)
	MONITOR_BGM_VOLUME,	// bgmvol
	MONITOR_HIBERNATE_DELAY,// hibernate
	MONITOR_ADC_VALUE,	// charging state (internal use)
	MONITOR_LUMINATION,	// lumination
	MONITOR_HUE,		// hue
	MONITOR_SATURATION,	// saturation
	MONITOR_CONTRAST,	// contrast
	MONITOR_UNUSED,		// (unused)
	MONITOR_AUDIOFIX,	// audiofix
	MONITOR_VALUE_MAX,
} MonitorValue;

typedef struct _KeyShmInfo {
	int id;
	void *addr;
} KeyShmInfo;

int	InitKeyShm(KeyShmInfo *);
int	SetKeyShm(KeyShmInfo* info, MonitorValue key, int value);
int	GetKeyShm(KeyShmInfo* info, MonitorValue key);
int	UninitKeyShm(KeyShmInfo *);

//	for clock / LCD
#define	CHR_WIDTH	(3 * 4 + 4)
#define	CHR_HEIGHT	(5 * 4)
#define	DISPLAY_WIDTH	(CHR_WIDTH * 19)
#define	DISPLAY_HEIGHT	(CHR_HEIGHT + 4)
enum { YEAR, MON, DAY, HOUR, MIN, SEC };
enum { LUM, HUE, SAT, CON };
typedef struct {
	uint32_t dev;		// fixed 0
	uint32_t eCscMatrix;	// fixed 3
	uint32_t u32Luma;	// luminance :  0 ~ 100
	uint32_t u32Contrast;	// contrast  :  0 ~ 100
	uint32_t u32Hue;	// hue       :  0 ~ 100
	uint32_t u32Saturation;	// saturation:  0 ~ 100
	uint32_t u32Sharpness;	// fixed 0
} MI_DISP_LcdParam;
typedef struct {
	uint32_t Luma;		// luminance :  0 ~ 20
	uint32_t Contrast;	// contrast  :  0 ~ 20
	uint32_t Hue;		// hue       :  0 ~ 20
	uint32_t Saturation;	// saturation:  0 ~ 20
} lcdprm;

//	for suspend / hibernate
#define CHECK_SEC	15	// Interval sec to check hibernate/adc
#define	SHUTDOWN_MIN	1	// Minutes to power off after hibernate
#define REPEAT_SEC(val)	((val * 1000 - 250) / 50)
#define PIDMAX 	32
uint32_t	suspendpid[PIDMAX];

//	Global Variables
int		fb_fd, input_fd, sar_fd, disp_fd;
uint32_t*	fb_addr;
uint8_t*	fbofs;
struct		fb_fix_screeninfo finfo;
struct		fb_var_screeninfo vinfo;
struct		input_event ev;
struct		pollfd fds[1];
int		adcvalue;
pthread_t	adc_pt;
uint32_t	adcthread_active , clkthread_active;
KeyShmInfo	shminfo;
struct		tm clk;
pthread_t	clock_pt;
pthread_mutex_t	clock_mx;
uint32_t	stride, bpp;
uint8_t*	savebuf;
uint32_t	focusC = HOUR;
uint32_t	focusL = LUM;
lcdprm		lcd;
char		clockfilename[128];
int		onion;

//
//	Trim Strings for reading json
//
char* trimstr(char* str, uint32_t first) {
	char *firstchar, *firstlastchar, *lastfirstchar, *lastchar;
	uint32_t i;

	firstchar = firstlastchar = lastfirstchar = lastchar = 0;

	for (i=0; i<strlen(str); i++) {
		if ((str[i]!='\r')&&(str[i]!='\n')&&(str[i]!=' ')&&(str[i]!='\t')&&
		    (str[i]!='{')&&(str[i]!='}')&&(str[i]!=',')) {
			if (!firstchar) {
				firstchar = &str[i];
				lastfirstchar = &str[i];
			}
			if (i) {
				if ((str[i-1]=='\r')||(str[i-1]=='\n')||(str[i-1]==' ')||(str[i-1]=='\t')||
				    (str[i-1]=='{')||(str[i-1]=='}')||(str[i-1]==',')) {
					lastfirstchar = &str[i];
				}
			}
			if (str[i] == '"') {
				for (i++; i<(strlen(str)-1); i++) {
					if ((str[i]=='\r')||(str[i]=='\n')||(str[i]=='"')) break;
				}
			}
			lastchar = &str[i];
		} else {
			if (!firstlastchar) {
				firstlastchar = lastchar;
			}
		}
	}
	if (first) {
		lastfirstchar = firstchar;
		lastchar = firstlastchar;
	}
	if (lastchar) {
		lastchar[1] = 0;
	}
	if (lastfirstchar) return lastfirstchar;
	return 0;
}


//
//	Read system.json
//
void readJson(void) {
	FILE *fp;
	char key[256];
	char val[256];
	char *keyptr, *valptr;
	int f;
	uint32_t i;

	if ((fp = fopen(jsonname, "r"))) {
		key[0] = 0; val[0] = 0;
		while ((f = fscanf(fp, "%255[^:]:%255[^\n]\n", key, val)) != EOF) {
			if (!f) { if (fscanf(fp, "%*[^\n]\n") == EOF) break; else continue; }
			if (!(keyptr = trimstr(key, 0))) continue;
			if (!(valptr = trimstr(val, 1))) { val[0] = 0; valptr = val; }
			for (i=0; i<JSONKEYSMAX; i++) {
				if (!strcmp(keyptr, jsonkeyname[i])) {
					strncpy(jsonvalue[i], valptr, JSONSTRSIZE-1);
					break;
				}
			}
			key[0] = 0; val[0] = 0;
		}
		fclose(fp);
	}
}

char* load_file(char const* path)
{
    char* buffer = 0;
    long length;
    FILE * f = fopen (path, "rb"); 

    if (f)
    {
      fseek (f, 0, SEEK_END);
      length = ftell (f);
      fseek (f, 0, SEEK_SET);
      buffer = (char*)malloc ((length+1)*sizeof(char));
      if (buffer)
      {
        fread (buffer, sizeof(char), length, f);
      }
      fclose (f);
    }
    buffer[length] = '\0';

    return buffer;
}



//
//	Write system.json
//
void writeJson(void) {
	FILE *fp;
	uint32_t i;

	if ((fp = fopen(jsonname, "w"))) {
		fputs("{\n\t",fp);
		for (i=0; i<JSONKEYSMAX; i++) {
			fputs(&jsonkeyname[i][0],fp);
			fputs(":\t",fp);
			fputs(&jsonvalue[i][0],fp);
			if (i != JSONKEYSMAX-1) {
				fputs(",\n\t",fp);
			} else {
				fputs("\n}",fp);
			}
		}
		fflush(fp);
		fsync(fileno(fp));
		fclose(fp);
	}
}

//
//	Get ADC Value
//
int GetADCValue(void) {
	char charging = 0;
	char filename[128];
	const char str_export[] = "59";
	const char str_direction[] = "in";
	int fd;

	concat(filename, GPIO_DIR2, "gpio59/value");
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		// export gpio59, direction: in
		concat(filename, GPIO_DIR1, "export");
		fd = open(filename, O_WRONLY); if (fd >= 0) { write(fd, str_export, 2); close(fd); }
		concat(filename, GPIO_DIR2, "gpio59/direction");
		fd = open(filename, O_WRONLY); if (fd >= 0) { write(fd, str_direction, 2); close(fd); }
		concat(filename, GPIO_DIR2, "gpio59/value");
		fd = open(filename, O_RDONLY);
	}
	if (fd >= 0) { read(fd, &charging, 1); close(fd); }
	if (charging == '1') {
		adcvalue = 100;
		return adcvalue;
	}

	if (!sar_fd) {
		sar_fd = open("/dev/sar", O_WRONLY);
		ioctl(sar_fd, IOCTL_SAR_INIT, NULL);
	}

	static SAR_ADC_CONFIG_READ adcCfg;
	ioctl(sar_fd, IOCTL_SAR_SET_CHANNEL_READ_VALUE, &adcCfg);
	if (adcvalue <= 100) adcvalue = adcCfg.adc_value;
	else if ( adcCfg.adc_value > adcvalue ) adcvalue++;
	else if ( adcCfg.adc_value < adcvalue ) adcvalue--;
	return adcvalue;
}

//
//	Get numeric value from json
//
uint32_t getValue(uint32_t key) {
	if (key < JSONKEYSMAX) {
		return (uint32_t)atoi(jsonvalue[key]);
	} else return 0;
}

//
//	Set numeric value to json
//
void setValue(uint32_t key,uint32_t val) {
	if (key < JSONKEYSMAX) {
		snprintf(jsonvalue[key], JSONSTRSIZE, "%u", val);
	}
}

//
//	Init json / shm
//
void initJson(void) {
	// set default value
	strcpy(jsonvalue[JSON_VOLUME],		"20");
	strcpy(jsonvalue[JSON_KEYMAP],		"\"L2,L,R2,R,X,A,B,Y\"");
	strcpy(jsonvalue[JSON_MUTE],		"0");
	strcpy(jsonvalue[JSON_BGM_VOLUME],	"8");
	strcpy(jsonvalue[JSON_BRIGHTNESS],	"7");
	strcpy(jsonvalue[JSON_LANGUAGE],	"\"en.lang\"");
	strcpy(jsonvalue[JSON_HIBERNATE_DELAY],	"0");
	strcpy(jsonvalue[JSON_LUMINATION],	"7");
	strcpy(jsonvalue[JSON_HUE],		"10");
	strcpy(jsonvalue[JSON_SATURATION],	"10");
	strcpy(jsonvalue[JSON_CONTRAST],	"10");
	strcpy(jsonvalue[JSON_THEME],		"\"./\"");
	strcpy(jsonvalue[JSON_FONTSIZE],	"24");
	strcpy(jsonvalue[JSON_AUDIOFIX],	"1");

	readJson();

	// audiofix fix for older FW
	if (access("/customer/app/audioserver", F_OK)) {
		strcpy(jsonvalue[JSON_AUDIOFIX], "0");
	}

	InitKeyShm(&shminfo);
	SetKeyShm(&shminfo, MONITOR_VOLUME,	     getValue(JSON_VOLUME));
	SetKeyShm(&shminfo, MONITOR_BRIGHTNESS,	     getValue(JSON_BRIGHTNESS));
	SetKeyShm(&shminfo, MONITOR_KEYMAP,	     0);
	SetKeyShm(&shminfo, MONITOR_MUTE,	     getValue(JSON_MUTE));
	SetKeyShm(&shminfo, MONITOR_VOLUME_CHANGED,  0);
	SetKeyShm(&shminfo, MONITOR_BGM_VOLUME,	     getValue(JSON_BGM_VOLUME));
	SetKeyShm(&shminfo, MONITOR_HIBERNATE_DELAY, getValue(JSON_HIBERNATE_DELAY));
	SetKeyShm(&shminfo, MONITOR_LUMINATION,	     getValue(JSON_LUMINATION));
	SetKeyShm(&shminfo, MONITOR_HUE,	     getValue(JSON_HUE));
	SetKeyShm(&shminfo, MONITOR_SATURATION,	     getValue(JSON_SATURATION));
	SetKeyShm(&shminfo, MONITOR_CONTRAST,	     getValue(JSON_CONTRAST));
	SetKeyShm(&shminfo, MONITOR_AUDIOFIX,	     getValue(JSON_AUDIOFIX));

	GetADCValue();
	SetKeyShm(&shminfo, MONITOR_ADC_VALUE, onion ? 640 : adcvalue);
}

//
//	Set Brightness (Raw)
//
void setBrightnessRaw(uint32_t brightness) {
	FILE	*fp;
	char filename[128];

	concat(filename, PWM_DIR, "pwm0/duty_cycle");
	if ( (fp = fopen(filename, "w")) ) {
		fprintf(fp, "%u", brightness);
		fclose(fp);
	}
}

//
//	Set Brightness
//
void setBrightness(uint32_t val) {
	setValue(JSON_BRIGHTNESS, val);
	SetKeyShm(&shminfo, MONITOR_BRIGHTNESS, val);
	setBrightnessRaw((val == 0) ? 6 : (val * 10));
}

//
//	Set Volume (Raw)
//
#define MI_AO_SETVOLUME	0x4008690b
#define MI_AO_GETVOLUME	0xc008690c
int setVolumeRaw(int volume, int add) {
	int recent_volume = 0;
	int fd = open("/dev/mi_ao", O_RDWR);
	if (fd >= 0) {
		int buf2[] = {0, 0};
		uint64_t buf1[] = {sizeof(buf2), (uintptr_t)buf2};
		ioctl(fd, MI_AO_GETVOLUME, buf1);
		recent_volume = buf2[1];
		if (add) {
			buf2[1] += add;
			if (buf2[1] > 30) buf2[1] = 30;
			else if (buf2[1] < -30) buf2[1] = -30;
		} else buf2[1] = volume;
		if (buf2[1] != recent_volume) ioctl(fd, MI_AO_SETVOLUME, buf1);
		close(fd);
	}
	return recent_volume;
}

//
//	Screen On/Off
//
void setScreen(uint32_t val) {
	int	fd;
	const char str_export[] = "4";
	const char str_direction[] = "out";
	const char value0[] = "0";
	const char value1[] = "1";
	char filename[128];

	// export gpio4, direction: out
	concat(filename, GPIO_DIR1, "export");
	fd = open(filename, O_WRONLY); if (fd >= 0) { write(fd, str_export, 1); close(fd); }
	concat(filename, GPIO_DIR2, "gpio4/direction");
	fd = open(filename, O_WRONLY); if (fd >= 0) { write(fd, str_direction, 3); close(fd); }

	// screen on/off
	concat(filename, GPIO_DIR2, "gpio4/value");
	fd = open(filename, O_WRONLY); if (fd >= 0) { write(fd, val ? value1 : value0, 1); close(fd); }

	// unexport gpio4
	concat(filename, GPIO_DIR1, "unexport");
	fd = open(filename, O_WRONLY); if (fd >= 0) { write(fd, str_export, 1); close(fd); }

	if (val) {
		// re-enable brightness control
		concat(filename, PWM_DIR, "export");
		fd = open(filename, O_WRONLY);  if (fd >= 0) { write(fd, value0, 1); close(fd); }
		concat(filename, PWM_DIR, "pwm0/enable");
		fd = open(filename, O_WRONLY);  if (fd >= 0) { write(fd, value0, 1); close(fd); }
		fd = open(filename, O_WRONLY);  if (fd >= 0) { write(fd, value1, 1); close(fd); }
	}
}

//
//	Search pid of running executable (forward match)
//
pid_t searchpid(const char *commname) {
	DIR *procdp;
	struct dirent *dir;
	char fname[24];
	char comm[128];
	pid_t pid;
	pid_t ret = 0;
	size_t commlen = strlen(commname);

	procdp = opendir("/proc");
	while ((dir = readdir(procdp))) {
		if (dir->d_type == DT_DIR) {
			pid = atoi(dir->d_name);
			if ( pid > 2 ) {
				sprintf(fname, "/proc/%d/comm", pid);
				FILE *fp = fopen(fname, "r");
				if (fp) {
					fscanf(fp, "%127s", comm);
					fclose(fp);
					if (!strncmp(comm, commname, commlen)) { ret = pid; break; }
				}
			}
		}
	}
	closedir(procdp);
	return ret;
}

//
//	Rumble ON(1) / OFF(0)
//
void rumble(uint32_t val) {
	int fd;
	const char str_export[] = "48";
	const char str_direction[] = "out";
	char value[1];
	value[0] = ((val&1)^1) + 0x30;
	char filename[128];

	concat(filename, GPIO_DIR1, "export");
	fd = open(filename, O_WRONLY); if (fd >= 0) { write(fd, str_export, 2); close(fd); }
	concat(filename, GPIO_DIR2, "gpio48/direction");
	fd = open(filename, O_WRONLY); if (fd >= 0) { write(fd, str_direction, 3); close(fd); }
	concat(filename, GPIO_DIR2, "gpio48/value");
	fd = open(filename, O_WRONLY); if (fd >= 0) { write(fd, value, 1); close(fd); }
}

//
//	Send short vibration pulse
//
void short_pulse(void) {
	if (access("/mnt/SDCARD/.tmp_update/.noVibration", F_OK)!=0){
		rumble(1);
		usleep(100000);		// 0.1s
		rumble(0);		
	}
			
}
void super_short_pulse(void) {
	if (access("/mnt/SDCARD/.tmp_update/.noVibration", F_OK)!=0){
		rumble(1);
		usleep(40000);		// 0.05s
		rumble(0);			
	}	
}
//
//	Terminate retroarch before kill/shotdown processes to save progress
//
int terminate_retroarch(void) {
	char fname[16];
	pid_t pid = searchpid("retroarch");
	if (!pid) pid = searchpid("ra32");

	if (pid) {

		// send signal
		kill(pid, SIGCONT); usleep(100000); kill(pid, SIGTERM);
		// wait for terminate
		sprintf(fname, "/proc/%d", pid);
		uint32_t count = 20;		// 4s
		while ((--count)&&(!access(fname, F_OK))) {
			usleep(200000);		// 0.2s
		}

		return 1;
	}
	return 0;
}

//
//	Suspend / Kill processes
//		mode: 0:STOP 1:TERM 2:KILL
//
int suspend(uint32_t mode) {
	DIR *procdp;
	struct dirent *dir;
	char fname[32];
	pid_t suspend_pid = getpid();
	pid_t pid;
	pid_t ppid;
	char state;
	uint32_t flags;
	char comm[128];
	int ret = 0;

	// terminate retroarch before kill
	if (mode == 2) ret = terminate_retroarch();

	sync();
	procdp = opendir("/proc");

	// Pick active processes to suspend and send SIGSTOP
	// Cond:1. PID is greater than 2(kthreadd) and not myself
	//	2. PPID is greater than 2(kthreadd)
	//	3. state is "R" or "S" or "D"
	//	4. comm is not "(sh)" / "(MainUI)" when AudioFix:OFF
	//	5. flags does not contain PF_KTHREAD (0x200000) (just in case)
	//	6. comm is not "(updater)" "(MainUI)" "(tee)" "(audioserver*" when kill mode
	if (!mode) suspendpid[0] = 0;
	while ((dir = readdir(procdp))) {
		if (dir->d_type == DT_DIR) {
			pid = atoi(dir->d_name);
			if (( pid > 2 )&&( pid != suspend_pid )) {
				sprintf(fname, "/proc/%d/stat", pid);
				FILE *fp = fopen(fname, "r");
				if (fp) {
					fscanf(fp, "%*d %127s %c %d %*d %*d %*d %*d %u", (char*)&comm, &state, &ppid, &flags);
					fclose(fp);
				}
				if ( (ppid > 2) && ((state == 'R')||(state == 'S')||(state == 'D')) &&
				     (strcmp(comm,"(sh)")) && (!(flags & PF_KTHREAD)) ) {
					if (mode) {
						if ( (strcmp(comm,"(updater)")) && (strcmp(comm,"(MainUI)"))
						  && (strcmp(comm,"(tee)")) && (strncmp(comm,"(audioserver",12)) ) {
							kill(pid, (mode == 1) ? SIGTERM : SIGKILL); ret++;
						}
					} else {
						// sometimes MainUI bgm goes wrong when audiofix:off
						if (!strcmp(comm,"(MainUI)")) {
							if (!GetKeyShm(&shminfo, MONITOR_AUDIOFIX)) continue;
						}
						if ( suspendpid[0] < PIDMAX ) {
							suspendpid[++suspendpid[0]] = pid;
							kill(pid, SIGSTOP); ret++;
						}
					}
				}
			}
		}
	}
	closedir(procdp);

	// reset FB when anything killed
	if ((mode == 2)&&(ret)) {
		ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
		vinfo.yoffset = 0;
		memset(fb_addr, 0, finfo.smem_len);
		ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo);
	}

	return ret;
}

//
//	Resume
//
void resume(void) {
	// Send SIGCONT to suspended processes
	if (suspendpid[0]) {
		for (uint32_t i=1; i <= suspendpid[0]; i++) {
			kill(suspendpid[i], SIGCONT);
		}
		suspendpid[0] = 0;
		super_short_pulse();
	}
}

//
//	[onion] get recent filename from content_history.lpl
//
char* getrecent_onion(char *filename) {
	FILE	*fp;
	char	key[256], val[256];
	char	*keyptr, *valptr, *strptr;
	int	f;

	*filename = 0;
	if ( (fp = fopen("/mnt/SDCARD/Saves/CurrentProfile/lists/content_history.lpl", "r")) ) {
		key[0] = 0; val[0] = 0;
		while ((f = fscanf(fp, "%255[^:]:%255[^\n]\n", key, val)) != EOF) {
			if (!f) { if (fscanf(fp, "%*[^\n]\n") == EOF) break; else continue; }
			if ( ((keyptr = trimstr(key, 0))) && ((valptr = trimstr(val, 1))) ) {
				if ( (!strcmp(keyptr, "\"path\"")) && ((valptr = strrchr(valptr, '/'))) ) {
					valptr++;
					if ((strptr = strrchr(valptr, '"'))) *strptr = 0;
					strcpy(filename, valptr);
					break;
				}
			}
			key[0] = 0; val[0] = 0;
		}
		fclose(fp);
	}
	if (*filename == 0) return NULL;
	return filename;
}

//
//	Get most recent file name for screenshot
//
char* getrecent_png(char *filename) {
	FILE		*fp;
	char		*fnptr,	*strptr;
	uint32_t	i;

	strcpy(filename, "/mnt/SDCARD/Screenshots/");
	if (access(filename, F_OK)) mkdir(filename, 777);

	fnptr = filename + strlen(filename);

	if (!access("/tmp/cmd_to_run.sh", F_OK)) {
		// for stock
		if ((fp = fopen("/mnt/SDCARD/Roms/recentlist.json", "r"))) {
			fscanf(fp, "%*255[^:]:\"%255[^\"]", fnptr);
			fclose(fp);
		}
	} else if (!access("/tmp/cmd_to_run_launcher.sh", F_OK)) {
		// for onionLauncher
		if (getrecent_onion(fnptr)) {
			if ((strptr = strrchr(fnptr, '.'))) *strptr = 0;
		}
	}

	if (!(*fnptr)) {
		if (searchpid("onionLauncher")) strcat(filename, "onionLauncher");
		else strcat(filename, "MainUI");
	}

	fnptr = filename + strlen(filename);
	for (i=0; i<1000; i++) {
		sprintf(fnptr, "_%03d.png", i);
		if ( access(filename, F_OK) != 0 ) break;
	} if (i > 999) return NULL;
	return filename;
}

//
//	Screenshot (640x480x32bpp only, rotate180, png)
//
void screenshot(void) {
	char		screenshotname[512];
	uint32_t	*buffer;
	uint32_t	*src;
	uint32_t	linebuffer[640], x, y, pix;
	FILE		*fp;
	png_structp	png_ptr;
	png_infop	info_ptr;

	if (getrecent_png(screenshotname) == NULL) return;

	ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
	buffer = fb_addr + 640*vinfo.yoffset;

	if ((fp = fopen(screenshotname, "wb"))) {
		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		info_ptr = png_create_info_struct(png_ptr);
		png_init_io(png_ptr, fp);
		png_set_IHDR(png_ptr, info_ptr, 640, 480, 8, PNG_COLOR_TYPE_RGBA,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png_ptr, info_ptr);
		src = buffer + 640*480;
		for (y=0; y<480; y++) {
			for (x=0; x<640; x++){
				pix = *--src;
				linebuffer[x] = 0xFF000000 | (pix & 0x0000FF00) | (pix & 0x00FF0000)>>16 | (pix & 0x000000FF)<<16;
			}
			png_write_row(png_ptr, (png_bytep)linebuffer);
			
		}
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fflush(fp);
		fsync(fileno(fp));
		fclose(fp);
	}
}


//
//	Screenshot (640x480x32bpp only, rotate180, png)
//
void screenshot_onion(void) {
	char		screenshotname[512]="/mnt/SDCARD/.tmp_update/screenshotGame.png";
	uint32_t	*buffer;
	uint32_t	*src;
	uint32_t	linebuffer[640], x, y, pix;
	FILE		*fp;
	png_structp	png_ptr;
	png_infop	info_ptr;

	ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
	buffer = fb_addr + 640*vinfo.yoffset;

	if ((fp = fopen(screenshotname, "wb"))) {
		png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		info_ptr = png_create_info_struct(png_ptr);
		png_init_io(png_ptr, fp);
		png_set_IHDR(png_ptr, info_ptr, 640, 480, 8, PNG_COLOR_TYPE_RGBA,
			PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
		png_write_info(png_ptr, info_ptr);
		src = buffer + 640*480;
		for (y=0; y<480; y++) {
			for (x=0; x<640; x++){
				pix = *--src;
				linebuffer[x] = 0xFF000000 | (pix & 0x0000FF00) | (pix & 0x00FF0000)>>16 | (pix & 0x000000FF)<<16;
			}
			png_write_row(png_ptr, (png_bytep)linebuffer);
			
		}
		png_write_end(png_ptr, info_ptr);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		fflush(fp);
		fsync(fileno(fp));
		fclose(fp);
	}
}

//
//	set CPU governor
//		mode: 0: save 1: restore
//
void setCPUsave(uint32_t mode) {
	static uint32_t minfreq;
	static char gov[16];
	char fn_min_freq[128];
	char fn_governor[128];
	const char powersave[] = "powersave";
	FILE* fp;
	concat(fn_min_freq, CPU_DIR, "scaling_min_freq");
	concat(fn_governor, CPU_DIR, "scaling_governor");

	if (!mode) {
		/* save min_freq */
		fp = fopen(fn_min_freq, "r");
		if (fp) { fscanf(fp, "%u", &minfreq); fclose(fp); }
		/* set min_freq to lowest */
		fp = fopen(fn_min_freq, "w");
		if (fp) { fprintf(fp, "%u", 0); fclose(fp); }
		/* save governor */
		fp = fopen(fn_governor, "r");
		if (fp) { fscanf(fp, "%15s", gov); fclose(fp); }
		/* set governor to powersave */
		fp = fopen(fn_governor, "w");
		if (fp) { fprintf(fp, "%s", powersave); fclose(fp); }
	} else {
		/* restore min_freq */
		fp = fopen(fn_min_freq, "w");
		if (fp) { fprintf(fp, "%u", minfreq); fclose(fp); }
		/* restore governor */
		fp = fopen(fn_governor, "w");
		if (fp) { fprintf(fp, "%s", gov); fclose(fp); }
	}
}

//
//	Draw frame, fixed 640x480x32bpp for now
//
void drawFrame(uint32_t color) {
	uint32_t* ofs = fb_addr;
	uint32_t i;
	for(i=0; i<640; i++) { ofs[i] = color; }
	ofs += 640*479;
	for(i=0; i<640*2; i++) { ofs[i] = color; }
	ofs += 640*480;
	for(i=0; i<640*2; i++) { ofs[i] = color; }
	ofs += 640*480;
	for(i=0; i<640; i++) { ofs[i] = color; }
	ofs = fb_addr + 639;
	for(i=0; i<480*3-1; i++, ofs+=640) { ofs[0] = color; ofs[1] = color; }
}

//
//	Draw Battery warning thread
//
static void* batteryWarnThread(void* param) {
	while (1) {
		drawFrame(0x00FF0000); // draw red frame
		usleep(0x4000);
	}
	return 0;
}

//
//	Check Battery warning
//
uint32_t checkBatteryWarning(void) {
	if ((adcvalue < 506)&&(adcvalue != 100)) return 1;
//	if ( ( (adcvalue < 506) && (adcvalue != 100) /* && (!access("/tmp/cmd_to_run.sh",F_OK)) */ )
//		|| (!access("/tmp/debug_show_battery_icon",F_OK)) ) return 1;
	return 0;
}

//
//	Update ADC Value
//
void updateADC(int mode) {
	if (mode) adcvalue = 0; // reset ADC value
	GetADCValue();
	if (!onion) SetKeyShm(&shminfo, MONITOR_ADC_VALUE, adcvalue);
	uint32_t warn = checkBatteryWarning();
	if ((!adcthread_active)&&(warn)) {
		pthread_create(&adc_pt, NULL, batteryWarnThread, NULL);
		adcthread_active = 1;
	} else if ((adcthread_active)&&(!warn)) {
		pthread_cancel(adc_pt); pthread_join(adc_pt, NULL);
		drawFrame(0); // erase red frame
		adcthread_active = 0;
	}
}

//
//	Read Clock (system)
//
void read_clock(void) {
	struct timeval tod;
	gettimeofday(&tod, NULL);
	gmtime_r(&tod.tv_sec, &clk);
}

//
//	Read Clock (RTC)
//
void read_clock_rtc(void) {
	int cfd = open("/dev/rtc0", O_RDONLY);
	if (cfd > 0) {
		ioctl(cfd, RTC_RD_TIME, &clk);
		close(cfd);
	} else	read_clock();
}

//
//	Write Clock (system)
//
void write_clock(void) {
	struct timeval tod;
	gettimeofday(&tod, NULL);
	tod.tv_sec = mktime(&clk);
	settimeofday(&tod, NULL);
}

//
//	Write Clock (RTC & system)
//
void write_clock_rtc(void) {
	int cfd = open("/dev/rtc0", O_WRONLY);
	if (cfd >= 0) { ioctl(cfd, RTC_SET_TIME, &clk); close(cfd); }
	write_clock();
}

//
//	Write clock.txt
//
void write_clockfile(void) {
	FILE* fp;

	if (!onion) {
		if ( (fp = fopen(clockfilename, "w")) ) {
			fprintf(fp, "%04u/%02u/%02u %02u:%02u:%02u",
				clk.tm_year+1900, clk.tm_mon+1, clk.tm_mday, clk.tm_hour, clk.tm_min, clk.tm_sec);
			fflush(fp);
			fsync(fileno(fp));
			fclose(fp);
		}
	} else {
		if ( (fp = fopen("/mnt/SDCARD/Saves/CurrentProfile/saves/currentTime.txt", "w")) ) {
			fprintf(fp, "%ld", mktime(&clk));
			fflush(fp);
			fsync(fileno(fp));
			fclose(fp);
		}
	}
}

//
//	Initialize Clock
//
void init_clock(void) {
	// make clock filename
	char	exepath[32];
	snprintf(exepath, 32, "/proc/%d/exe", getpid());
	readlink(exepath, clockfilename, 128-10);
	strcat(clockfilename, "_clock.txt");

	read_clock_rtc();
	write_clock();

	FILE* fp = fopen(clockfilename, "r");
	if (fp) {
		time_t now = mktime(&clk);
		fscanf(fp, "%04u/%02u/%02u %02u:%02u:%02u",
			&clk.tm_year, &clk.tm_mon, &clk.tm_mday, &clk.tm_hour, &clk.tm_min, &clk.tm_sec);
		fclose(fp);
		clk.tm_year -= 1900; clk.tm_mon--;
		if ( now < mktime(&clk) ) write_clock_rtc();
	}
}

//
//	Quit
//
void quit(int exitcode) {
	if (clkthread_active) { pthread_cancel(clock_pt); pthread_join(clock_pt, NULL); }
	if (adcthread_active) { pthread_cancel(adc_pt); pthread_join(adc_pt, NULL); }
	if (savebuf) free(savebuf);
	if (fb_addr) munmap(fb_addr, finfo.smem_len);
	if (fb_fd > 0) close(fb_fd);
	if (input_fd > 0) close(input_fd);
	if (sar_fd > 0) close(sar_fd);
	if (disp_fd > 0) close(disp_fd);
	read_clock();
	write_clock_rtc();
	write_clockfile();
	exit(exitcode);
}

//
//	Shutdown
//
void shutdown(void) {
	write_offorder();
	terminate_retroarch();
	read_clock();
	write_clockfile();
	sync();
	reboot(RB_AUTOBOOT);
	while(1) pause();
	exit(0);
}

//
//	Print Number / Character
//
void print_num(uint8_t num, uint32_t x, uint32_t color) {
	const uint16_t pix[16] = {
		0b000000000000000,	// space
		0b000001010100000,	// /
		0b111101101101111,	// 0
		0b001001001001001,	// 1
		0b111001111100111,	// 2
		0b111001111001111,	// 3
		0b101101111001001,	// 4
		0b111100111001111,	// 5/S
		0b111100111101111,	// 6
		0b111001001001001,	// 7
		0b111101111101111,	// 8
		0b111101111001111,	// 9
		0b000010000010000,	// :
		0b100100100100111,	// L
		0b101101111101101,	// H
		0b111100100100111 };	// C
	uint32_t	c32, i, y;
	uint16_t	numpix, c16;
	uint8_t		*ofs;
	uint16_t	*ofs16;
	uint32_t	*ofs32;
	uint32_t	s16 = stride / 2;
	uint32_t	s32 = stride / 4;

	switch(num) {
		case ' ': num = 0; break;
		case 'S': num = 7; break;
		case 'L': num = 13; break;
		case 'H': num = 14; break;
		case 'C': num = 15; break;
		default: num -= 0x2e; break;
	}
	if ( ( num > 15 ) || ( x > 18 ) ) return;
	numpix = pix[num];
	ofs = fbofs + ((18-x) * CHR_WIDTH * bpp);

	for (y=5; y>0; y--, ofs+=stride*4) {
		if (bpp == 4) {
			ofs32 = (uint32_t*)ofs;
			for (i=3; i>0; i--, numpix >>= 1, ofs32 += 4) {
				c32 = (numpix & 1) ? color : 0;
				ofs32[0] = c32; ofs32[1] = c32; ofs32[2] = c32; ofs32[3] = c32;
				ofs32[s32+0] = c32; ofs32[s32+1] = c32; ofs32[s32+2] = c32; ofs32[s32+3] = c32;
				ofs32[s32*2+0] = c32; ofs32[s32*2+1] = c32; ofs32[s32*2+2] = c32; ofs32[s32*2+3] = c32;
				ofs32[s32*3+0] = c32; ofs32[s32*3+1] = c32; ofs32[s32*3+2] = c32; ofs32[s32*3+3] = c32;
			}
		} else {
			ofs16 = (uint16_t*)ofs;
			for (i=3; i>0; i--, numpix >>= 1, ofs16 += 4) {
				c16 = (numpix & 1) ? (color & 0xffff) : 0;
				ofs16[0] = c16; ofs16[1] = c16; ofs16[2] = c16; ofs16[3] = c16;
				ofs16[s16+0] = c16; ofs16[s16+1] = c16; ofs16[s16+2] = c16; ofs16[s16+3] = c16;
				ofs16[s16*2+0] = c16; ofs16[s16*2+1] = c16; ofs16[s16*2+2] = c16; ofs16[s16*2+3] = c16;
				ofs16[s16*3+0] = c16; ofs16[s16*3+1] = c16; ofs16[s16*3+2] = c16; ofs16[s16*3+3] = c16;
			}
		}
	}
}

//
//	Print Clock param
//
void print_clock(void) {
	char		datestr[20];
	uint32_t	x, fx, fw, color, white, red;

	if (bpp == 4)	{ white = 0x00FFFFFF; red = 0x00FF0000; }
	else		{ white = 0x0000FFFF; red = 0x0000F800; }

	read_clock();
	sprintf(datestr, "%04u/%02u/%02u %02u:%02u:%02u",
		clk.tm_year+1900, clk.tm_mon+1, clk.tm_mday, clk.tm_hour, clk.tm_min, clk.tm_sec);
	fx = focusC * 3 + ((focusC != YEAR) ? 2 : 0);
	fw = (focusC == YEAR) ? 4 : 2;
	for (x=0; x<19; x++) {
		color = ((x >= fx) && (x < fx+fw)) ? red : white;
		print_num(datestr[x], x, color);
	}
}

//
//	Update Clock thread
//
static void* update_clock_thread(void* param) {
	struct timeval tod;

	while(1) {
		pthread_mutex_lock(&clock_mx);
		print_clock();
		pthread_mutex_unlock(&clock_mx);
		gettimeofday(&tod, NULL);
		usleep(1000000 - tod.tv_usec);
	}
	return 0;
}

//
//	Get day max
//
int get_day_max(void) {
	int y;
	switch (clk.tm_mon + 1) {
		case 2:
			y = clk.tm_year + 1900;
			if ( ((y % 4 == 0) && (y % 100 != 0)) || (y % 400 == 0)) return 29;
			return 28;
		case 4:
		case 6:
		case 9:
		case 11:
			return 30;
		default:
			return 31;
	}
}

//
//	Set Clock param
//
void set_clock_focus(int add) {
	static int	*cofs[6] = { &clk.tm_year, &clk.tm_mon, &clk.tm_mday, &clk.tm_hour, &clk.tm_min, &clk.tm_sec };
	const int	cmin[6] = {  70,  0,  1,  0,  0,  0 };
	const int	cmax[6] = { 137, 11, 31, 23, 59, 59 };
	int		*ofs = cofs[focusC];
	int		min = cmin[focusC];
	int		max = (focusC == DAY) ? get_day_max() : cmax[focusC];

	pthread_mutex_lock(&clock_mx);
	*ofs += add;
	while (*ofs < min) *ofs += (max - min + 1);
	while (*ofs > max) *ofs -= (max - min + 1);
	if ( ((focusC == YEAR) || (focusC == MON)) && (clk.tm_mday > 28) ) {
		max = get_day_max();
		if (clk.tm_mday > max) clk.tm_mday = max;
	}
	write_clock();
	pthread_mutex_unlock(&clock_mx);
}

//
//	Write LCD param
//
#define MI_DISP_SETLCDPARAM 0x401c6927
void write_lcd(void) {
	if (!disp_fd) disp_fd = open("/dev/mi_disp", O_WRONLY);
	MI_DISP_LcdParam LcdParam;
	// set LcdParam
	LcdParam.dev = 0;
	LcdParam.eCscMatrix = 3;
	LcdParam.u32Luma = lcd.Luma + 35;
	LcdParam.u32Hue = lcd.Hue * 5;
	LcdParam.u32Saturation = lcd.Saturation * 5;
	LcdParam.u32Contrast = (lcd.Contrast + 15) * 2;
	LcdParam.u32Sharpness = 0;
	uint64_t buf[] = {sizeof(LcdParam), (uintptr_t)&LcdParam};
	ioctl(disp_fd, MI_DISP_SETLCDPARAM, buf);
}

//
//	Print LCD param
//
void print_lcd(void) {
	char		lcdstr[20];
	uint32_t	x, fx, fw, color, white, red;

	if (bpp == 4)	{ white = 0x00FFFFFF; red = 0x00FF0000; }
	else		{ white = 0x0000FFFF; red = 0x0000F800; }

	sprintf(lcdstr, "L:%02u H:%02u S:%02u C:%02u",
		lcd.Luma, lcd.Hue, lcd.Saturation, lcd.Contrast);
	fx = focusL * 5 + 2; fw = 2;
	for (x=0; x<19; x++) {
		color = ((x >= fx) && (x < fx+fw)) ? red : white;
		print_num(lcdstr[x], x, color);
	}
}

//
//	Set LCD param
//
void set_lcd_focus(int add) {
	static uint32_t *lofs[4] = { &lcd.Luma, &lcd.Hue, &lcd.Saturation, &lcd.Contrast };
	int *ofs = (int*)lofs[focusL];

	*ofs += add;
	while (*ofs < 0) *ofs += 21;
	while (*ofs > 20) *ofs -= 21;
	write_lcd();
}

//
//	Save/Clear Display area
//
void save_displayarea(void) {
	stride = finfo.line_length;
	ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
	bpp = vinfo.bits_per_pixel / 8;	// byte per pixel
	fbofs = (uint8_t*)fb_addr + ( vinfo.yoffset * stride );

	// Save display area and clear
	if ((savebuf = malloc(DISPLAY_WIDTH * bpp * DISPLAY_HEIGHT))) {
		uint32_t i, ofss, ofsd;
		ofss = ofsd = 0;
		for (i=DISPLAY_HEIGHT; i>0; i--, ofss += stride, ofsd += DISPLAY_WIDTH * bpp) {
			memcpy(savebuf + ofsd, fbofs + ofss, DISPLAY_WIDTH * bpp);
			memset(fbofs + ofss, 0, DISPLAY_WIDTH * bpp);
		}
	}
}

//
//	Restore Display area
//
void restore_displayarea(void) {
	// Restore display area
	if (savebuf) {
		uint32_t i, ofss, ofsd;
		ofss = ofsd = 0;
		for (i=DISPLAY_HEIGHT; i>0; i--, ofsd += stride, ofss += DISPLAY_WIDTH * bpp) {
			memcpy(fbofs + ofsd, savebuf + ofss, DISPLAY_WIDTH * bpp);
		}
		free(savebuf); savebuf = NULL;
	}
}

//
//	Adjust Clock interface
//
void adjust_clock(int timeout) {
	save_displayarea();

	// Start updating clock
	clock_mx = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
	pthread_create(&clock_pt, NULL, update_clock_thread, NULL);
	clkthread_active = 1;

	// Adjust clock
	uint32_t finish = 0;
	uint32_t dirty = 0;
	do {
		int ready = poll(fds, 1, timeout);
		if (ready > 0) {
			read(input_fd, &ev, sizeof(ev));
			if ( (ev.type != EV_KEY) || (ev.value > REPEAT) ) continue;
			if ( ev.value != RELEASED ) {
				switch (ev.code) {
					case BUTTON_LEFT:
					case BUTTON_L1:
						focusC = (focusC != YEAR) ? focusC-1 : SEC;
						break;
					case BUTTON_RIGHT:
					case BUTTON_R1:
						focusC = (focusC != SEC) ? focusC+1 : YEAR;
						break;
					case BUTTON_UP:
						set_clock_focus(-1); dirty = 1;
						break;
					case BUTTON_DOWN:
						set_clock_focus(+1); dirty = 1;
						break;
					case BUTTON_L2:
						set_clock_focus(-4); dirty = 1;
						break;
					case BUTTON_R2:
						set_clock_focus(+4); dirty = 1;
						break;
					default:
						continue;
				}
				pthread_mutex_lock(&clock_mx);
				print_clock();
				pthread_mutex_unlock(&clock_mx);
			} else {
				switch (ev.code) {
					case BUTTON_MENU:
						short_pulse();
						// screenshot
						short_pulse();
						setCPUsave(1);
						screenshot();
						setCPUsave(0);
						setBrightness(GetKeyShm(&shminfo, MONITOR_BRIGHTNESS));
						
						break;
					case BUTTON_A:
					case BUTTON_B:
					case BUTTON_X:
					case BUTTON_Y:
					case BUTTON_SELECT:
					case BUTTON_POWER:
					case BUTTON_START:
						finish = 1;
						break;
					default:
						break;
				}
			}
		} else if ((!ready)&&(GetADCValue() != 100)) {
			// shutdown
			setCPUsave(1); resume(); usleep(100000); shutdown();
		}
	} while(!finish);

	// Stop updating clock
	pthread_cancel(clock_pt);
	pthread_join(clock_pt, NULL);
	clkthread_active = 0;

	restore_displayarea();

	// update clockd.txt
	read_clock();
	if (dirty) write_clock_rtc();
	write_clockfile();
}

//
//	Adjust LCD interface
//
void adjust_lcd(int timeout) {
	save_displayarea();

	// get LcdParam
	lcd.Luma = GetKeyShm(&shminfo, MONITOR_LUMINATION);
	lcd.Hue = GetKeyShm(&shminfo, MONITOR_HUE);
	lcd.Saturation = GetKeyShm(&shminfo, MONITOR_SATURATION);
	lcd.Contrast = GetKeyShm(&shminfo, MONITOR_CONTRAST);
	print_lcd();

	// Adjust LCD
	uint32_t finish = 0;
	uint32_t dirty = 0;
	do {
		int ready = poll(fds, 1, timeout);
		if (ready > 0) {
			read(input_fd, &ev, sizeof(ev));
			if ( (ev.type != EV_KEY) || (ev.value > REPEAT) ) continue;
			if ( ev.value != RELEASED ) {
				switch (ev.code) {
					case BUTTON_LEFT:
					case BUTTON_L1:
						focusL = (focusL != LUM) ? focusL-1 : CON;
						break;
					case BUTTON_RIGHT:
					case BUTTON_R1:
						focusL = (focusL != CON) ? focusL+1 : LUM;
						break;
					case BUTTON_UP:
					case BUTTON_L2:
						set_lcd_focus(-1); dirty = 1;
						break;
					case BUTTON_DOWN:
					case BUTTON_R2:
						set_lcd_focus(+1); dirty = 1;
						break;
					default:
						continue;
				}
				print_lcd();
			} else {
				switch (ev.code) {
					case BUTTON_MENU:
						// screenshot
						short_pulse();
						setCPUsave(1);
						screenshot();
						setCPUsave(0);
						setBrightness(GetKeyShm(&shminfo, MONITOR_BRIGHTNESS));
						
						break;
					case BUTTON_A:
					case BUTTON_B:
					case BUTTON_X:
					case BUTTON_Y:
					case BUTTON_SELECT:
					case BUTTON_POWER:
					case BUTTON_START:
						finish = 1;
						break;
					default:
						break;
				}
			}
		} else if ((!ready)&&(GetADCValue() != 100)) {
			// shutdown
			setCPUsave(1); resume(); usleep(100000); shutdown();
		}
	} while(!finish);

	restore_displayarea();

	if (dirty) {
		SetKeyShm(&shminfo, MONITOR_LUMINATION, lcd.Luma);
		SetKeyShm(&shminfo, MONITOR_HUE, lcd.Hue);
		SetKeyShm(&shminfo, MONITOR_SATURATION, lcd.Saturation);
		SetKeyShm(&shminfo, MONITOR_CONTRAST, lcd.Contrast);
		readJson();
		setValue(JSON_LUMINATION, lcd.Luma);
		setValue(JSON_HUE, lcd.Hue);
		setValue(JSON_SATURATION, lcd.Saturation);
		setValue(JSON_CONTRAST, lcd.Contrast);
		writeJson();
	}
}




//
//	[onion] suspend/resume PlayActivity timer
// 
void onion_pa_suspend(int mode) {
	
		if (mode) {
			// The current time is resumed
		//	logMessage("a");
			chdir("cd /mnt/SDCARD/.tmp_update/");
			system("./loadTime.sh; sync");
			//system("cd /mnt/SDCARD/.tmp_update/; ./loadTime.sh; sync");
			
		} else {
		//logMessage("b");
			// The current time is saved
			chdir("cd /mnt/SDCARD/.tmp_update/");
			system("./saveTime.sh; sync");
			
			//system("cd /mnt/SDCARD/.tmp_update/; ./saveTime.sh; sync");
		}
	
}

//
//	Suspend interface
//
void suspend_exec(int timeout) {
	// stop input event for other processes
	while (ioctl(input_fd, EVIOCGRAB, 1) < 0) { usleep(100000); }

	// suspend
	if (adcthread_active) {
		pthread_cancel(adc_pt); pthread_join(adc_pt, NULL);
		drawFrame(0); // erase red frame
		adcthread_active = 0;
	}
	if (onion) onion_pa_suspend(0);
	suspend(0);
	rumble(0);
	int recent_volume = setVolumeRaw(-60,0);
	setBrightnessRaw(0);
	setScreen(0);
	setCPUsave(0);

	uint32_t repeat_power = 0;
	uint32_t killexit = 0;

	while(1) {
		int ready = poll(fds, 1, timeout);
		if (ready > 0) {
			read(input_fd, &ev, sizeof(ev));
			if (( ev.type != EV_KEY ) || ( ev.value > REPEAT )) continue;
			if ( ev.code == BUTTON_POWER ) {
				if ( ev.value == RELEASED ) break;
				else if ( ev.value == PRESSED ) repeat_power = 0;
				else if ( ev.value == REPEAT ) {
					if ( ++repeat_power >= REPEAT_SEC(5) ) {
						killexit = 1; break;
					}
				}
			} else if ( ev.value == RELEASED ) {
				if ( ev.code == BUTTON_MENU ) {
					// screenshot
					short_pulse();
					setCPUsave(1);
					setScreen(1);
					screenshot();
					setScreen(0);
					setCPUsave(0);
					// break;   //  avoid bad screen state after the screen shot
					
				} else if ( ev.code == BUTTON_START )  {
					// adjust clock
					/*
					setScreen(1);
					setBrightness(GetKeyShm(&shminfo, MONITOR_BRIGHTNESS));
					adjust_clock(timeout);
					break;
					*/
					
				} else if ( ev.code == BUTTON_SELECT ) {
					// adjust LCD
					/*
					setScreen(1);
					setBrightness(GetKeyShm(&shminfo, MONITOR_BRIGHTNESS));
					adjust_lcd(timeout);
					break;
					*/
				}
			}
		} else if ((!ready)&&(GetADCValue() != 100)) {
			// shutdown
			setCPUsave(1); resume(); usleep(100000); shutdown();
		}
	}

	// resume
	setCPUsave(1);
	if (killexit) { resume(); usleep(100000); suspend(2); usleep(400000); }
	setScreen(1);
	setBrightness(GetKeyShm(&shminfo, MONITOR_BRIGHTNESS));
	setVolumeRaw(recent_volume, 0);
	if (!killexit) {
		resume();
		if (onion) onion_pa_suspend(1);
	}
	updateADC(1);

	// restart input event for other processes
	while (ioctl(input_fd, EVIOCGRAB, 0) < 0) { usleep(100000); }
}

//
//	Choose onion mode or stock mode
//
void check_onion(void) {
	FILE*	fp; 
	char	comm[32];

	// Check filename of myself
	snprintf(comm, 32, "/proc/%d/comm", getpid());
	if ((fp = fopen(comm, "r"))) {
		fscanf(fp, "%31s", comm);
		if (!strcmp(comm, "keymon_onion")) {
			// onion mode, check ADC to kill
			onion = 2;
		} 
		fclose(fp);
	}
}

//
//	[onion] Check retroarch running & savestate_auto_save in retroarch.cfg is true
//
int check_autosave(void) {
	int	f, ret = 0;
	pid_t	ra_pid;
	char	key[256] = {0};
	char	value[256] = {0};
	char	*keyptr, *valptr, *strptr;
	char	cfgpath[64];
	const char cfg_ext[] = ".cfg";

	char ra_name[12] = "retroarch";
	if ( (ra_pid = searchpid(ra_name)) ) {
		strptr = cfgpath + sprintf(cfgpath, "/proc/%d/cwd/", ra_pid);
		// standard retroarch ( ./.retroarch/retroarch.cfg )
		sprintf(strptr, ".retroarch/%s%s", ra_name, cfg_ext);
		if (access(cfgpath, R_OK)) {
			// stock retroarch ( ./retroarch.cfg )
			sprintf(strptr, "%s%s", ra_name, cfg_ext);
			if (access(cfgpath, R_OK)) return 0;
		}
	} else {
		strcpy(ra_name, "ra32");
		if ( (ra_pid = searchpid(ra_name)) ) {
			// stock ra32.ss ( ./ra32.cfg )
			sprintf(cfgpath, "/proc/%d/cwd/%s%s", ra_pid, ra_name, cfg_ext);
			if (access(cfgpath, R_OK)) return 0;
		} else return 0;
	}

	FILE* fp = fopen(cfgpath, "r");
	if (!fp) return 0;
	while ((f = fscanf(fp, "%255[^=]=%255[^\n]\n", key, value)) != EOF) {
		if (!f) { if (fscanf(fp, "%*[^\n]\n") == EOF) break; else continue; }
		if ((keyptr = trimstr(key, 0))) {
			if (!strcmp(keyptr, "savestate_auto_save")) {
				if ((valptr = trimstr(value, 1))) {
					if (!strcmp(valptr, "\"true\"")) ret = 1;
				}
				break;
			}
		}
	}
	fclose(fp);
	return ret;
}

//
//	[onion] Write percBat to /tmp/percBat
//
void write_percBat(void) {
	FILE*	fp;
	int	percBat;

	if (adcvalue == 100) percBat = 500;
	else if (adcvalue >= 578) percBat = 100;
	else if (adcvalue >= 528) percBat = adcvalue - 478;
	else if (adcvalue >= 512) percBat = (int)(adcvalue * 2.125 - 1068);
	else if (adcvalue >= 480) percBat = (int)(adcvalue * 0.51613 - 243.742);
	else percBat = 0;


	if (percBat <= 4) {
		if (check_autosave()) terminate_retroarch();
	}


	if ( (fp = fopen("/tmp/percBat", "w")) ) {
		fprintf(fp, "%d", percBat);
		fclose(fp);
	}
}

//
//	[onion] write /mnt/SDCARD/.tmp_update/.offOrder
//
void write_offorder(void) {
	close(creat("/mnt/SDCARD/.tmp_update/.offOrder", 777));
	read_clock();
	write_clockfile();
	sync();
}

//
//	[onion] deepsleep if MainUI/onionLauncher/retroarch is running
//
void deepsleep(void) {

	pid_t pid;
	if ((pid = searchpid("MainUI"))) {
		short_pulse();
		write_offorder(); kill(pid, SIGKILL); 
		
	} else if ((pid = searchpid("onionLauncher"))) {
		short_pulse();
		write_offorder(); kill(pid, SIGKILL);
		
	} else if ((pid = searchpid("retroarch")))
	 {
	 	if (check_autosave()){
			short_pulse();
			write_offorder(); 
			terminate_retroarch(); 
			
	 	}
	 }
}


void logMessage(char* Message) {
	FILE *file = fopen("/mnt/SDCARD/.tmp_update/log_keymon.txt", "a");
    char valLog[200];
    sprintf(valLog, "%s %s", Message, "\n");
    fputs(valLog, file);
	fclose(file); 
}

//
//	Main
//
int main(void) {
	// Initialize
	signal(SIGTERM, quit);
	signal(SIGSEGV, quit);
	check_onion();
	// for onion, do not init clock because it adjusts clock right after brightnessCont is executed
	if (!onion) init_clock();
	initJson();

	// Set Initial Volume / Brightness
	setVolumeRaw(0,0);
	setBrightness(getValue(JSON_BRIGHTNESS));

	// Open and mmap FB
	fb_fd = open("/dev/fb0", O_RDWR);
	ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
	fb_addr = mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);

	// Prepare for Poll button input
	input_fd = open("/dev/input/event0", O_RDONLY);
	memset(&fds, 0, sizeof(fds));
	fds[0].fd = input_fd;
	fds[0].events = POLLIN;

	// Main Loop
	uint32_t button_flag = 0;
	uint32_t repeat_LR = 0;
	uint32_t repeat_power = 0;
	uint32_t repeat_menu = 0;
	uint32_t val;
	uint32_t b_BTN_Not_Menu_Pressed = 0;
	uint32_t b_BTN_Menu_Pressed = 0;
	uint32_t comboKey = 0;
	
	struct timespec recent;
	clock_gettime(CLOCK_MONOTONIC_COARSE, &recent);
	struct timespec hibernate_start = recent;
	int hibernate_time;
	int elapsed_sec = 0;

	// The next calls are in 15s
	// Update ADC Value
	updateADC(0);
	if (onion) write_percBat();
	// Update recent time
	clock_gettime(CLOCK_MONOTONIC_COARSE, &recent);	
	

	while(1) {
		if (poll(fds, 1, (CHECK_SEC - elapsed_sec) * 1000) > 0) {
			
			read(input_fd, &ev, sizeof(ev));
			val = ev.value;
			
			if (( ev.type != EV_KEY ) || ( val > REPEAT )) continue;
			
			
			if ((ev.code == BUTTON_POWER)&&(val == 1)){
					super_short_pulse();			
			}
			
			if (val!=REPEAT){

				
				
				if (ev.code == BUTTON_MENU){
					b_BTN_Menu_Pressed = val;
					if (val == 1){
						super_short_pulse();
					}
				}

				else {
					b_BTN_Not_Menu_Pressed = val;
				}
					
	
				if ((b_BTN_Menu_Pressed == 1)&&(b_BTN_Not_Menu_Pressed == 1)){
					comboKey = 1;
				}
				
				if ((b_BTN_Menu_Pressed == 1)&&(ev.code == BUTTON_POWER)){
					// screenshot
					
				}
				
				if ((ev.code == BUTTON_MENU)&&(val == 0)){	
					
					if (comboKey == 0){
						if (access("/mnt/SDCARD/.tmp_update/.menuInverted", F_OK)==-1) {
								if (check_autosave()) {
									if (access("/mnt/SDCARD/.tmp_update/.noGameSwitcher", F_OK)==-1) {  // just in case if someone want to desactivate Game Switcher totally
										if (onion) {
										//super_short_pulse();
										close(creat("/tmp/.trimUIMenu", 777));
										screenshot_onion(); 
										terminate_retroarch();	
										}
									}
								}
						}
						else  // The file ".menuInverted" exists, we exit retroarch on short press
						{
								
								if (check_autosave()) {
									remove("/tmp/.trimUIMenu");
									terminate_retroarch();
								}
						}
					}
					else{
					comboKey = 0;
					} 
					
				}	
				
			}
			
			
			

			switch (ev.code) {
			case BUTTON_POWER:
				if ( val == REPEAT ) {
					repeat_power++;
					if ((onion)&&(repeat_power == 7)) {
					deepsleep(); // 0.5sec deepsleep(onion)
				}
					 
					else if (repeat_power == REPEAT_SEC(5)) {
						short_pulse();
						suspend(2);	   // 5sec kill processes
						
					}
					else if (repeat_power >= REPEAT_SEC(10)) {
						short_pulse();
						shutdown();	   // 10sec shutdown
					}
					break;
				}
				if (( val == RELEASED )&&(repeat_power < 7)&&(access("/tmp/stay_awake", F_OK))) {
					// suspend
					hibernate_time = GetKeyShm(&shminfo, MONITOR_HIBERNATE_DELAY);
					suspend_exec( (hibernate_time == 0) ? -1 : (hibernate_time + SHUTDOWN_MIN) * 60000 );
				}
				repeat_power = 0;
				break;
			case BUTTON_SELECT:
				if ( val != REPEAT ) {
					button_flag = (button_flag & (~SELECT)) | (val<<SELECT_BIT);
				}
				break;
			case BUTTON_START:
				if ( val != REPEAT ) {
					button_flag = (button_flag & (~START)) | (val<<START_BIT);
				}
				break;
			case BUTTON_L2:
				if ( val == REPEAT ) {
					// Adjust repeat speed to 1/2
					val = repeat_LR;
					repeat_LR ^= PRESSED;
				} else {
					button_flag = (button_flag & (~L2)) | (val<<L2_BIT);
					repeat_LR = 0;
				}
				if ( val == PRESSED ) {
					switch (button_flag & (SELECT|START)) {
					case START:
						// SELECT + L2 : volume down / + R2 : reset
						setVolumeRaw(0, (button_flag & R2) ? 0 : -3);
						break;
					case SELECT:
						// START + L2 : brightness down
						readJson();
						val = getValue(JSON_BRIGHTNESS);
						if (val>0) {
							setBrightness(--val);
							writeJson();
						}
						break;
					default:
						break;
					}
				}
				break;
			case BUTTON_R2:
				if ( val == REPEAT ) {
					// Adjust repeat speed to 1/2
					val = repeat_LR;
					repeat_LR ^= PRESSED;
				} else {
					button_flag = (button_flag & (~R2)) | (val<<R2_BIT);
					repeat_LR = 0;
				}
				if ( val == PRESSED ) {
					switch (button_flag & (SELECT|START)) {
					case START:
						// SELECT + R2 : volume up / + L2 : reset
						setVolumeRaw(0, (button_flag & L2) ? 0 : +3);
						break;
					case SELECT:
						// START + R2 : brightness up
						readJson();
						val = getValue(JSON_BRIGHTNESS);
						if (val<BRIMAX) {
							setBrightness(++val);
							writeJson();
						}
						break;
					default:
						break;
					}
				} 
				break;
			case BUTTON_MENU:
				if (onion) {
					if ( val == REPEAT ) {
						repeat_menu++;
						if ((repeat_menu == REPEAT_SEC(1))&&(!button_flag)) {    // long press on menu
							
								// The file does not exists
							if (access("/mnt/SDCARD/.tmp_update/.menuInverted", F_OK)==-1) {
									if (check_autosave()) {
										remove("/tmp/.trimUIMenu");
										short_pulse();
										terminate_retroarch();
									}
							}
							else  // The file ".menuInverted" exists, we show menu on long press
							{
								if (access("/mnt/SDCARD/.tmp_update/.noGameSwitcher", F_OK)==-1) {  // just in case if someone want to desactivate Game Switcher totally
									if (check_autosave()) {
										short_pulse();
										close(creat("/tmp/.trimUIMenu", 777));
										screenshot_onion();
										terminate_retroarch();	
									}
								}
								else   // if .menuInverted and .noGameSwitcher exist then long press does nothing
								{
									comboKey = 1;  
								}
							}
						
							repeat_menu = 0;
							

						}
						
						}
										
				}
				break;
			default:
				break;
			}
			


			clock_gettime(CLOCK_MONOTONIC_COARSE, &hibernate_start);
			elapsed_sec = hibernate_start.tv_sec - recent.tv_sec;
			if ( elapsed_sec < CHECK_SEC ) continue;
		}

		// Comes here every CHECK_SEC(def:15) seconds interval

		// Check Hibernate
		if (!access("/tmp/battery_charging", F_OK)) hibernate_time = 1;
		else hibernate_time = GetKeyShm(&shminfo, MONITOR_HIBERNATE_DELAY);
		if ((hibernate_time)&&(access("/tmp/stay_awake", F_OK))) {
			clock_gettime(CLOCK_MONOTONIC_COARSE, &recent);
			if ( (recent.tv_sec - hibernate_start.tv_sec) >= (hibernate_time * 60) ) {
				suspend_exec(SHUTDOWN_MIN * 60000);
				clock_gettime(CLOCK_MONOTONIC_COARSE, &hibernate_start);
			}
		}
		// Update ADC Value
		updateADC(0);
		if (onion) write_percBat();
		// Update recent time
		clock_gettime(CLOCK_MONOTONIC_COARSE, &recent);
		elapsed_sec = 0;
		
	}
}

