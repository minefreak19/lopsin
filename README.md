# Lopsin
Some kind of language? I don't even know. It has a cool virtual machine though.

Readme will be updated once we have, y'know, some actual features.

## Bytecode format
When loading a program from a file, the VM **ignores** all contents before the magic sequence (hex) `45 4c 4f 50 52 69 42 61`, defined as a macro in [lopsinvm.h](src/lopsinvm/lopsinvm.h).

This was done to allow use of shebangs (see [lopasm/main.c: usage](src/lopasm/main.c)).

## Notes
 - This project uses [my fork of tsoding's String_View library](https://github.com/minefreak19/sv)
 - This project is proudly a [nobuild](https://github.com/tsoding/nobuild) project
