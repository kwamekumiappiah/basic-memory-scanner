# 🔍 Linux Memory Scanner

> A modular, low-overhead **Linux CLI memory scanner written in C**. Point it at a running process, search for an integer value, change the value in the target, and watch the candidate list shrink down to the real address. 🎯

**[kwamekumiappiah/basic-memory-scanner](https://github.com/kwamekumiappiah/basic-memory-scanner)**: a simple C project meant to solidify my understanding of low-level programming and manipulation of memory in C by building a program that scans memory for values. 🧠

[![GitHub Repo](https://img.shields.io/badge/GitHub-basic--memory--scanner-181717?logo=github)](https://github.com/kwamekumiappiah/basic-memory-scanner)
[![LinkedIn](https://img.shields.io/badge/LinkedIn-Kwame%20Appiah%20Kumi-0A66C2?logo=linkedin&logoColor=white)](https://www.linkedin.com/in/kwameappiah-kumi-appiah/)

![Language](https://img.shields.io/badge/language-C-blue?logo=c)
![Platform](https://img.shields.io/badge/platform-Linux-yellow?logo=linux&logoColor=white)
![Tests](https://img.shields.io/badge/tests-unit%20%2B%20integration-brightgreen)
![Purpose](https://img.shields.io/badge/purpose-educational-purple)

---

## 📑 Table of Contents

- [📌 Project Overview & Key Learning Outcomes](#-project-overview--key-learning-outcomes)
- [🧠 How It Works](#-how-it-works)
- [🗂️ Project Structure](#️-project-structure)
- [🚀 Getting Started](#-getting-started)
- [🎮 Example Session](#-example-session)
- [🧪 Testing](#-testing)
- [🐛 Bugs Encountered & Workarounds](#-bugs-encountered--workarounds)
- [⚠️ Known Limitations](#️-known-limitations)
- [🛣️ Roadmap](#️-roadmap)
- [🔐 Permissions & Responsible Use](#-permissions--responsible-use)
- [👨‍💻 Author & Contact](#-author--contact)

---

## 📌 Project Overview & Key Learning Outcomes

### 🎯 Project Goals
Build a **modular, low-overhead Linux CLI memory scanner in C** capable of inspecting remote process memory and interactively filtering candidate addresses across variable state changes.

### 🗺️ Virtual Memory Parsing
Learned how the Linux kernel exposes virtual address spaces via `/proc/[pid]/maps`. The scanner filters specifically for **readable and writable (`rw`)** segments, ignoring code and read-only segments, since that's where mutable values like health, score, and ammo live.

### ⚡ Low-Level System Calls
Used `/proc/[pid]/mem` with **`pread()`** rather than `lseek()` + `read()`:

- ✅ Reads directly at a specific byte offset
- ✅ Does not move the file descriptor's offset (no shared state to corrupt)
- ✅ One system call instead of two, so less overhead per read

### 🛡️ Data Encapsulation
Dynamic heap allocations are managed through **opaque structs** (`RegionList` and `AddressList`). The header only exposes the type names, so `main.c` can never touch the internals of `memory_scanner.c`. That gives strict information hiding and a clean interface boundary.

### 🎮 How Game Cheat Engines Work
Replicates the core loop used by tools like Cheat Engine:

1. 📸 **Snapshot**: scan the target's memory for an initial value (e.g., player health)
2. ⏸️ **Pause**: let the value change in the target process
3. 🔁 **Refine**: re-read *only the surviving candidates* and keep those matching the new value
4. 🏁 **Repeat** until one address remains, eliminating false positives along the way

---

## 🧠 How It Works

```
   /proc/<pid>/maps                 /proc/<pid>/mem
         │                                 │
         ▼                                 ▼
  ┌──────────────┐   rw regions   ┌────────────────┐
  │ parse_maps   │ ─────────────► │ read_memory    │
  │ _line()      │                │ _segment()     │
  └──────────────┘                └───────┬────────┘
                                          │ raw bytes
                                          ▼
                                  ┌────────────────┐
                                  │ scan_buffer()  │  ◄── target value
                                  └───────┬────────┘
                                          │ candidate addresses
                                          ▼
                                  ┌────────────────┐
                        ┌────────►│ filter_        │──┐
       new value ───────┘         │ addresses()    │  │ still matching
                                  └────────────────┘  │
                                          ▲───────────┘
                                          │
                                   repeat until 1 left ✨
```

<details>
<summary>📖 <b>Click to expand: the public API (<code>memory_scanner.h</code>)</b></summary>

| Function | Purpose |
|---|---|
| `create_region_list()` / `free_region()` | 🗺️ Create and free the list of memory regions |
| `get_region_count()` | 🔢 Number of regions stored |
| `parse_maps_line()` | 📄 Parse one line of `/proc/pid/maps`, keep it if it is `rw` |
| `create_address_list()` / `free_addr_list()` | 📍 Create and free the candidate address list |
| `add_address()` | ➕ Append a candidate (grows the array automatically) |
| `get_address_count()` | 🔢 Number of candidates remaining |
| `read_memory_segment()` | 📥 Read one whole region into a heap buffer with `pread()` |
| `scan_buffer()` | 🔎 Find every occurrence of the target `int` in a buffer |
| `filter_addresses()` | 🧹 Re-read candidates and keep only those equal to the new value |
| `print_addresses()` | 🖨️ Print each candidate address with its current value |

</details>

---

## 🗂️ Project Structure

```
memory_scanner/
├── 📁 build/                  # Compiled binaries (generated)
│   ├── scanner
│   ├── target_program
│   └── test_lists
├── 📁 include/
│   └── memory_scanner.h       # Public interface (opaque types)
├── 📁 src/
│   ├── main.c                 # CLI driver + interactive refinement loop
│   ├── memory_scanner.c       # Scanner implementation
│   └── target_program.c       # Dummy victim process with a "health" variable
├── 📁 test/
│   ├── test_lists.c           # Unit tests (no root needed)
│   └── integration.sh         # End-to-end test (needs sudo)
├── Makefile
└── README.md
```

---

## 🚀 Getting Started

### 📋 Requirements
- 🐧 Linux (uses the `/proc` filesystem)
- 🔧 `gcc` and `make`
- 🔑 `sudo`, or permission to ptrace the target (see [Permissions](#-permissions--responsible-use))

### 📥 Get the Code

```bash
git clone https://github.com/kwamekumiappiah/basic-memory-scanner.git
cd basic-memory-scanner
```

### 🔨 Build

```bash
make            # builds build/scanner and build/target_program
make clean      # removes build/
```

### ▶️ Run

```bash
./build/scanner <pid> <target_int_value>
```

| Argument | Meaning |
|---|---|
| `<pid>` | 🆔 Process ID of the process to scan |
| `<target_int_value>` | 🔢 The integer to search for |

During refinement, enter a new value each round, or **`-999`** to exit. 👋

---

## 🎮 Example Session

**🖥️ Terminal 1: start the victim**

```console
$ ./build/target_program
Current Process ID: 6652

Health value: 100
Health memory address: 0x7ffd49d41690
Enter new health value:
```

**🖥️ Terminal 2: scan for `100`**

```console
$ sudo ./build/scanner 6652 100
[+] Initial Pass Complete. Candidates Found: 37
Address: 0x55d0c0a012a0 | Value: 100
Address: 0x7ffd49d41690 | Value: 100
...
```

**🔁 Back in Terminal 1**, type `73` so the value changes. Then in Terminal 2:

```console
Enter updated target value (-999 to exit): 73
[+] Refinement Pass Complete. Remaining Candidates: 1
Address: 0x7ffd49d41690 | Value: 73
```

🎉 **Found it!** It matches the address printed by the target program.

> 💡 The candidate counts above are illustrative. Real numbers depend on your system.

---

## 🧪 Testing

The project has **two layers of tests**, because the two halves of the code need different kinds of checks:

| Test | File | Needs root? | What it checks |
|---|---|---|---|
| 🧩 Unit | `test/test_lists.c` | ❌ No | List growth, `NULL` safety, `rw`-only map parsing, buffer scanning |
| 🔗 Integration | `test/integration.sh` | ✅ Yes | Launches the real target, scans it, changes the value, and verifies the true `health` address survives refinement |

```bash
make test
```

Expected output:

```
All unit tests passed.
target pid=6652  health address=0x7ffd49d41690
PASS: scanner found health at 0x7ffd49d41690
```

<details>
<summary>🩺 <b>Click to expand: troubleshooting the tests</b></summary>

- ⏱️ **"timed out waiting for..."**: the target's prompt isn't reaching the log. Make sure `target_program.c` calls `fflush(stdout);` after its prompts, since output to a file is fully buffered.
- 🔒 **Permission denied**: run through `sudo`, or check `/proc/sys/kernel/yama/ptrace_scope`.
- 🛑 **"No rule to make target ..."**: check the filename matches the Makefile (`test_lists.c`, with an `s`).
- 📁 **`make test` says "up to date"**: the `test/` directory shares its name with the target. Make sure `test` is listed in `.PHONY`.

</details>

---

## 🐛 Bugs Encountered & Workarounds

| # | 🐞 Bug | 🔎 Cause | ✅ Fix |
|---|---|---|---|
| 1 | `incomplete typedef 'AddressList'` | `main.c` tried to access internal fields (e.g., `candidates->count`) of an opaque struct | Added the `get_address_count()` accessor so callers query counts safely |
| 2 | `too many arguments to scan_buffer` | `scan_buffer()` was called with 6 arguments in `main.c` but declared with 5 in the header | Aligned the prototypes in `memory_scanner.h` and `memory_scanner.c` to accept `AddressList *addr_list` |
| 3 | `undefined reference to get_region_count` | Declared in the header but never defined, so the compiler was satisfied and the **linker** failed | Added the missing function body to `memory_scanner.c` |
| 4 | `make test`: *No rule to make target 'test/test_lists.c'* | The file was named `test_list.c` (no `s`) but the Makefile expected `test_lists.c` | Renamed the file to match the Makefile |
| 5 | Integration test could hang or time out | Prompt output was buffered when stdout was redirected to a file | Added `fflush(stdout)` to the target program |

> 🧠 **Lesson learned:** compile errors and link errors are different failures. A header can promise a function that no `.c` file delivers, and only the linker will notice.

---

## ⚠️ Known Limitations

- 🔢 Only searches for **32-bit signed integers** (`int`)
- 📖 Only scans **`rw`** regions (values in read-only or executable mappings are skipped)
- 🚫 Some `rw` regions (e.g., `[vvar]`) can't be read via `/proc/pid/mem`. Failed reads are skipped.
- 📏 Partial reads: a region that is only partly readable should use the actual number of bytes read, not the full region size
- 🧵 The target's memory can change while it's being scanned, so the results are a snapshot, not a frozen view
- 🖨️ The first pass prints every match, which can be thousands of lines on a real process

---

## 🛣️ Roadmap

Ideas for taking the project further. Tick them off as you go! ✅

- [ ] ✍️ **Write mode**: open `/proc/pid/mem` with `O_RDWR` and `pwrite()` a new value to a chosen address
- [ ] 🔢 **More value types**: `float`, `double`, 64-bit ints, 16-bit shorts, and byte patterns
- [ ] 🎚️ **Smarter filters**: "changed", "unchanged", "increased", "decreased" (for when you don't know the exact value)
- [ ] ⚡ **Aligned scanning**: step by `sizeof(int)` for roughly 4x faster scans
- [ ] 🧷 **Safe integer reads**: use `memcpy` instead of casting to `int *` to avoid unaligned access
- [ ] 🧹 **Quieter output**: drop the `printf` inside `scan_buffer` and cap how many candidates are printed
- [ ] 🚦 **Check `add_address()` return values** so allocation failures aren't silently ignored
- [ ] 🧱 **Chunked reads**: read big regions in smaller blocks to reduce memory usage
- [ ] 🏷️ **Region labels**: show which region (`[heap]`, `[stack]`, a library) each address belongs to
- [ ] 🧪 **More tests**: verify that refinement drops addresses whose value changed
- [ ] 🤖 **CI**: run the unit tests automatically on every push

---

## 🔐 Permissions & Responsible Use

Reading another process's memory is restricted on purpose:

- 👤 You generally need to be the **same user** as the target, plus ptrace permission, or be **root**
- 🛡️ The **Yama** security module (`/proc/sys/kernel/yama/ptrace_scope`) can block access to processes that aren't your descendants, which is why the tests use `sudo`
- ⚖️ This tool is built **for learning**. Only use it on processes you own or have explicit permission to inspect. Don't use it against online games or software whose terms forbid memory tampering.

---

## 👨‍💻 Author & Contact

Built by **Kwame Appiah Kumi-Appiah** as a hands-on way to solidify low-level programming skills and memory manipulation in C. 💪

| | Link |
|---|---|
| 📦 **Project repo** | [github.com/kwamekumiappiah/basic-memory-scanner](https://github.com/kwamekumiappiah/basic-memory-scanner) |
| 🐙 **GitHub** | [github.com/kwamekumiappiah](https://github.com/kwamekumiappiah) |
| 💼 **LinkedIn** | [linkedin.com/in/kwameappiah-kumi-appiah](https://www.linkedin.com/in/kwameappiah-kumi-appiah/) |
| 📧 **Email** | [kwameappiahkumi@gmail.com](mailto:kwameappiahkumi@gmail.com) |

💬 Feedback, questions, and pull requests are welcome. If this project helped you, drop a ⭐ on the repo!

---

<p align="center">
  Built with ☕ and curiosity while exploring how Linux processes really work. 🐧
</p>