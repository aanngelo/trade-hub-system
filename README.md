# Trade Hub System

A simple marketplace program written in C where students can buy and sell school items like uniforms, books, and supplies.

---

## Overview

Trade Hub is a program that runs in the terminal. It lets students trade school items with each other. There are two roles:

- **Buyer** — Look at items, search for what you need, add to cart, and place an order
- **Seller** — Add items for sale, manage your listings, and see your sales history

The program uses a text-based menu that you navigate by typing numbers.

---

## Data Structures and Algorithms Used

| Data Structure / Algorithm | File | What It Does |
|---|---|---|
| Linked List | `catalog.c` | Stores the list of items for sale |
| Hash Table | `hash.c` | Quickly finds an item by its ID |
| Queue | `queue.c` | Handles buyer orders one by one (first come, first served) |
| Stack | `stack.c` | Keeps track of past transactions |
| Sorting Algorithm | `catalog.c` | Sorts items by price or name |
| Search Algorithm | `catalog.c` / `hash.c` | Searches for items in the catalog |

---

## Project Files

```
trade-hub-system/
├── main.c          # Starts the program, shows the main menu
├── catalog.c/h     # List of items using linked list
├── cart.c/h        # Shopping cart
├── queue.c/h       # Order queue
├── stack.c/h       # Transaction history
├── hash.c/h        # Item search using hash table
└── utils.c/h       # Helper functions
```

---

## How to Compile and Run

### What You Need
- GCC compiler (MinGW for Windows)

### Step 1: Compile

```bash
gcc main.c catalog.c cart.c queue.c stack.c hash.c utils.c -o trade_hub
```

### Step 2: Run

**Windows:**
```bash
trade_hub.exe
```

**Linux/Mac:**
```bash
./trade_hub
```

---

## Developed By

- Beraquit, Jinky
- Lorio, Angelo
- Montañez, April Ann
- Salud, Sandara

Central Bicol State University of Agriculture — Sipocot (CBSUA-SIPOCOT)  
CC104 — Data Structures and Algorithms

---

## AI Tools Used

### Claude AI (claude.ai)
We used Claude to help plan and generate parts of the code.

Prompts we used:
- Build a marketplace system in C using data structures
- Add a hash table for searching items
- Add a queue for managing orders
- Add a stack for transaction history
- Design a text-based menu for buyer and seller

What we changed:
We checked the code, tested it, and fixed bugs. We also put all the parts together and adjusted the program based on what worked during testing.

### Cursor AI (Cursor Editor)
We used Cursor as our code editor. It helped by suggesting code while we were typing.

### GitHub Copilot (VS Code)
We also used GitHub Copilot inside VS Code. It helped complete some parts of the code while we were writing and fixing it.
