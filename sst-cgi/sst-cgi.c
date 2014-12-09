#include <stdio.h>
#include <string.h>

#define PAGE_SIZE 0x1000
#define MAX_CMD_LENGTH 0x100


static void cmd_unknown()
{
	puts("Content-type: text/plain; charset=UTF-8\n\n");
	puts("ERR");
	puts("Unknown command");
}

static void cmd_echo_line()
{
	char buff[PAGE_SIZE];
	if(fgets(buff, sizeof(buff), stdin)) {
		puts("Content-type: text/plain; charset=UTF-8\n\n");
		puts("OK");
		puts(buff);
	}
}

static void* dispatch_cmd()
{
	char cmd_buff[MAX_CMD_LENGTH];
	if(!fgets(cmd_buff, sizeof(cmd_buff), stdin)) return cmd_unknown;
	if(!strcmp(cmd_buff, "echo-line\n")) return cmd_echo_line;
	return cmd_unknown;
}

int main(int argc, char* argv[])
{
	void (*cmd_handler)() = dispatch_cmd();
	cmd_handler();
	return 0;

}



