# lsblk for windows

Utility similar to Linux *lsblk(1)* for Windows. This is not a port of the Linux lsblk source code but rather a completely new utility with just a similar output.

The latest version supports printing drive letters, mount points and filesystem types.

```
NAME            HCTL      SIZE ST TR RM MD RO TYPE  DESCRIPTION
PhysicalDrive0  0:1:0:0  3577G 1  1  0  1  0  SATA  SAMSUNG M2345-005 
 L Partition 1              0G                GPT   LDM metadata partition 
 L Partition 2              0G                GPT   Microsoft reserved partition 
 L Partition 3           3577G                GPT   NTFS D:\ C:\mnt 
PhysicalDrive1  2:0:0:0   477G 1  1  0  1  0  NVME  NVMe     Samsung SSD 950  
 L Partition 1              0G                GPT   Basic data partition [Required] 
 L Partition 2              0G                GPT   EFI system partition 
 L Partition 3              0G                GPT   Microsoft reserved partition 
 L Partition 4            476G                GPT   NTFS C:\ 
PhysicalDrive2  0:0:0:0     7G 1  0  1  1  1  USB   Generic- SD/MMC           
 L Partition 1              7G                MBR   FAT32  
PhysicalDrive3  0:0:0:1        1  0  1  0  0  USB   Generic- Compact Flash    
PhysicalDrive4  0:0:0:2        1  0  1  0  0  USB   Generic- SM/xD-Picture    
PhysicalDrive5  0:0:0:3        1  0  1  0  0  USB   Generic- MS/MS-Pro        
PhysicalDrive6  3:0:0:1     4G 0  1  0  1  1  VHD   Msft     Virtual Disk     
 L Partition 1              1G                MBR   IFS (HPFS/NTFS)  
```

Download under [releases](https://github.com/tenox7/lsblk/releases)

For a similar more native Windows utility see [listdisk](https://github.com/tenox7/listdisk)

