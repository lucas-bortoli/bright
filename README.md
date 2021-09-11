# bright
A tiny C program to set the laptop display brightness.

## Usage
```
    bright %|+%|-%

VAL must be a percentage. %=100 means the maximum brightness supported by the video driver, and %=0 means the minimum.
+/- specifies relative change.

EXAMPLE: 
    bright 25    : set brightness to 25%
    bright -10   : set brightness 10% lower
```

## Compiling

```
git clone https://github.com/lucas-bortoli/bright
cd bright
gcc -o bright bright.c
```

