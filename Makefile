RM		= rm -rf

all: assembler processor

assembler:
			cd asm && make all && mv assembler ..

processor:
			cd cpu && make all && mv processor ..
			
clean:
		cd asm && make fclean
		cd cpu && make fclean
		$(RM) assembler processor

re:		clean all

