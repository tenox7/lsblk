# lsblk for windows

A utility similar to Linux *lsblk* for Windows. This is not a port of the Linux lsblk source code but rather a completely new utility with just similar output.

This is a very early version. Currently lacking support for Volumes, Mount Points and a number of other output fields from the Linux counterpart.


```
NAME            HCTL      SIZE TRIM  REMOV TYPE  DESCRIPTION
PhysicalDrive0  0:1:0:0  3577G True  False SATA    SAMSUNG MZ7LM3T8HCJM-00003 
 L Partition 1              0G             GPT   LDM metadata partition 
 L Partition 2              0G             GPT   Microsoft reserved partition 
 L Partition 3           3577G             GPT   LDM data partition 
PhysicalDrive1  2:0:0:0   477G True  False NVME  NVMe     Samsung SSD 950  
 L Partition 1              0G             GPT   Basic data partition [Required] 
 L Partition 2              0G             GPT   EFI system partition 
 L Partition 3              0G             GPT   Microsoft reserved partition 
 L Partition 4            476G             GPT   Basic data partition 
PhysicalDrive2  0:0:0:0              True  USB   Generic- SD/MMC           
PhysicalDrive3  0:0:0:1              True  USB   Generic- Compact Flash    
PhysicalDrive4  0:0:0:2              True  USB   Generic- SM/xD-Picture    
PhysicalDrive5  0:0:0:3              True  USB   Generic- MS/MS-Pro        
PhysicalDrive6  3:0:0:1     4G True  False VHD   Msft     Virtual Disk     
 L Partition 1              1G             MBR   IFS (HPFS/NTFS)  
```

For a similar more native Windows utility see [listdisk](https://github.com/tenox7/listdisk)

