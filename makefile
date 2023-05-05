# The makefile contains the necessary instructions that are to be executed to update all the listed target files based on the dependencies.
# target_file : dependencies[]
#	commands for updating the target_file based on the dependencies


jarvis : *.c ./Prompt/*.c ./Logger/*.c ./funcs/*.c
	@gcc -g -Wall *.c ./Prompt/*.c ./Logger/*.c ./funcs/*.c -o $@

clean:
	@rm jarvis
