# FOS – Operating System Course Project

This project is a mini operating system simulation developed as part of the Operating Systems course at Ain Shams University. It follows a structured educational template provided by the faculty and aims to give hands-on experience with core OS concepts such as memory management, process scheduling, and inter-process communication.

---

## 🧠 Overview

The project simulates fundamental components of an operating system, including:

- **Kernel and User Stack/Heap Management**  
- **Process Scheduling and Context Switching**  
- **Memory Management using Paging**  
- **Thread Safety via Locks and Semaphores**  
- **Inter-Process Communication through Shared Objects**  

---

## ⚙️ Features Implemented

### ✅ Kernel & Memory Layout
- Designed and initialized both **kernel and user-level stack and heap memory segments**.
- Ensured memory isolation between processes.

### 🗃️ Process Management
- Created and maintained **process control blocks (PCBs)**.
- Implemented lifecycle management: creation, termination, and scheduling metadata.

### 📦 Memory Management
- Employed **paging** as the main memory allocation technique.
- Designed a basic page replacement policy (if required by use case).

### 🧵 Multithreading and Synchronization
- Developed synchronization tools including:
  - **Locks**: for critical section protection.
  - **Semaphores**: to handle concurrent access among multiple processes.

### 🔁 CPU Scheduling
- Implemented **Round Robin** scheduling algorithm for fairness across time-sliced processes.
- Simulated context switching and time quantum logic.

### 🔄 Shared Objects & IPC
- Built support for **shared memory objects** between processes.
- Allowed processes to communicate through defined shared regions securely.

---

## 🛠️ Tools & Technologies

- **Language**: C  
- **IDE**: Eclipse  
- **Compiler**: GCC  
- **Platform**: Simulated educational kernel (user-space only)

---

## 📚 Educational Purpose

This project was created under the supervision of Ain Shams University as a part of the FOS (Fundamentals of Operating Systems) curriculum. It serves as a practical reinforcement of key theoretical OS concepts and simulates behavior in a controlled environment.

---

> ⚠️ This project is not intended to be run on real hardware. It is a user-space simulation for educational purposes only.

