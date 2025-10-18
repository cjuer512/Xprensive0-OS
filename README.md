# Xprensive0
This is an operating system under development.
Why I named this Xprensive"0" is  because this is in develop and very simple.
If you want to communicate with me,please send email to chinajuer2009@outlook.com.You need write email in Chinese(both simplified Chinese and traditional Chinese) or English.If you write in other language,maybe I won't reply with you.I am only able to write email in English and Chinese.
You must write comments in English or Chinese.The code you write must be in C or Intel assembly, and helper scripts can use Python.
# How to Build
1.make sure you have installed:
NASM version 3.01 compiled on Oct 10 2025,python 3.13.7  
2.download all codes.  
3.in cmd run `nasm -f bin boot.asm -o boot.bin `  
4.in cmd run `python imgmake.py boot.asm boot.bin  
Now,you get "boot.img".you can use it.
