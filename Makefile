NAME = ctimer

all: $(NAME)

run: $(NAME)
	./$(NAME) $(A)

$(NAME): $(NAME).c style.h
	cc $(NAME).c -o $(NAME) -Wall

clean:
ifeq ($(OS),Windows_NT)
	del /q $(NAME)
	del /q $(NAME).exe
else
	rm -f $(NAME)
	rm -f $(NAME).exe
endif

