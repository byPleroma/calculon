# 🖥️ Calculon (GTK)

![C](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c)
![GTK3](https://img.shields.io/badge/UI-GTK3-7fe719?style=for-the-badge&logo=gnome)
![Linux](https://img.shields.io/badge/Platform-Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![License](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)

Uma calculadora de alta performance para Linux que traz a estética **Fluent Design** do Windows 11 para o ambiente desktop GTK. Desenvolvida em C puro, com uma arquitetura desacoplada que separa a interface (UI) do motor de cálculo (Core).

<img width="432" height="597" alt="image" src="https://github.com/user-attachments/assets/ec7187c2-e64e-40ef-92bd-4d6132a7c5fd" />


---

## ✨ Destaques

* **🎨 Estética Windows 11:** Interface limpa com bordas arredondadas, sombras suaves e esquema de cores inspirado no design moderno da Microsoft.
* **⚙️ Engine Desacoplada:** Lógica de cálculo isolada em um "Engine" puro, facilitando testes e manutenção.
* **⌨️ Suporte a Teclado:** Totalmente operável via teclado (Números, operadores, Enter para resultado, Backspace e Esc).
* **🧠 Funções de Memória:** Implementação completa de MC, MR, M+, M- e MS.
* **📜 Histórico Dinâmico:** Exibe a operação anterior em tempo real, assim como na versão original.
* **⚡ Performance:** Escrita em C com GTK3 para garantir o mínimo consumo de recursos e resposta instantânea.

---

## 🛠️ Tecnologias

* **Linguagem:** C (Padrão C11)
* **Interface Gráfica:** GTK+ 3.0
* **Estilização:** CSS Customizado (Injetado via `GtkCssProvider`)
* **Dependências:** `glib-2.0`, `gtk+-3.0`, `math.h`

---

## 🚀 Como Compilar e Rodar

### 1. Instale as dependências (Debian/Ubuntu/Mint)
```bash
sudo apt update
sudo apt install build-essential libgtk-3-dev
```

### 2. Compile o projeto
Utilize o GCC para linkar o GTK3 e a biblioteca matemática:
```bash
gcc main.c calc-engine.c -o wf-calc `pkg-config --cflags --libs gtk+-3.0` -lm
```

### 3. Execute
```bash
./wf-calc
```

---

## ⌨️ Atalhos de Teclado

| Tecla | Ação |
| :--- | :--- |
| `0-9` | Insere dígitos |
| `+`, `-`, `*`, `/` | Operadores básicos |
| `^` | Potência |
| `Enter` / `=` | Calcula o resultado |
| `Backspace` | Apaga o último dígito |
| `Esc` | Limpa tudo (C) |
| `R` | Recíproca (1/x) |
| `S` | Raiz Quadrada |
| `F9` | Alterna Sinal (+/-) |

---

## 📂 Estrutura do Projeto

* `main.c`: Gerenciamento da janela, widgets GTK e injeção de CSS Fluent.
* `calc-engine.h`: Definições de tipos, estados e protótipos da lógica de cálculo.
* `calc-engine.c`: Implementação robusta das operações matemáticas, gestão de memória e estados (FSM).

---

## 📝 Licença

Este projeto está sob a licença MIT. Sinta-se livre para usar, modificar e distribuir.

---
*Desenvolvido com foco em lógica, precisão e objetividade.*
