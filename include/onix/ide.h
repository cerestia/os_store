#ifndef ONIX_IDE_H
#define ONIX_IDE_H

#include <onix/types.h>
#include <onix/mutex.h>

#define SECTOR_SIZE 512

#define IDE_CTRL_NR 2
#define IDE_DISK_NR 2

typedef struct ide_disk_t
{
    char name[8];
    struct ide_ctrl_t* ctrl;
    u8 selector;
    bool master;
}ide_disk_t;

typedef struct ide_ctrl_t
{
    char name[8];
    lock_t lock;
    u16 iobase;
    ide_disk_t disks[IDE_DISK_NR];
    ide_disk_t* active;
}ide_ctrl_t;

int ide_pio_read(ide_disk_t* disk, void* buf,u8 count, idx_t lba);
int ide_pio_write(ide_disk_t* disk, void* buf, u8 count,idx_t lba);

#endif