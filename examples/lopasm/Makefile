LOPASM = ../../bin/lopasm
LOPSINVM = ../../bin/lopsinvm

LOPASMFLAGS += --vm $(LOPSINVM) 

LOPASM_SRC = $(wildcard *.lopasm)
LOPSINVM_OUT = $(LOPASM_SRC:lopasm=lopsinvm)

.PHONY: all clean
all: $(LOPSINVM_OUT)

%.lopsinvm: %.lopasm
	$(LOPASM) $(LOPASMFLAGS) -o $@ $<

clean:
	rm $(LOPSINVM_OUT)
