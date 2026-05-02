# Trade Hub System

A console-based marketplace application written in C that allows students to buy and sell school-related items such as uniforms, books, and school supplies.

---

## Overview

Trade Hub is a terminal-based system designed for students who want to trade school items within their campus community. It supports two roles:

- **Buyer** — Browse the catalog, search for items, add to cart, and place orders
- **Seller** — List items for sale, manage inventory, and view transaction history

The system features a menu-driven ASCII console interface with full marketplace functionality.

---

## Data Structures and Algorithms Used

| Data Structure / Algorithm | Module | Feature It Supports |
|---|---|---|
| Linked List | `catalog.c` | Stores and manages the product catalog dynamically |
| Hash Table | `hash.c` | Fast item lookup and search by item ID |
| Queue | `queue.c` | Manages buyer order queue (FIFO processing) |
| Stack | `stack.c` | Tracks transaction history (undo/view last actions) |
| Sorting Algorithm | `catalog.c` | Sorts items by price or name |
| Search Algorithm | `catalog.c` / `hash.c` | Linear and hash-based item search |

---

## Project Structure

```
trade-hub-system/
├── main.c          # Entry point, main menu
├── catalog.c/h     # Product catalog using linked list
├── cart.c/h        # Shopping cart management
├── queue.c/h       # Order queue (FIFO)
├── stack.c/h       # Transaction history (LIFO)
├── hash.c/h        # Hash table for item search
└── utils.c/h       # Utility/helper functions
```

---

## How to Compile and Run

### Requirements
- GCC compiler (MinGW for Windows)

### Compile

```bash
gcc main.c catalog.c cart.c queue.c stack.c hash.c utils.c -o trade_hub
```

### Run

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

## AI Acknowledgment

AI Tool Used: Claude AI (claude.ai)

Prompts Used:
- Generate a console-based marketplace system in C using data structures
- Implement hash table for item search
- Implement queue for order management
- Implement stack for transaction history
- Design ASCII console UI for buyer and seller roles

How the output was adapted:
The generated code was reviewed, tested, and modified to fit the requirements of the Trade Hub System. Bugs were fixed manually, modules were integrated together, and the UI flow was adjusted based on actual testing.
