# lsblk for windows

Utility similar to Linux *lsblk(1)* for Windows. This is not a port of the Linux lsblk source code but rather a completely new utility with just a similar output.

The latest version supports printing drive letters, mount points and filesystem types.

![lsblk screenshot](lsblk.png)

Download under [releases](https://github.com/tenox7/lsblk/releases)

For a similar more native Windows utility see [listdisk](https://github.com/tenox7/listdisk)

Explanation of header columns:

```
ST - Status (1=healthy, 0=unhealthy)
TR - Trim/Unmap/Discard capability support
RM - Removable media (1=yes, 0=no)
MD - Media changed (for removable media)
RO - Read only (1=yes, 0=no)
```

Flags:

```
-n - do not list, query and print volumes, drive letters and mount points
-d - debug output
```
