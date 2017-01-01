Title: Bcache for Ubuntu 14.04 Root Filesystem
Date: 2014-05-28
Tags: Linux, Storage
Category: Technology

It's been a while since bcache made its way into the Linux kernel, but the
installers of most distributions have not yet caught up to allow users to
install to a bcache-backed volume, and user-land tools necessary to make use of
bcache are not yet installed by default.  There has been a tutorial on <a
href="https://github.com/g2p/blocks">how to convert the root of an existing
installation into bcache with a tool named blocks</a>, but the code base and
the depending code bases are either too old or too new  to be directly usable
with the default python 3.4 setup of Ubuntu 14.04.  After a frustrating process
of fixing all compatibility issues, I was able to make the code run, but I
don't trust my own patch (and the stability of github forked code) enough to
apply that on my real data.  I ended up this not-so-drastic, but cleaner and
safer way to get a bcache-backed root filesystem with minimal external
dependencies.

The idea is to (1) install Ubuntu into a normal partition (&gt;= 5GB) which would later be converted to the swap space, (2) setup the bcache, and (3) migrate / to the bcache device.

In my case (lenovo U430P), /dev/sda is a 16G SSD, and /dev/sdb is a 1T HDD.

# 1. Initial Installation
Install Ubuntu 14.04 using the following disk partitioning scheme
    - 64MB EFI partition /dev/sda1, fat32, mounted at /boot/efi. This is not necessary if the machine is booted in the traditional BIOS way.</li>
    - 200MB ext4 partition /dev/sda2 to be mounted on /boot.  It is necessary to make /boot on a separate partition.  I made this on SSD to make it faster for the kernel to be loaded (haven't compared, but I guess the speedup over HDD -- if not slowdown -- won't be that obvious as the kernel is a multi-megabyte file.)</li>
    - 16GB ext4 partition /dev/sdb1 to be mounted as / in installation.  We'll later convert that to be a swap partition.</li>
    - An empty big partition /dev/sdb2 later to be used as root (/dev/sda2).  Create this partition, but do not use it for now.</li>
    - An empty partition /dev/sda3 on SSD later to be used as the cache.  Create this partition, but do not use it for now.</li>

The installer will complain about not having a swap space.  Ignore that.
# 2. Setting Up Bcache
After installation, boot into the newly installed system, install bcache-tools (in PPA) and setup the system:

```bash
$ sudo bash
# add-apt-repository ppa:g2p/storage
# apt-get update
# apt-get install bcache-tools
# make-bcache -C /dev/sda3 -B /dev/sdb2
# mkfs.ext4 /dev/bcache0
```
# 3. Migrating Root Filesystem
Keep working in the newly installed system.

```bash
$ sudo bash
$ mkdir OLD NEW
# mount /dev/sdb1 OLD    # the old root
# mount /dev/bcache0 NEW # this would be our new root
# rsync -a OLD/ NEW/     # now NEW contains the root
# ### mount a serious directory in preparation for grub-install
# mount /dev/sda2 NEW/boot
# mount /dev/sda1 NEW/boot/efi
# mount -o bind /dev NEW/dev
# mount -t proc none NEW/proc
# mount -t sysfs none NEW/sys
# chroot NEW
# #### find out the UUID of /dev/bcache0 and /dev/sdb1
# ls -l /dev/disk/by-uuid/ | grep bcache0
lrwxrwxrwx 1 root root 13 May 29 21:49 4c492013-e8a3-40b5-b5cd-9220ed2e0195 -> ../../bcache0
# ls -l /dev/disk/by-uuid/ | grep sdb1
lrwxrwxrwx 1 root root 10 May 29 21:49 765d6fc0-9ff4-4cf4-95f9-17a6e76ae80c -> ../../sdb1
# vi NEW/etc/fstab NEW/boot/grub.cfg #### edit the files NEW/etc/fstab and NEW/boot/grub.cfg, replace all UUID of sdb1 to that of bcache0.
# grub-install /dev/sda
```
# 4. Final Configurations in New System
Reboot into the newly installed system. Now the root is on /dev/bcache0. The old data on /dev/sdb1 is not used, and /dev/sdb1 can be converted to the swap space.

```bash
$ sudo bash
# mkswap /dev/sdb1
Setting up swapspace version 1, size = 15624188 KiB
no label, UUID=e35bc636-9944-4dd5-ab3d-6c371b0cb7a8
# swapon /dev/sdb1
##### make sure to change the UUID of the command below
echo "UUID=e35bc636-9944-4dd5-ab3d-6c371b0cb7a8 none swap defaults 0 0" >> /etc/fstab
```

Now I'm having my Ubuntu running happily on bcache, and I hope it's not going to cause and data loss.
