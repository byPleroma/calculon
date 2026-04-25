# 🖥️ Calculon (GTK)

![C](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c)
![GTK3](https://img.shields.io/badge/UI-GTK3-7fe719?style=for-the-badge&logo=gnome)
![Linux](https://img.shields.io/badge/Platform-Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![License](https://img.shields.io/badge/License-GPLv3-blue.svg?style=for-the-badge)

A high-performance calculator for Linux designed to bring the Windows 11 **Fluent Design** experience to the GTK desktop environment. Developed in pure C, featuring a decoupled architecture that strictly separates the User Interface (UI) from the Calculation Engine (Core).

<img width="332" height="497" alt="image" src="https://github.com/user-attachments/assets/f4c0474f-67a7-420f-966a-35b8e9667864" />

---

## ✨ Features

* **🎨 Windows 11 Aesthetics:** Clean interface with rounded corners, soft shadows, and a color scheme inspired by Microsoft's modern design language.
* **⚙️ Decoupled Engine:** Calculation logic is isolated in a standalone "Engine", making it easy to test and maintain.
* **⌨️ Keyboard Support:** Fully operable via keyboard (Numbers, operators, Enter for results, Backspace, and Esc).
* **🧠 Memory Functions:** Complete implementation of MC, MR, M+, M-, and MS.
* **📜 Dynamic History:** Displays the previous operation in real-time, just like the original version.
* **⚡ Performance:** Written in C with GTK3 to ensure minimal resource consumption and instant response.

---

## 🛠️ Technologies

* **Language:** C (C11 Standard)
* **Graphical Interface:** GTK+ 3.0
* **Styling:** Custom CSS (Injected via `GtkCssProvider`)
* **Dependencies:** `glib-2.0`, `gtk+-3.0`, `math.h`

---

## 🚀 How to Build and Run

### 1. Install dependencies (Debian/Ubuntu/Mint)
```bash
sudo apt update
sudo apt install build-essential libgtk-3-dev
```

### 2. Compile the project
Use GCC to link GTK3 and the math library:
```bash
gcc main.c calc-engine.c -o wf-calc `pkg-config --cflags --libs gtk+-3.0` -lm
```

### 3. Run
```bash
./wf-calc
```

---

## ⌨️ Keyboard Shortcuts

| Key | Action |
| :--- | :--- |
| `0-9` | Input digits |
| `+`, `-`, `*`, `/` | Basic operators |
| `^` | Power |
| `Enter` / `=` | Calculate result |
| `Backspace` | Erase last digit |
| `Esc` | Clear all (C) |
| `R` | Reciprocal (1/x) |
| `S` | Square Root |
| `F9` | Toggle Sign (+/-) |

---

## 📂 Project Structure

* `main.c`: Window management, GTK widgets, and Fluent CSS injection.
* `calc-engine.h`: Type definitions, states, and prototypes for the calculation logic.
* `calc-engine.c`: Robust implementation of math operations, memory management, and State Machine (FSM).

---

## 📝 License

Distributed under the **GNU General Public License v3.0 (GPLv3)**. See the `LICENSE` file for more details.

---
*Developed with a focus on logic, precision, and objectivity.*
