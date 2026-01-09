//
//	miyoomini over/underclocking tool
//
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "system/device_model.h"

#define BASE_REG_RIU_PA (0x1F000000)
#define BASE_REG_MPLL_PA (BASE_REG_RIU_PA + 0x103000 * 2)
#define PLL_SIZE (0x1000)

void *pll_map;

void print_clock(void)
{
    uint32_t rate;
    uint32_t lpf_value;
    uint32_t post_div;
    volatile uint8_t *p8 = (uint8_t *)pll_map;
    volatile uint16_t *p16 = (uint16_t *)pll_map;

    //get LPF / post_div
    lpf_value = p16[0x2A4] + (p16[0x2A6] << 16);
    post_div = p16[0x232] + 1;
    if (lpf_value == 0)
        lpf_value = (p8[0x2C2 << 1] << 16) + (p8[0x2C1 << 1] << 8) + p8[0x2C0 << 1];

    /*
	 * Calculate LPF value for DFS
	 * LPF_value(5.19) = (432MHz / Ref_clk) * 2^19  =>  it's for post_div=2
	 * Ref_clk = CPU_CLK * 2 / 32
	 */
    static const uint64_t divsrc = 432000000llu * 524288;
    rate = (divsrc / lpf_value * 2 / post_div * 16);

    // print MHz
    printf("%d\n", rate / 1000000);
}

void writefile(const char *fname, char *str)
{
    int fd = open(fname, O_WRONLY);
    if (fd >= 0) {
        write(fd, str, strlen(str));
        close(fd);
    }
}

//
//	set cpu clock
//		set governor = userspace, clk = 1200000 before call
//
void set_cpuclock(int clock)
{
    uint32_t post_div;
    if (clock >= 800000)
        post_div = 2;
    else if (clock >= 400000)
        post_div = 4;
    else if (clock >= 200000)
        post_div = 8;
    else
        post_div = 16;

    static const uint64_t divsrc = 432000000llu * 524288;
    uint32_t rate = (clock * 1000) / 16 * post_div / 2;
    uint32_t lpf = (uint32_t)(divsrc / rate);
    volatile uint16_t *p16 = (uint16_t *)pll_map;

    uint32_t cur_post_div = (p16[0x232] & 0x0F) + 1;
    uint32_t tmp_post_div = cur_post_div;
    if (post_div > cur_post_div) {
        while (tmp_post_div != post_div) {
            tmp_post_div <<= 1;
            p16[0x232] = (p16[0x232] & 0xF0) | ((tmp_post_div - 1) & 0x0F);
        }
    }

    p16[0x2A8] = 0x0000;       //reg_lpf_enable = 0
    p16[0x2AE] = 0x000F;       //reg_lpf_update_cnt = 32
    p16[0x2A4] = lpf & 0xFFFF; //set target freq to LPF high
    p16[0x2A6] = lpf >> 16;    //set target freq to LPF high
    p16[0x2B0] = 0x0001;       //switch to LPF control
    p16[0x2B2] |= 0x1000;      //from low to high
    p16[0x2A8] = 0x0001;       //reg_lpf_enable = 1
    while (!(p16[0x2BA] & 1))
        ;                      //polling done
    p16[0x2A0] = lpf & 0xFFFF; //store freq to LPF low
    p16[0x2A2] = lpf >> 16;    //store freq to LPF low

    if (post_div < cur_post_div) {
        while (tmp_post_div != post_div) {
            tmp_post_div >>= 1;
            p16[0x232] = (p16[0x232] & 0xF0) | ((tmp_post_div - 1) & 0x0F);
        }
    }
}

int main(int argc, char *argv[])
{
    getDeviceModel();
    if (!IS_MIYOO_PLUS_OR_FLIP() && DEVICE_ID != MIYOO283) {
        puts("This tool is only for Miyoo Mini");
        return 1;
    }

    int fd_mem = open("/dev/mem", O_RDWR);
    pll_map = mmap(0, PLL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd_mem, BASE_REG_MPLL_PA);

    int clock = 0;
    if (argc == 1) {
        print_clock();
        return 0;
    }
    if (argc == 2)
        clock = atoi(argv[1]);

    if ((clock < 100) || (clock > 2400)) {
        puts("usage: cpuclock freq[MHz, 100 - 2400]");
        munmap(pll_map, PLL_SIZE);
        close(fd_mem);
        return 1;
    }

    const char fn_governor[] = "/sys/devices/system/cpu/cpufreq/policy0/scaling_governor";
    const char fn_setspeed[] = "/sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed";
    char clockstr[16];
    sprintf(clockstr, "%d", clock * 1000);
    writefile(fn_governor, "userspace");
    writefile(fn_setspeed, clockstr);

    set_cpuclock(clock * 1000);
    print_clock();

    munmap(pll_map, PLL_SIZE);
    close(fd_mem);

    return 0;
}
