flags := -lpthread -O3

%: %.c
	gcc $(flags) -o $@ $<

#so % pattern matches or anything, there is some nuance abt also using * but I'm not getting into that tbh
#flags is just so i.t's more readable 
#$@ is the target name and $< is the first prerequisite