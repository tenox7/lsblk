//
// lsblk for Windows
//
// Copyright (c) 2016-2018 by Antoni Sawicki
// Copyright (c) 2021 by Google LLC
//

#include <windows.h>
#include <wchar.h>
#include <stdio.h>
#include <initguid.h>
#include <winternl.h>
#include <Ntddscsi.h>
#include <ntdddisk.h>

#pragma comment(lib, "ntdll.lib")
#pragma comment(lib, "shlwapi.lib")

DWORD debug = 0;

#define DIRECTORY_QUERY                 (0x0001)
#define DIRECTORY_TRAVERSE              (0x0002)
typedef struct _OBJECT_DIRECTORY_INFORMATION {
    UNICODE_STRING Name;
    UNICODE_STRING TypeName;
} OBJECT_DIRECTORY_INFORMATION, * POBJECT_DIRECTORY_INFORMATION;

typedef struct _VOLINFO {
    WCHAR    DiskName[256];
    LONGLONG Start;
    LONGLONG Length;
    WCHAR    MntPaths[1024];
    WCHAR    FsType[MAX_PATH + 1];
} VOLINFO, * PVOLINFO;

WCHAR* ft[] = { L"False", L"True" };
WCHAR* bus[] = { L"UNKNOWN", L"SCSI", L"ATAPI", L"ATA", L"1394", L"SSA", L"FC", L"USB", L"RAID", L"ISCSI", L"SAS", L"SATA", L"SD", L"MMC", L"VIRTUAL", L"VHD", L"MAX", L"NVME" };
WCHAR* layout[] = { L"MBR", L"GPT", L"RAW" };
char* MBRTypes[] = { "Unused", "FAT12", "XENIX root", "XENIX /usr", "FAT16 < 32 MiB", "Extended", "FAT16", "IFS (HPFS/NTFS)", "AIX boot, OS/2, Commodore DOS", "AIX data, Coherent, QNX", "Coherent swap, OPUS, OS/2 Boot Manager", "FAT32", "FAT32 (LBA)", "Unknown", "FAT16 (LBA)", "Extended (LBA)", "OPUS", "Hidden FAT12", "Compaq diagnostics, recovery partition", "Unknown", "Hidden FAT16 < 32 MiB, AST-DOS", "Unknown", "Hidden FAT16", "Hidden IFS (HPFS/NTFS)", "AST-Windows swap", "Willowtech Photon coS", "Unknown", "Hidden FAT32", "Hidden FAT32 (LBA)", "Unknown", "Hidden FAT16 (LBA)", "Unknown", "Willowsoft Overture File System", "Oxygen FSo2", "Oxygen Extended ", "SpeedStor reserved", "NEC-DOS", "Unknown", "SpeedStor reserved", "Hidden NTFS", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "SpeedStor reserved", "Unknown", "SpeedStor reserved", "SpeedStor reserved", "Unknown", "SpeedStor reserved", "Unknown", "Theos", "Plan 9", "Unknown", "Unknown", "Partition Magic", "Hidden NetWare", "Unknown", "Unknown", "VENIX 80286", "PReP Boot", "Secure File System", "PTS-DOS", "Unknown", "Priam, EUMEL/Elan", "EUMEL/Elan", "EUMEL/Elan", "EUMEL/Elan", "Unknown", "ALFS/THIN lightweight filesystem for DOS", "Unknown", "Unknown", "QNX 4", "QNX 4", "QNX 4, Oberon", "Ontrack DM, R/O, FAT", "Ontrack DM, R/W, FAT", "CP/M, Microport UNIX", "Ontrack DM 6", "Ontrack DM 6", "EZ-Drive", "Golden Bow VFeature", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Priam EDISK", "Unknown", "Unknown", "Unknown", "Unknown", "SpeedStor", "Unknown", "GNU Hurd, System V, 386/ix", "NetWare 286", "NetWare", "NetWare 386", "NetWare", "NetWare", "NetWare NSS", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "DiskSecure Multi-Boot", "Unknown", "UNIX 7th Edition", "Unknown", "Unknown", "IBM PC/IX", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Old MINIX", "MINIX, Old Linux", "Linux swap, Solaris", "Linux", "Hidden by OS/2, APM hibernation", "Linux extended", "NT Stripe Set", "NT Stripe Set", "Linux Plaintext", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "Linux LVM", "Unknown", "Unknown", "Unknown", "Unknown", "Amoeba, Hidden Linux", "Amoeba bad blocks", "Unknown", "Unknown", "Unknown", "Unknown", "Mylex EISA SCSI", "Unknown", "Unknown", "Unknown", "Unknown", "Unknown", "BSD/OS", "Hibernation", "HP Volume Expansion", "Unknown", "HP Volume Expansion", "HP Volume Expansion", "FreeBSD", "OpenBSD", "NeXTStep", "Apple UFS", "NetBSD", "Olivetti DOS FAT12", "Apple Boot", "Unknown", "Unknown", "Unknown", "Apple HFS", "BootStar", "HP Volume Expansion", "Unknown", "HP Volume Expansion", "HP Volume Expansion", "Unknown", "HP Volume Expansion", "BSDi", "BSDi swap", "Unknown", "Unknown", "PTS BootWizard", "Unknown", "Unknown", "Solaris boot", "Solaris", "Novell DOS, DR-DOS secured", "DR-DOS secured FAT12", "DR-DOS reserved", "DR-DOS reserved", "DR-DOS secured FAT16 < 32 MiB", "Unknown", "DR-DOS secured FAT16", "Syrinx", "DR-DOS reserved", "DR-DOS reserved", "DR-DOS reserved", "DR-DOS secured FAT32", "DR-DOS secured FAT32 (LBA)", "DR-DOS reserved", "DR-DOS secured FAT16 (LBA)", "DR-DOS secured extended (LBA)", "Multiuser DOS secured FAT12", "Multiuser DOS secured FAT12", "Unknown", "Unknown", "Multiuser DOS secured FAT16 < 32 MiB", "Multiuser DOS secured extended", "Multiuser DOS secured FAT16", "Unknown", "CP/M", "Unknown", "Filesystem-less data", "CP/M, CCP/M, CTOS", "Unknown", "Unknown", "Dell partition", "BootIt EMBRM", "Unknown", "SpeedStor", "DOS read/only", "SpeedStor", "SpeedStor", "Tandy DOS", "SpeedStor", "Unknown", "Unknown", "Unknown", "Unknown", "BeOS", "Unknown", "Spryt*x", "Guid Partition Table", "EFI system partition", "Linux boot", "SpeedStor", "DOS 3.3 secondary, Unisys DOS", "SpeedStor", "SpeedStor", "Prologue", "SpeedStor", "Unknown", "Unknown", "Unknown", "Unknown", "VMWare VMFS", "VMWare VMKCORE", "Linux RAID, FreeDOS", "SpeedStor, LANStep, PS/2 IML", "Xenix bad block" };


VOID ErrPt(BOOL, WCHAR*, ...);
VOID ListDisks(PVOLINFO*, DWORD);
VOID QueryDisk(WCHAR*, PVOLINFO*, DWORD);
DWORD ListVolumes(PVOLINFO*);
NTSTATUS QueryVolume(WCHAR*, PVOLINFO);
VOID DumpVolumes(PVOLINFO*, DWORD);

VOID ErrPt(BOOL exit, WCHAR* msg, ...) {
    va_list ap;
    WCHAR buff[1024] = { 0 };
    DWORD err;

    wprintf(L"Error: ");
    va_start(ap, msg);
    vwprintf(msg, ap);
    va_end(ap);
    putchar(L'\n');
    err = GetLastError();
    if (err) {
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buff, ARRAYSIZE(buff), NULL);
        wprintf(L"[0x%08X] %s\n", err, buff);
    }

    if (exit)
        ExitProcess(1);
}

VOID ListDisks(PVOLINFO* Mounts, DWORD mnts) {
    HANDLE hDir;
    OBJECT_ATTRIBUTES attr = { 0 };
    UNICODE_STRING root = { 0 };
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    DWORD nDirInfo = 2048;
    ULONG i = 0, index = 0, bytes = 0, istart = 0, first = 0;
    NTSTATUS status;

    RtlInitUnicodeString(&root, L"\\GLOBAL??");
    InitializeObjectAttributes(&attr, &root, 0, NULL, NULL);

    status = NtOpenDirectoryObject(&hDir, DIRECTORY_QUERY | DIRECTORY_TRAVERSE, &attr);
    if (status != 0)
        ErrPt(TRUE, L"NtOpenDirectoryObject NTSTATUS=%08X", status);

    if (debug) wprintf(L"Disks:\n");
    DirInfo = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(OBJECT_DIRECTORY_INFORMATION) * nDirInfo);
    if (DirInfo == NULL)
        ErrPt(TRUE, L"Unable to allocate memory");
    first = TRUE;
    istart = 0;
    do {
    moremem:
        status = NtQueryDirectoryObject(hDir, DirInfo, sizeof(OBJECT_DIRECTORY_INFORMATION) * nDirInfo, FALSE, first, &index, &bytes);
        if (status == 0x00000105) {
            if (debug) wprintf(L"\nNtQueryDirectoryObject needs more memory n=%d\n", nDirInfo);
            nDirInfo = nDirInfo * 2;
            DirInfo = (POBJECT_DIRECTORY_INFORMATION)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DirInfo, sizeof(OBJECT_DIRECTORY_INFORMATION) * nDirInfo);
            if (DirInfo == NULL)
                ErrPt(TRUE, L"Unable to allocate memory for NtQueryDirectoryObject");
            goto moremem;
        }
        if (status < 0)
            break;

        for (i = 0; i < index - istart; i++) {
            if (wcsncmp(DirInfo[i].Name.Buffer, L"PhysicalDrive", 13) == 0) {
                if (debug) wprintf(L"\\\\.\\%s\n", DirInfo[i].Name.Buffer);
                QueryDisk(DirInfo[i].Name.Buffer, Mounts, mnts);
            }
        }

        istart = index;
        first = FALSE;
    } while (TRUE);

    NtClose(hDir);
    HeapFree(GetProcessHeap(), 0, DirInfo);
}

VOID QueryDisk(WCHAR* name, PVOLINFO* Mounts, DWORD mnts) {
    HANDLE hDisk;
    GET_LENGTH_INFORMATION  DiskLengthInfo;
    GET_DISK_ATTRIBUTES DiskAttributes;
    PDRIVE_LAYOUT_INFORMATION_EX DiskLayout;
    WCHAR nDiskLayout = 64;
    SCSI_ADDRESS DiskAddress;
    STORAGE_PROPERTY_QUERY desc_q = { StorageDeviceProperty,  PropertyStandardQuery };
    STORAGE_DESCRIPTOR_HEADER desc_h = { 0 };
    PSTORAGE_DEVICE_DESCRIPTOR desc_d;
    STORAGE_PROPERTY_QUERY trim_q = { StorageDeviceTrimProperty,  PropertyStandardQuery };
    DEVICE_TRIM_DESCRIPTOR trim_d = { 0 };
    OBJECT_ATTRIBUTES attr = { 0 };
    UNICODE_STRING diskname = { 0 };
    WCHAR buff[1024] = { 0 };
    PWCHAR mnt = NULL;
    PWCHAR fst = NULL;
    IO_STATUS_BLOCK iosb;
    NTSTATUS status;
    int i, n, p;

    swprintf(buff, ARRAYSIZE(buff), L"\\??\\%s", name);
    RtlInitUnicodeString(&diskname, buff);
    InitializeObjectAttributes(&attr, &diskname, 0, NULL, NULL);

    wprintf(L"%-15s", name);

    status = NtOpenFile(&hDisk, GENERIC_READ | SYNCHRONIZE, &attr, &iosb, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);
    if (status != 0) {
        ErrPt(FALSE, L"Unable to open %s, NTSTATUS=0x%08X\n\n", diskname.Buffer, status);
        return;
    }

    // SCSI Address
    status = NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_SCSI_GET_ADDRESS, NULL, 0, &DiskAddress, sizeof(DiskAddress));
    if (status == 0)
        wprintf(L" %d:%d:%d:%d", DiskAddress.PortNumber, DiskAddress.PathId, DiskAddress.TargetId, DiskAddress.Lun);
    else
        wprintf(L" -:-:-:-");

    // Size
    status = NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_DISK_GET_LENGTH_INFO, NULL, 0, &DiskLengthInfo, sizeof(DiskLengthInfo));
    if (status == 0)
        wprintf(L" %5.0fG", (float)DiskLengthInfo.Length.QuadPart / 1024.0 / 1024.0 / 1024.0);
    else
        wprintf(L"       ");

    // Status
    status = NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_DISK_GET_DISK_ATTRIBUTES, NULL, 0, &DiskAttributes, sizeof(DiskAttributes));
    if (status == 0)
        wprintf(L" %d ", (DiskAttributes.Attributes) ? 0 : 1);
    else
        wprintf(L"   ");

    // Trim
    status = NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_STORAGE_QUERY_PROPERTY, &trim_q, sizeof(trim_q), &trim_d, sizeof(trim_d));
    if (status == 0)
        wprintf(L" %d ", (trim_d.Version == sizeof(DEVICE_TRIM_DESCRIPTOR) && trim_d.TrimEnabled == 1) ? 1 : 0);
    else
        wprintf(L" 0 ");

    // Device Property
    status = NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_STORAGE_QUERY_PROPERTY, &desc_q, sizeof(desc_q), &desc_h, sizeof(desc_h));
    if (status != 0) {
        NtClose(hDisk);
        wprintf(L"\n");
        if (debug) ErrPt(FALSE, L"IOCTL_STORAGE_QUERY_PROPERTY NTSTATUS=%08X", status);
        return;
    }

    desc_d = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, desc_h.Size);
    if (desc_d == NULL) {
        NtClose(hDisk);
        wprintf(L"\n");
        if (debug) ErrPt(FALSE, L"Unable to allocate memory");
        return;
    }

    status = NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_STORAGE_QUERY_PROPERTY, &desc_q, sizeof(desc_q), desc_d, desc_h.Size);
    if (status != 0 || desc_d->Version != sizeof(STORAGE_DEVICE_DESCRIPTOR)) {
        NtClose(hDisk);
        wprintf(L"\n");
        return;
    }
    swprintf(buff, sizeof(buff) / sizeof(WCHAR), L"%S %S",
        (desc_d->VendorIdOffset) ? (char*)desc_d + desc_d->VendorIdOffset : "",
        (desc_d->ProductIdOffset) ? (char*)desc_d + desc_d->ProductIdOffset : ""
    );
    StrTrimW(buff, L" ");
    wprintf(
        L" %d "
        L" %d "
        L" %d "
        L" %-5s "
        L"%s\n",
        desc_d->RemovableMedia,
        (NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_STORAGE_CHECK_VERIFY2, NULL, 0, NULL, 0) == 0) ? 1 : 0,
        (NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_DISK_IS_WRITABLE, NULL, 0, NULL, 0) < 0) ? 1 : 0,
        (desc_d->BusType < ARRAYSIZE(bus)) ? bus[desc_d->BusType] : bus[0],
        buff
    );
    HeapFree(GetProcessHeap(), 0, desc_d);

    // Partitions
    DiskLayout = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DRIVE_LAYOUT_INFORMATION_EX) * nDiskLayout);
moremem:
    status = NtDeviceIoControlFile(hDisk, NULL, NULL, NULL, &iosb, IOCTL_DISK_GET_DRIVE_LAYOUT_EX, NULL, 0, DiskLayout, sizeof(DRIVE_LAYOUT_INFORMATION_EX) * nDiskLayout);
    if (status == 0xC0000023) {
        if (debug) wprintf(L"\nIOCTL_DISK_GET_DRIVE_LAYOUT_EX needs more memory n=%d\n", nDiskLayout);
        nDiskLayout = nDiskLayout * 2;
        DiskLayout = (PDRIVE_LAYOUT_INFORMATION_EX)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, DiskLayout, sizeof(DRIVE_LAYOUT_INFORMATION_EX) * nDiskLayout);
        if (DiskLayout == NULL)
            ErrPt(TRUE, L"Unable to allocate memory for DiskLayout");
        goto moremem;
    }
    if (status != 0) {
        NtClose(hDisk);
        HeapFree(GetProcessHeap(), 0, DiskLayout);
        wprintf(L"\n");
        if (debug) ErrPt(FALSE, L"IOCTL_DISK_GET_DRIVE_LAYOUT_EX NTSTATUS=%08X", status);
        return;
    }

    for (n = 0; n < DiskLayout->PartitionCount; n++) {
        if (!DiskLayout->PartitionEntry[n].PartitionNumber)
            continue;

        // Mount Points
        for (p = 0; p < mnts; p++) {
            if (debug > 1) wprintf(L"\n  %d: %s==%s %lld==%lld %lld=%lld\n", p, (*Mounts)[p].DiskName, name, (*Mounts)[p].Start, DiskLayout->PartitionEntry[n].StartingOffset.QuadPart, (*Mounts)[p].Length, DiskLayout->PartitionEntry[n].PartitionLength.QuadPart);
            mnt = NULL;
            if (wcscmp((*Mounts)[p].DiskName, name) == 0 &&
                (*Mounts)[p].Start == DiskLayout->PartitionEntry[n].StartingOffset.QuadPart &&
                (*Mounts)[p].Length == DiskLayout->PartitionEntry[n].PartitionLength.QuadPart
                ) {
                //wprintf(L"%s", (*Mounts)[p].MntPaths);
                mnt = (*Mounts)[p].MntPaths;
                fst = (*Mounts)[p].FsType;
                break;
            }
        }

        wprintf(L" L Partition %d          %5.0fG                %-4s  ",
            DiskLayout->PartitionEntry[n].PartitionNumber,
            (float)DiskLayout->PartitionEntry[n].PartitionLength.QuadPart / 1024 / 1024 / 1024,
            layout[DiskLayout->PartitionEntry[n].PartitionStyle]
        );

        if (fst && wcslen(fst))
            wprintf(L"%s ", fst);

        if (DiskLayout->PartitionEntry[n].PartitionStyle == PARTITION_STYLE_MBR) {
            if (mnt != NULL && wcslen(mnt) > 2)
                wprintf(L"%s", mnt);
            else
                wprintf(L"%S ", MBRTypes[DiskLayout->PartitionEntry[n].Mbr.PartitionType]);

            wprintf(L"%s", (DiskLayout->PartitionEntry[n].Mbr.BootIndicator) ? L" [Active]" : L" ");
        }
        else if (DiskLayout->PartitionEntry[n].PartitionStyle == PARTITION_STYLE_GPT) {
            if (mnt != NULL && wcslen(mnt) > 2)
                wprintf(L"%s", mnt);
            else
                wprintf(L"%s ", DiskLayout->PartitionEntry[n].Gpt.Name);

            if ((DiskLayout->PartitionEntry[n].Gpt.Attributes & GPT_ATTRIBUTE_PLATFORM_REQUIRED) == GPT_ATTRIBUTE_PLATFORM_REQUIRED)
                wprintf(L"[Required] ");
            if ((DiskLayout->PartitionEntry[n].Gpt.Attributes & GPT_BASIC_DATA_ATTRIBUTE_HIDDEN) == GPT_BASIC_DATA_ATTRIBUTE_HIDDEN)
                wprintf(L"[Hidden] ");
            if ((DiskLayout->PartitionEntry[n].Gpt.Attributes & GPT_BASIC_DATA_ATTRIBUTE_SHADOW_COPY) == GPT_BASIC_DATA_ATTRIBUTE_SHADOW_COPY)
                wprintf(L"[Shadow_Copy] ");
            if ((DiskLayout->PartitionEntry[n].Gpt.Attributes & GPT_BASIC_DATA_ATTRIBUTE_READ_ONLY) == GPT_BASIC_DATA_ATTRIBUTE_READ_ONLY)
                wprintf(L"[Readonly] ");
        }
        else {
            wprintf(L"RAW");
        }

        wprintf(L"\n");
    }

    NtClose(hDisk);
    HeapFree(GetProcessHeap(), 0, DiskLayout);
}

DWORD ListVolumes(PVOLINFO* Mounts) {
    WCHAR VolName[1024] = { 0 };
    HANDLE find;
    DWORD n = 1;

    find = FindFirstVolumeW(VolName, ARRAYSIZE(VolName));
    if (find == INVALID_HANDLE_VALUE) {
        ErrPt(FALSE, L"Unable to list volumes\n");
        return 0;
    }

    *Mounts = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(VOLINFO));
    if (*Mounts == NULL) {
        ErrPt(FALSE, L"Unable to allocate memory\n");
        return 0;
    }

    if (debug) wprintf(L"Volumes:\n");

    do {
        if (QueryVolume(VolName, &(*Mounts)[n - 1]) != 0)
            continue;

        if (debug) wprintf(L"n=%d d=%s f=%s p=%s\n", n, (*Mounts)[n - 1].DiskName, (*Mounts)[n - 1].FsType, (*Mounts)[n - 1].MntPaths);

        n++;
        *Mounts = (PVOLINFO)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, *Mounts, n * sizeof(VOLINFO));
        if (*Mounts == NULL) {
            ErrPt(FALSE, L"Unable to reallocate memory\n");
            return n - 1;
        }
    } while (FindNextVolumeW(find, VolName, ARRAYSIZE(VolName)));

    return n - 1;
}

NTSTATUS QueryVolume(WCHAR* name, PVOLINFO Mounts) {
    WCHAR buff[1024] = { 0 };
    DWORD len, n;
    PWCHAR next;
    NTSTATUS status;
    OBJECT_ATTRIBUTES attr = { 0 };
    UNICODE_STRING volname = { 0 };
    IO_STATUS_BLOCK iosb;
    HANDLE hVol;
    VOLUME_DISK_EXTENTS volext;

    if (debug) (L"\n* %s\n", name);

    // Filesystem Type
    GetVolumeInformationW(name, NULL, 0, NULL, NULL, NULL, Mounts->FsType, ARRAYSIZE(Mounts->FsType));

    // Extents
    swprintf(buff, ARRAYSIZE(buff), L"\\GLOBAL??%s", name + 3);
    buff[wcslen(buff) - 1] = L'\0';
    if (debug) wprintf(L" %s\n", buff);
    RtlInitUnicodeString(&volname, buff);
    InitializeObjectAttributes(&attr, &volname, 0, NULL, NULL);

    status = NtOpenFile(&hVol, SYNCHRONIZE, &attr, &iosb, FILE_SHARE_READ, FILE_SYNCHRONOUS_IO_NONALERT);
    if (status != 0) {
        if (debug) ErrPt(FALSE, L"Unable to open %s, NTSTATUS=0x%08X\n\n", volname.Buffer, status);
        return 1;
    }

    // TODO: add support for dynamic disks
    status = NtDeviceIoControlFile(hVol, NULL, NULL, NULL, &iosb, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, NULL, 0, &volext, sizeof(VOLUME_DISK_EXTENTS));
    if (status != 0) {
        if (debug) ErrPt(FALSE, L"Unable to get volume extents, NTSTATUS=0x%08X\n\n", status);
        NtClose(hVol);
        return 1;
    }
    NtClose(hVol);

    if (volext.NumberOfDiskExtents < 1)
        return 1;

    if (debug) wprintf(L" Extent: disk=%d start=%llu len=%llu \n\n",
        volext.Extents[0].DiskNumber,
        volext.Extents[0].StartingOffset.QuadPart,
        volext.Extents[0].ExtentLength.QuadPart
    );

    swprintf(Mounts->DiskName, ARRAYSIZE(Mounts->DiskName), L"PhysicalDrive%d", volext.Extents[0].DiskNumber);
    Mounts->Start = volext.Extents[0].StartingOffset.QuadPart;
    Mounts->Length = volext.Extents[0].ExtentLength.QuadPart;

    // Mount points
    if (!GetVolumePathNamesForVolumeNameW(name, buff, ARRAYSIZE(buff), &len))
        return 0;

    if (len < 2)
        return 0;

    if (debug) wprintf(L"Mounts: \n");

    for (next = buff; next[0] != L'\0'; next += wcslen(next) + 1) {
        if (debug) wprintf(L" - %s ", next);
        swprintf(Mounts->MntPaths, ARRAYSIZE(Mounts->MntPaths), L"%s%s ", Mounts->MntPaths, next);
    }
    if (debug) wprintf(L"\n");

    return 0;
}

VOID DumpVolumes(PVOLINFO* Mounts, DWORD mnts) {
    DWORD n;
    wprintf(L"\nMounts:\n\n");
    for (n = 0; n < mnts; n++) {
        wprintf(L"%d: disk=%s start=%llu len=%llu fs=%s paths=%s\n",
            n,
            (*Mounts)[n].DiskName,
            (*Mounts)[n].Start,
            (*Mounts)[n].Length,
            (*Mounts)[n].FsType,
            (*Mounts)[n].MntPaths
        );
    }
    return;
}



int wmain(int argc, WCHAR** argv) {
    PVOLINFO Vols;
    DWORD nvol = 0;
    BOOL getvols = TRUE;
    int n;

    for (n = 1; n < argc; n++) {
        if (wcscmp(argv[n], L"-d") == 0)
            debug = 1;
        if (wcscmp(argv[n], L"-n") == 0)
            getvols = FALSE;
    }

    if (getvols)
        nvol = ListVolumes(&Vols);
    if (debug) DumpVolumes(&Vols, nvol);

    wprintf(L"lsblk for Windows, v2.0, Copyright (c) 2021 Google LLC\n\n"
        L"NAME            HCTL      SIZE ST TR RM MD RO TYPE  DESCRIPTION\n");

    ListDisks(&Vols, nvol);
    return 0;
}
