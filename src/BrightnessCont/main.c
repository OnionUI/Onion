
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>  
#include "sys/ioctl.h"
#include <signal.h>
#include <dirent.h>
#include <signal.h>
#include "cJSON.h"

//	for ev.value
#define RELEASED	0
#define PRESSED		1
#define REPEAT		2

//	Global Variables
#define		PIDMAX	32
uint32_t	suspendpid[PIDMAX];

typedef enum {
	MONITOR_VOLUME,			// vol
	MONITOR_BRIGHTNESS,		// brightness
	MONITOR_KEYMAP,			// keymap (maybe unused)
	MONITOR_MUTE,			// mute
	MONITOR_VOLUME_CHANGED,		// volume change (internal use)
	MONITOR_BGM_VOLUME,		// bgmvol
	MONITOR_HIBERNATE_DELAY,	// hibernate
	MONITOR_ADC_VALUE,		// charging state (internal use)
	MONITOR_LUMINATION,		// lumination
	MONITOR_HUE,			// hue
	MONITOR_SATURATION,		// saturation
	MONITOR_CONTRAST,		// contrast
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

bool file_exists (char *filename) {
  struct stat   buffer;   
  return (stat (filename, &buffer) == 0);
}

void logMessage(char* Message) {
	FILE *file = fopen("/mnt/SDCARD/.tmp_update/log_BrightnessCont.txt", "a");
	/*char tempMess[] = "\r\n";
    strcat(Message,tempMess);
    */
    char valLog[200];
    sprintf(valLog, "%s %s", Message, "\n");
    fputs(valLog, file);
	fclose(file); 
}

//
//	Suspend 
//
void suspend(void) {
	DIR *procdp;
	struct dirent *dir;
	char fname[32];
	pid_t suspend_pid = getpid();
	pid_t parent_pid = getppid();
	pid_t pid;
	pid_t ppid;
	char state;
	char comm[128];

	// Send SIGSTOP to active processes
	// Cond:1. PID is greater than suspend's
	// 	2. PPID is greater than or equal to suspend's
	//	3. state is "R" or "S" or "D"
	//	4. comm is not "(sh)" or "(kworker/.." "(mmcqd/.." "(bioset)"
	suspendpid[0] = 0;
	procdp = opendir("/proc");
	while ((dir = readdir(procdp))) {
		if (dir->d_type == DT_DIR) {
			pid = atoi(dir->d_name);
			if ( pid > suspend_pid) {
				sprintf(fname, "/proc/%d/stat", pid);
				FILE *fd = fopen(fname, "r");
				if (fd != NULL) {
					fscanf(fd, "%*d %128s %c %d", (char*)&comm, &state, &ppid);
					fclose(fd);
				}
				//fprintf(stdout, "pid: %d ppid:%d parent:%d state:%c comm:%s\n",pid,ppid,parent_pid,state,comm);
				if ((ppid >= parent_pid)&&((state == 'R')||(state == 'S')||(state == 'D'))) {
					if ( (strcmp(comm,"(sh)")) && (strncmp(comm,"(kworker/",9)) &&
					     (strncmp(comm,"(mmcqd/",7)) && (strcmp(comm,"(bioset)")) ) {
						if ( suspendpid[0] < PIDMAX ) {
							suspendpid[++suspendpid[0]] = pid;
							kill(pid,SIGSTOP);		
							
						}
					}
				}
			}
		}
	}
	closedir(procdp);
}

void killAll(void) {
	DIR *procdp;
	struct dirent *dir;
	char fname[32];
	pid_t suspend_pid = getpid();
	pid_t parent_pid = getppid();
	pid_t pid;
	pid_t ppid;
	char state;
	char comm[128];
	

	// Send SIGSTOP to active processes
	// Cond:1. PID is greater than suspend's
	// 	2. PPID is greater than or equal to suspend's
	//	3. state is "R" or "S" or "D"
	//	4. comm is not "(sh)" or "(kworker/.." "(mmcqd/.." "(bioset)"
	suspendpid[0] = 0;
	procdp = opendir("/proc");
	while ((dir = readdir(procdp))) {
		if (dir->d_type == DT_DIR) {
			pid = atoi(dir->d_name);
			if ( pid > suspend_pid) {
				sprintf(fname, "/proc/%d/stat", pid);
				FILE *fd = fopen(fname, "r");
				if (fd != NULL) {
					fscanf(fd, "%*d %128s %c %d", (char*)&comm, &state, &ppid);
					fclose(fd);
				}
				//fprintf(stdout, "pid: %d ppid:%d parent:%d state:%c comm:%s\n",pid,ppid,parent_pid,state,comm);
				if ((ppid >= parent_pid)&&((state == 'R')||(state == 'S')||(state == 'D'))) {
					if ( (strcmp(comm,"(sh)")) && (strncmp(comm,"(kworker/",9)) &&
					     (strncmp(comm,"(mmcqd/",7)) && (strcmp(comm,"(bioset)")) ) {
						if ( suspendpid[0] < PIDMAX ) {
							suspendpid[++suspendpid[0]] = pid;
							kill(pid,SIGKILL);
						}
					}
				}
			}
		}
	}
	closedir(procdp);
}

int sigTermAll(void) {
	DIR *procdp;
	struct dirent *dir;
	char fname[32];
	pid_t suspend_pid = getpid();
	pid_t parent_pid = getppid();
	pid_t pid;
	pid_t ppid;
	char state;
	char comm[128];
	int nfound = 0;

	// Send SIGSTOP to active processes
	// Cond:1. PID is greater than suspend's
	// 	2. PPID is greater than or equal to suspend's
	//	3. state is "R" or "S" or "D"
	//	4. comm is not "(sh)" or "(kworker/.." "(mmcqd/.." "(bioset)"
	suspendpid[0] = 0;
	procdp = opendir("/proc");
	while ((dir = readdir(procdp))) {
		if (dir->d_type == DT_DIR) {
			pid = atoi(dir->d_name);
			if ( pid > suspend_pid) {
				sprintf(fname, "/proc/%d/stat", pid);
				FILE *fd = fopen(fname, "r");
				if (fd != NULL) {
					fscanf(fd, "%*d %128s %c %d", (char*)&comm, &state, &ppid);
					fclose(fd);
				}
				//fprintf(stdout, "pid: %d ppid:%d parent:%d state:%c comm:%s\n",pid,ppid,parent_pid,state,comm);
				if ((ppid >= parent_pid)&&((state == 'R')||(state == 'S')||(state == 'D'))) {
					if ( (strcmp(comm,"(sh)")) && (strncmp(comm,"(kworker/",9)) &&
					     (strncmp(comm,"(mmcqd/",7)) && (strcmp(comm,"(bioset)")) ) {
						if ( suspendpid[0] < PIDMAX ) {
							suspendpid[++suspendpid[0]] = pid;
							if (kill(pid,SIGTERM)==0){
								nfound = 1;
							}
						}
					}
				}
			}
		}
	}
	closedir(procdp);
	return nfound;
}



void resume(void) {
	// Send SIGCONT to suspended processes
	if (suspendpid[0]) {
		for (uint32_t i=1; i <= suspendpid[0]; i++) {
			kill(suspendpid[i],SIGCONT);
		}
	}
} 
void screenshot(void) {
	const uint8_t bmpheader[] = {		// 640x480 RGB888 32bit BMP Header
		0x42,0x4D,0x36,0xC0,0x12,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x00,0x28,0x00,
		0x00,0x00,0x80,0x02,0x00,0x00,0xE0,0x01,0x00,0x00,0x01,0x00,0x20,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00,0x12,0x0B,0x00,0x00,0x12,0x0B,0x00,0x00,0x00,0x00,
		0x00,0x00,0x00,0x00,0x00,0x00 };
	remove("screenshotGame.bmp");
	char		screenshotname[] = "screenshotGame.bmp";
	uint32_t	linebuffer[640], x, y, pix1, pix2;
	int		fd, fd_fb;
	struct		fb_var_screeninfo vinfo;
	
	fd_fb = open("/dev/fb0", O_RDWR);
	ioctl(fd_fb, FBIOGET_VSCREENINFO, &vinfo);

	fd = creat(screenshotname, 777);
	if (fd > 0) {
		write(fd,bmpheader,sizeof(bmpheader));
		lseek(fd_fb,640*vinfo.yoffset*4,SEEK_SET);
		for (y=480; y>0; y--) {
			read(fd_fb,linebuffer,sizeof(linebuffer));
			for (x=320; x>0; x--){
				pix1 = linebuffer[320-x] | 0xFF000000;
				pix2 = linebuffer[320+x-1]  | 0xFF000000;
				linebuffer[320+x-1] = pix1;
				linebuffer[320-x] = pix2;
			}
			write(fd,linebuffer,sizeof(linebuffer));
		}
		close(fd);
		sync();
	}
	close(fd_fb);
}

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

void SetRawBrightness(int val) {  // val = 0-100
    int fd = open("/sys/class/pwm/pwmchip0/pwm0/duty_cycle", O_WRONLY);
    if (fd>=0) {
        dprintf(fd,"%d",val);
        close(fd); 
    }
}

void SetBrightness(int value) {  // value = 0-10
    KeyShmInfo	info;
    InitKeyShm(&info);
	SetKeyShm(&info, MONITOR_BRIGHTNESS, value);
	UninitKeyShm(&info);
    SetRawBrightness(value==0?6:value*10);
}

void turnScreenOff(void) {  
    SetRawBrightness(0);
}

char* load_file(char const* path)
{
    char* buffer = 0;
    long length;
    FILE * f = fopen (path, "rb"); //was "rb"

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


void setMiyooLum(int nLum){

	cJSON* request_json = NULL;
	cJSON* itemBrightness;

	char sBrightness[20]; 
	
	const char *request_body = load_file("/appconfigs/system.json");
	request_json = cJSON_Parse(request_body);
	itemBrightness = cJSON_GetObjectItem(request_json, "brightness");

	int dBrightness = cJSON_GetNumberValue(itemBrightness);
	sprintf(sBrightness, "%d", dBrightness);
	
	
	cJSON_SetNumberValue(itemBrightness, nLum);

	FILE *file = fopen("/appconfigs/system.json", "w");	
	char *test = cJSON_Print(request_json);	
	fputs(test, file);
	fclose(file); 	
}



#define	BUTTON_A	KEY_SPACE
#define	BUTTON_B	KEY_LEFTCTRL  

#define	BUTTON_X	KEY_LEFTSHIFT
#define	BUTTON_Y	KEY_LEFTALT  

#define	BUTTON_START	KEY_ENTER
#define	BUTTON_SELECT	KEY_RIGHTCTRL

#define	BUTTON_MENU	KEY_ESC
#define	BUTTON_POWER	KEY_POWER

#define	BUTTON_L2	KEY_TAB
#define	BUTTON_R2	KEY_BACKSPACE

#define	BUTTON_L1	KEY_E
#define	BUTTON_R1	KEY_T

#define	BUTTON_UP	KEY_UP
#define	BUTTON_DOWN	KEY_DOWN

#define	BUTTON_LEFT	KEY_LEFT
#define	BUTTON_RIGHT KEY_RIGHT

int main() {
	int			input_fd;
	struct input_event	ev;
	uint32_t		val;
	uint32_t		l2_pressed = 0;
	uint32_t		r2_pressed = 0;
	uint32_t		l1_pressed = 0;
	uint32_t		r1_pressed = 0;
	uint32_t		menu_pressed = 0;
	uint32_t		a_pressed = 0;
	uint32_t		b_pressed = 0;
	uint32_t		up_pressed = 0;
	uint32_t		down_pressed = 0;
	uint32_t		left_pressed = 0;
	uint32_t		right_pressed = 0;
	uint32_t		x_pressed = 0;
	uint32_t		y_pressed = 0;
	uint32_t		start_pressed = 0;
	uint32_t		power_pressed = 0;
	uint32_t		select_pressed = 0;	
	
	
	// Safe boot exit called once
	int 			bSafeExitDone = 0;	
	// Prepare for Poll button input
	input_fd = open("/dev/input/event0", O_RDONLY);
	
	// Check current brightness value
	int brightness_value = 10;
	FILE *fp;
	long lSize;
	char *currLum;
	fp = fopen ( "/mnt/SDCARD/.tmp_update/brightSett" , "rb" );
	if( fp > 0 ) {
		fseek( fp , 0L , SEEK_END);
		lSize = ftell( fp );
		rewind( fp );
		currLum = (char*)calloc( 1, lSize+1 );
		if( !currLum ) fclose(fp),fputs("memory alloc fails",stderr),exit(1);
	
		if( 1!=fread( currLum , lSize, 1 , fp) )
  		fclose(fp),free(currLum),fputs("entire read fails",stderr),exit(1);
		fclose(fp);
	}
	
	sscanf(currLum, "%d", &brightness_value);

	//logMessage(currLum);
	//logMessage("----------");
	
	if (brightness_value > 10) {
		brightness_value = 8;
	}

	SetBrightness(brightness_value);
	
	int keyNotMenuPressed=0;
	int comboKey=0;
		
	
	while( read(input_fd, &ev, sizeof(ev)) == sizeof(ev) ) {
		
		val = ev.value;
			
		if (( ev.type != EV_KEY ) || ( val > 1 )) continue;

		if ( ev.code == BUTTON_UP ) {
				keyNotMenuPressed = val;
				up_pressed = val;
		}
		else
			if ( ev.code == BUTTON_DOWN ) {
					keyNotMenuPressed = val;
					down_pressed = val;
			}
			else
				if ( ev.code == BUTTON_LEFT ) {
						keyNotMenuPressed = val;
						left_pressed = val;
				}
			else
				if ( ev.code == BUTTON_RIGHT ) {
						keyNotMenuPressed = val;
						right_pressed = val;
				}	
			
			
			else								
				if ( ev.code == BUTTON_A ) {
						keyNotMenuPressed = val;
						a_pressed = val;
				}
				else
					if ( ev.code == BUTTON_B ) {
							keyNotMenuPressed = val;
							b_pressed = val;
					}		
				else
					if ( ev.code == BUTTON_X ) {
							keyNotMenuPressed = val;
							x_pressed = val;
					}	
					
				else
					if ( ev.code == BUTTON_Y ) {
							keyNotMenuPressed = val;
							y_pressed = val;
					}	
							
					else
					if ( ev.code == BUTTON_L2 ) {
							keyNotMenuPressed = val;
							l2_pressed = val;
					}
					else
						if ( ev.code == BUTTON_R2 ) {
								keyNotMenuPressed = val;
								r2_pressed = val;
						}
					if ( ev.code == BUTTON_L1 ) {
							keyNotMenuPressed = val;
							l1_pressed = val;
					}
					else
						if ( ev.code == BUTTON_R1 ) {
								keyNotMenuPressed = val;
								r1_pressed = val;
						}
						else
							if ( ev.code == BUTTON_START ) {
									keyNotMenuPressed = val;
									start_pressed = val;
							}
							else
								if ( ev.code == BUTTON_SELECT ) {
										keyNotMenuPressed = val;
										select_pressed = val;
								}
								else	
									if ( ev.code == BUTTON_MENU ) {
										menu_pressed = val;
									}
									else
										if ( ev.code == BUTTON_POWER ) {
												keyNotMenuPressed = val;
												power_pressed = val;
										}	
		

	
		
		if ((menu_pressed == 1)&&(keyNotMenuPressed == 1)){
		
			comboKey = 1;
		}
		
		if ((ev.code == BUTTON_MENU)&&(menu_pressed == 0)){
			if (comboKey == 0){
				 if (file_exists(".disableMenu")!=1){
				 	system("killall -15 retroarch");
					screenshot();		
				 }			
			}
			comboKey = 0;
		}

		
		if (start_pressed & select_pressed & a_pressed & b_pressed) {
			 if (file_exists(".altBrightShortcut")==1){
			 	remove(".altBrightShortcut");
			 }
			 else {
			 	int fd = creat(".altBrightShortcut", 777);
				close(fd); 
			 }
		}
		
		
		// Shorcuts handlers	
		// Panic mode 
		if (start_pressed & select_pressed & menu_pressed & r2_pressed & l2_pressed) {
			 system("rm /mnt/SDCARD/App/OnionLauncher/data/.enabled"); 
			 system("reboot");
			 sleep(10);
		} 
		
		// Force turn off
		if (start_pressed & select_pressed & r2_pressed & l2_pressed) {
			 system("reboot");
			 sleep(10);
		}
			 
		if (power_pressed && ! menu_pressed){
	
			//SetRawBrightness(0);
			int fd = creat(".offOrder", 777);
			close(fd); 
			 
			if (file_exists("/tmp/cmd_to_run_launcher.sh")==1){
				// One game is running
				system("sync");			
				system("killall -15 retroarch");
			}
			else{
				if (file_exists("/tmp/cmd_to_run.sh")==1){
					// One game/app is running
					system("sync");
					sigTermAll();	
				}
				else {
					
					system("sync");
					system("killall -9 MainUI");
					system("killall -9 onionLauncher");
					// Reset framebuffer
					
					
					system("sleep 5");
					killAll();
					system("sleep 10");
				}	
			}

		}
		 
		// Sleep mode
		if (power_pressed && menu_pressed) {
			
			// suspend
			turnScreenOff(); 
			// Timer registration
			system("cd /tmp/; value=$(cat romName.txt); cd /mnt/SDCARD/App/PlayActivity; ./playActivity \"$value\"");

			usleep(200000); 
			suspend(); 
			rumble(0);
			// Wait for Resleeplease button
			do { 
				if ( read(input_fd, &ev, sizeof(ev)) != sizeof(ev) ) {
					break;
				}
			} 
			while ((ev.type != EV_KEY) || (ev.code != BUTTON_POWER) || ( ev.value != 1 ));
			// resume
			resume(); 
			// Timer initialisation
			system("cd /mnt/SDCARD/App/PlayActivity; ./playActivity \"init\"");
			
			usleep(200000); 
			SetBrightness(brightness_value);
			menu_pressed = 0;
		}	
			
		// Brightness possible values
		// 0 1 2 4 6 8 10
		
		if (select_pressed & l2_pressed) {
		//	start_pressed = select_pressed = l2_pressed = 0;
			if(file_exists(".altBrightShortcut")==0){	
				if (brightness_value >= 1){
					brightness_value--;
					if (brightness_value > 2){
						brightness_value--;
					}	
					remove("brightSett");
					
					SetBrightness(brightness_value);	
					//setMiyooLum(brightness_value);
					fclose(fopen("/mnt/SDCARD/.tmp_update/brightSett","w"));
					int adc_fd = open("/mnt/SDCARD/.tmp_update/brightSett", O_CREAT | O_WRONLY);
					if (adc_fd>0) {
						char val[3];
						sprintf(val, "%d", brightness_value);
        				write(adc_fd, val, strlen(val));
        				close(adc_fd);
					}
				}
			}	
		}
		if (menu_pressed & down_pressed) {
		//	start_pressed = select_pressed = l2_pressed = 0;
			if(file_exists(".altBrightShortcut")==1){
			
				if (brightness_value >= 1){
					brightness_value--;
					if (brightness_value > 2){
						brightness_value--;
					}	
					remove("brightSett");
					
					SetBrightness(brightness_value);	
					//setMiyooLum(brightness_value);
					fclose(fopen("/mnt/SDCARD/.tmp_update/brightSett","w"));
					int adc_fd = open("/mnt/SDCARD/.tmp_update/brightSett", O_CREAT | O_WRONLY);
					if (adc_fd>0) {
						char val[3];
						sprintf(val, "%d", brightness_value);
        				write(adc_fd, val, strlen(val));
        				close(adc_fd);
					}
				}
			}
		}	

		if (select_pressed & r2_pressed) {
			if (file_exists(".altBrightShortcut")==0) {
				if (brightness_value <= 8){
					brightness_value++;
					if (brightness_value > 2){
						brightness_value++;
					}	
					remove("brightSett");
					
					SetBrightness(brightness_value);	
					//setMiyooLum(brightness_value);
					fclose(fopen("/mnt/SDCARD/.tmp_update/brightSett","w"));
					int adc_fd = open("/mnt/SDCARD/.tmp_update/brightSett", O_CREAT | O_WRONLY);
					if (adc_fd>0) {
						char val[3];
						sprintf(val, "%d", brightness_value);
        				write(adc_fd, val, strlen(val));
        				close(adc_fd);
					}	
				}
			}
		}	
		if (menu_pressed & up_pressed) {
		//	start_pressed = select_pressed = r2_pressed = 0;
			if (file_exists(".altBrightShortcut")==1){
				if (brightness_value <= 8){
					brightness_value++;
					if (brightness_value > 2){
						brightness_value++;
					}	
					remove("brightSett");
					
					SetBrightness(brightness_value);	
					//setMiyooLum(brightness_value);
					fclose(fopen("/mnt/SDCARD/.tmp_update/brightSett","w"));
					int adc_fd = open("/mnt/SDCARD/.tmp_update/brightSett", O_CREAT | O_WRONLY);
					if (adc_fd>0) {
						char val[3];
						sprintf(val, "%d", brightness_value);
        				write(adc_fd, val, strlen(val));
        				close(adc_fd);
					}	
				}
			
			}
		}					
			
	}
	
	close(input_fd);
	return 1;
}
