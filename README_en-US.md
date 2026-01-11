# Xprensive: A Homemade Operating System

This is a completely self-made operating system called **Xprensive**. As it is currently not fully developed, it is named **Xprensive0**. In the future, once refined, I plan to officially release **Xprensive**, with Xprensive0 potentially becoming a branch of it. This repository uses the **Apache License**.

# About This Project

**Why this name?**
When I was in sixth grade, I made up a lot of English words for a while. When it came time to name this project, I took the lazy route and just used one of those made-up words—it doesn't have any particular meaning. Yes, I'm that casual about it.

**Why no releases published?**
My updates are small and frequent, which would make version numbers change too quickly. Also, GitHub's servers are not located in China, so the speed is slow.

**Why use command-line scripts instead of CMake for building?**
Because I don't know how to use CMake.

**Any plans to introduce other programming languages?**
No. I think Intel-style assembly and C are just fine. I don't want to introduce C++ or Rust—one is too complex, with uncontrollable overhead, and the other compiles too slowly for my computer to handle.

**Code from past versions?**
Aside from my very first few attempts, I haven't kept any.

**Why write this operating system?**
I was exposed to computer technology in elementary school and wanted to create big projects but never succeeded. Back then, I thought about making an operating system. Now that I'm older and want to learn some low-level computer knowledge, I remembered that idea and decided to write one. I believe it's something to be proud of that the code I write can run on different computers.

**How to contact me?**
Search for "编程爱好者橘儿" on CSDN (the profile picture is also a Pikachu), or send an email to `chinajuer2009@outlook.com`.

# Compilation

**How to compile?**
Follow these steps on a Linux operating system (I use the Windows Subsystem for Linux - WSL):
1. Install `nasm` (the assembler) and `gcc` (the C compiler).
2. In the terminal, run `./build.sh`. This will generate a virtual hard disk and start the system. (It's been too long; I forgot if the temporary directory contains a pure binary file or just the virtual hard disk image.)

# Other Notes

1. Please do not post irrelevant content in the Issues section.
2. I'm doing this out of passion; I'm an open-source contributor, not anyone's employee. I will do my best to fulfill reasonable requests, but please understand that I am not obligated to meet demands if treated merely as a worker.
3. Please conscientiously adhere to the open-source license.
4. This README might make me seem unapproachable, but that's not the case at all—I'm actually quite lively and outgoing.
5. English version README is translated from Chinese version README by Deepseek.I recommend you to read Chinese version README if you can