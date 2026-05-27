# Maximus Script Language (MSL)

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Language: C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform: DOS](https://img.shields.io/badge/Platform-DOS-blue.svg)](https://en.wikipedia.org/wiki/DOS)

**A complete, self-contained scripting language interpreter for the Maximus BBS system.**

This tool was designed to automate user management tasks (e.g., parsing user databases, modifying access levels, finding duplicates, generating statistics) and can also serve as a general-purpose file and text processor.

---

## 📜 History & Context

This project is a time capsule from the 2001 BBS scene. The `MSL` interpreter allowed sysops to write complex scripts directly in C-like syntax, interfacing deeply with the Maximus BBS user database.

The repository contains the complete source code and a rich set of example scripts, showcasing the full capabilities of the language.

---

## ✨ Features

*   **Complete Interpreter:** A fully functional, Turing-complete language with its own parser and virtual machine, written in pure C for DOS.
*   **Rich Command Set:** Over 40 built-in commands, including:
    *   **Variables:** `integer`, `real`, `string`.
    *   **Control Flow:** `if` `go to`, `for`, `break`, `sub`/`call`/`ret` (subroutines).
    *   **Arithmetic:** Full expressions with `+`, `-`, `*`, `/`, parentheses, and operator precedence.
    *   **File I/O:** `fopen`, `fcreate`, `fclose`, `fread`, `fwrite`, `fseek`, `feof`.
    *   **BBS User Management:** Direct read/write access to user records (`rd_user`, `wr_user`, `dl_user`), and low-level field access via `define`.
    *   **String Handling:** `str_len`, `str_find`, `stri_find`.
*   **BBS-Oriented:** Built specifically to read and modify `USER.BBS` structures, including date conversion (`date_to_num`) and byte manipulation (`byte_to_num`/`int_to_byte`).
*   **Self-Contained:** The interpreter is a single executable that runs on any DOS system (including modern emulators like DOSBox).

---

## 🚀 Quick Start & Examples

You can find numerous ready-to-run scripts in the [`EXAMPLES/`](EXAMPLES/) directory.

**1. "Hello, World!"**
```
string name = 'Guest'
write('Hello, ', name, '!')
pause
exit;
```
**2. Arithmetic**
```
integer result
result = (10 - 4*2 / 4 + 13) * (((-15 - 20/5) * 2) - 25/5 + 6)
write('Result: ', result)
pause
```
**3. User Database Access**
```
integer handle, dob_day
define user_dob_day [390, 0]    ; Day of birth (byte at offset 390)

fopen('USER.BBS', handle)
rd_user(handle, 1)               ; Read first user record
byte_to_num(user_dob_day, dob_day)
write('User 1 birthday (day): ', dob_day)
fclose(handle)
pause
```
**4. File Unpacker (from Kyrandia games)**

The `EXAMPLES/FOR_PAK.MSL/` is a full-featured unpacker for PAK archives used in The Legend of Kyrandia 1 & 2.

## 🛠️ Building from Source

You can build the interpreter using **Borland C++ 3.1** under **DOSBox**.

### Prerequisites
*   [DOSBox](https://www.dosbox.com/)
*   Borland C++ 3.1 (or Turbo C++)

### Build Steps
1.  Mount your development directory in DOSBox.
2.  Navigate to the `MaximusSL/` folder.
3.  Run the provided batch file:
    ```
    FIND.BAT
    ```
This will compile the `FIND.C` source along with all necessary include files to produce `MSL.EXE`.

## 📁 Repository Structure

| Путь | Описание |
| :--- | :--- |
| `DOCS/` | Documentation |
| `EXAMPLES/` | Real-world scripts demonstrating the language |
| `EXAMPLES/DEL_*.txt` | User deletion scripts (by activity, level, etc.) |
| `EXAMPLES/FOR_PAK.MSL/` | Game resource unpacker (Kyrandia 1 & 2) |
| `EXAMPLES/BADUSER/` | Duplicate user finder |
| `EXAMPLES/...` | More examples (loops, file I/O, subroutines) |
| `INCLUDE/` | Header files and auxiliary C modules |
| `INCLUDE/USER.H` | Maximus user structure definition |
| `INCLUDE/COMMON.H` | Common type definitions |
| `INCLUDE/PROC_IF.C` | IF statement logic |
| `INCLUDE/PROC_SET.C` | Assignment and arithmetic |
| `INCLUDE/STR.C` | String manipulation functions |
| `INCLUDE/USR.C` | User database low-level access |
| `INCLUDE/OTHER.C` | Utility functions (skip_space, str_size, etc.) |
| `/` | Main interpreter source code |
| `FIND.C` | Main interpreter (version 010 beta, 40 commands) |
| `FIND.BAT` | Build script for Borland C++ 3.1 |
| `COMPILER.PRJ` | Turbo C project file |
| `COMPILER.DSK` | Turbo C workspace file |
| `LICENSE.txt` | GNU General Public License v2.0 |
| `README.md` | This file |

## 🤝 Contributing & Support

This project is an **historical archive**. While the code is considered stable and complete for its era, you are welcome to:

*   **Report issues** if you discover significant bugs.
*   **Submit pull requests** with improvements or ports to other platforms.
*   **Share your own scripts** to add to the `EXAMPLES/` directory.


---

## 📄 License

This project is open-source and distributed under the terms of the **GNU General Public License v3.0**. See the [LICENSE.txt](LICENSE.txt) file for details.

---

## 👥 Original Authors (2001)

*   **Idea:** Alex von Wolf
*   **Programming:** Andrei Agafonov
*   **Testing:** Andrei Agafonov, Kostya Chirkin

*© 2001 Most Needful Things [MNT]. Re-release for preservation, 2026.*
