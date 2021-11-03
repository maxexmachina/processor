
NAME	= stack.a

SRCS	= $(wildcard *.cpp)

HDRS	= $(wildcard *.h)

OBJS	= $(SRCS:.cpp=.o)

CC		= g++

RM		= rm -f

CFLAGS	= -Wall -Wextra -Werror

.c.o:		
			$(CC) $(CFLAGS) -c $< -o $(<:.cpp=.o)  

$(NAME):	$(OBJS)
			ar rc $(NAME) $(OBJS)
			mv $(NAME) ../

all:		$(NAME)

clean:
			$(RM) $(OBJS)

fclean:		clean
			$(RM) $(NAME)

re:			fclean all

.PHONY:		all clean fclean re
