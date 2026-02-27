## 💎 Crystal Blue OS

Crystal Blue OS is a 32-bit x86 operating system, written entirely **from scratch**. This project is not based on an existing kernel (like Linux or BSD) and does not depend on any standard library (libc). Everything displayed on the screen, every keyboard reaction, and every tick of the clock is managed directly by the kernel.

> [!IMPORTANT]
> **Project Status:** In development

The goal is to understand and master every layer between the silicon and the user:

* **Zero Dependencies:** No `printf`, no system `malloc`. Every utility function (`strcmp`, `kprint`, etc.) is manually re-implemented.
* **Bare Metal Mode:** The code runs directly on the processor. The kernel itself handles the transition into 32-bit Protected Mode.
* **Hardware Dialogue:** Direct communication with chips via I/O ports (`outb`, `inb`).

---

### Current Features (v0.6.1)

The system features a solid technical foundation:

* **Bootloader:** Loading the kernel from the boot sector.
* **Memory Management:** Implementation of the **GDT** (Global Descriptor Table).
* **Interrupt System (IDT):** Full handler for the keyboard and timer.
* **Real-Time Clock:** Live time display in the top right corner (reading the CMOS chip via the PIT).
* **Shell Security:** Integrated login system with password masking.
* **Interactive Shell:** Command support (`help`, `cls`, `whoami`, `cpu`, `echo`, `reboot`, `halt`).

---

### Installation & Testing

1. To compile and launch Crystal Blue OS, you need to install `gcc`, `nasm`, `ld`, and `qemu`.
2. Clone the project.
3. Compile: `make`
4. Launch in QEMU: `make run`

---

### Final Objective

The ultimate ambition for Crystal Blue OS is to become an autonomous, stable, and portable system.

* **Universal ISO Format:** Create a standard ISO image (El Torito compatible) allowing the OS to boot on any virtual machine (VMware, VirtualBox) or on a real PC via a USB drive.
* **100% Functional Kernel:**
* **Preemptive Multitasking:** The ability to run multiple programs simultaneously using the Timer.
* **File System:** Being able to create, read, and delete files on the disk (FAT32 implementation).
* **Dynamic Memory Manager:** Implementing a robust `kmalloc` to manage RAM efficiently.


* **Graphical User Interface (GUI):** Moving beyond text mode to a graphical desktop with windows and mouse support.
* **Self-hosting:** The ability to compile Crystal Blue OS from within Crystal Blue OS itself.
