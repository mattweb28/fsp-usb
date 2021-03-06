
#include <switch.h>
#include <cstdio>
#include <sys/stat.h>
#include <dirent.h>
#include <string>
#include <iostream>

extern "C"
{
    #include "fspusb.h"
}

#define LOG(...) { std::cout << __VA_ARGS__ << std::endl; consoleUpdate(NULL); }

void TestGetSetLabel()
{
    LOG("Searching for drives...")

    u32 count = 0;
    auto rc = fspusbGetMountedDriveCount(&count);
    if(R_SUCCEEDED(rc))
    {
        LOG("Found " << count << " drives")
        if(count > 0)
        {
            // Test with the 1st drive
            u32 drv = 0;
            LOG("Using first drive...")

            char label[0x10] = {0};
            rc = fspusbGetDriveLabel(drv, label, 0x10);
            if(R_SUCCEEDED(rc)) LOG("Got drive's label: '" << label << "'")
            else LOG("Error getting drive's label: 0x" << std::hex << rc)

            LOG("Setting new drive label...")
            const char *newlabel = "TEST USB";
            rc = fspusbSetDriveLabel(drv, newlabel);
            if(R_SUCCEEDED(rc)) LOG("Drive label set to: '" << newlabel << "'")
            else LOG("Error setting drive's label: 0x" << std::hex << rc)

        }
    }
}

void TestFsType()
{
    LOG("Searching for drives...")

    u32 count = 0;
    auto rc = fspusbGetMountedDriveCount(&count);
    if(R_SUCCEEDED(rc))
    {
        LOG("Found " << count << " drives")
        if(count > 0)
        {
            // Test with the 1st drive
            u32 drv = 0;
            LOG("Using first drive...")

            FspUsbFileSystemType fstype;
            rc = fspusbGetDriveFileSystemType(drv, &fstype);
            if(R_SUCCEEDED(rc))
            {
                const char *fs = "<unknown type>";
                switch(fstype)
                {
                    case FspUsbFileSystemType_FAT12:
                        fs = "FAT12";
                        break;
                    case FspUsbFileSystemType_FAT16:
                        fs = "FAT16";
                        break;
                    case FspUsbFileSystemType_FAT32:
                        fs = "FAT32";
                        break;
                    case FspUsbFileSystemType_exFAT:
                        fs = "exFAT";
                        break;
                    default:
                        break;
                }
                LOG("Drive's filesystem type: " << fs)
            }
            else LOG("Error getting drive's filesystem type: 0x" << std::hex << rc)

        }
    }
}

void TestFileSystem()
{
    LOG("Searching for drives...")

    u32 count = 0;
    auto rc = fspusbGetMountedDriveCount(&count);
    if(R_SUCCEEDED(rc))
    {
        LOG("Found " << count << " drives")
        if(count > 0)
        {
            // Test with the 1st drive
            u32 drv = 0;
            LOG("Using first drive...")

            FsFileSystem drvfs;
            rc = fspusbOpenDriveFileSystem(drv, &drvfs);
            if(R_SUCCEEDED(rc))
            {
                LOG("Opened drive's filesystem. Mounting it...")
                int res = fsdevMountDevice("usbdrv", drvfs); // Mount the filesystem as usbdrv:/
                if(res == -1) LOG("Error mounting drive filesystem...")
                else
                {
                    LOG("Drive filesystem mounted as usbdrv:/")

                    LOG("Listing all files and directories on root...")

                    DIR *dp = opendir("usbdrv:/");
                    if(dp)
                    {
                        dirent *dt;
                        while(true)
                        {
                            dt = readdir(dp);
                            if(dt == NULL) break;
                            LOG(" [" << ((dt->d_type & DT_DIR) ? "D" : "F") << "] usbdrv:/" << dt->d_name)
                        }
                        closedir(dp);

                        LOG("Listed all files and dirs! :) ")
                    }
                    else
                    {
                        LOG("Bad opendir()...")
                        consoleUpdate(NULL);  
                    }
                    LOG("Unmounting device...")
                    fsdevUnmountDevice("usbdrv");
                    LOG("Device unmounted.")
                }
            }
            else LOG("Error getting drive's filesystem type: 0x" << std::hex << rc)
        }
    }
}

int main()
{
    socketInitializeDefault();
    nxlinkStdio();

    consoleInit(NULL);

    auto rc = fspusbInitialize();
    if(R_SUCCEEDED(rc))
    {
        // Run all tests
        TestFsType();
        LOG(" --- ")
        TestGetSetLabel();
        LOG(" --- ")
        TestFileSystem();

        fspusbExit();
    }
    else LOG("Error accessing fsp-usb: 0x" << std::hex << rc)

    while(appletMainLoop())
    {
        hidScanInput();
        if(hidKeysDown(CONTROLLER_P1_AUTO)) break;
    }

    consoleExit(NULL);
    socketExit();
    
    return 0;
}