RM		= rm -rf

all: assembler disassembler processor

assembler:
			cd asm && make all && mv assembler ..

disassembler:
			cd disasm && make all && mv disassembler ..

processor:
			cd cpu && make all && mv processor ..
			
clean:
		cd asm && make fclean
		cd cpu && make fclean
		cd disasm && make fclean
		$(RM) assembler disassembler processor *.jf

re:		clean all

