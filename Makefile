RM		= rm -rf

all: assembler disassembler processor

assembler:
			cd asm && make all && mv assembler ../build

disassembler:
			cd disasm && make all && mv disassembler ../build

processor:
			cd cpu && make all && mv processor ../build
			
clean:
		cd asm && make fclean
		cd cpu && make fclean
		cd disasm && make fclean
		$(RM) build/assembler build/disassembler build/processor *.jf 

re:		clean all

