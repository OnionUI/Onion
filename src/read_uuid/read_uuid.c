#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#define BIT8 0x100
#define BANK_TO_ADDR32(b) (b << 9)

#define REG_ADDR(riu_base, bank, reg_offset) ((riu_base) + BANK_TO_ADDR32(bank) + (reg_offset * 4))

typedef struct
{
    unsigned char *virt_addr;
    unsigned char *mmap_base;
    unsigned int mmap_length;
} MmapHandle;

static unsigned int const page_size_mask = 0xFFF;

MmapHandle *devMemMMap(unsigned int phys_addr, unsigned int length)
{
    int fd;
    unsigned int phys_offset;
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1)
    {
        printf("open /dev/mem fail\n");
        return NULL;
    }
    MmapHandle *handle = malloc(sizeof(MmapHandle));
    phys_offset = (phys_addr & (page_size_mask));
    phys_addr &= ~(page_size_mask);
    handle->mmap_length = length + phys_offset;
    handle->mmap_base = mmap(NULL, handle->mmap_length,
                             PROT_READ | PROT_WRITE, MAP_SHARED, fd, phys_addr);
    handle->virt_addr = handle->mmap_base + phys_offset;
    if (handle->mmap_base == MAP_FAILED)
    {
        printf("mmap fail\n");
        close(fd);
        free(handle);
        return NULL;
    }
    close(fd);
    return handle;
}

int devMemUmap(MmapHandle *handle)
{
    int ret = 0;
    ret = munmap(handle->mmap_base, handle->mmap_length);
    if (ret != 0)
    {
        printf("munmap fail\n");
        return ret;
    }

    free(handle);
    return ret;
}

int main()
{
    uint16_t word[3];
    MmapHandle *riu_base = devMemMMap(0x1F000000, 0x2B0000);

    *(unsigned short *)REG_ADDR(riu_base->virt_addr, 0x20, 0x03) &= ~BIT8;

    word[0] = *(unsigned short *)REG_ADDR(riu_base->virt_addr, 0x20, 0x16);
    word[1] = *(unsigned short *)REG_ADDR(riu_base->virt_addr, 0x20, 0x17);
    word[2] = *(unsigned short *)REG_ADDR(riu_base->virt_addr, 0x20, 0x18);
    
    devMemUmap(riu_base);

    printf("%04X%04X%04X\n", word[2], word[1], word[0]);
    return 0;
}