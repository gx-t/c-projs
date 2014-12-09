#include <stdio.h>
#include <string.h>
#include <uuid/uuid.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define PAGE_SIZE			0x1000
#define MAX_CMD_LENGTH			0x100
#define MIN_PASSWORD_LENGTH		5
static const char users_root[] =	"users/";
#define UUID_SIZE			36

static void resp_text_ok()
{
	puts("Content-type: text/plain; charset=UTF-8\n");
	puts("OK");
}

static void resp_text_err()
{
	puts("Content-type: text/plain; charset=UTF-8\n");
	puts("ERR");
}

static void cmd_unknown()
{
	resp_text_err();
	puts("Unknown command");
}

static void cmd_echo_line()
{
	char buff[PAGE_SIZE];
	if(fgets(buff, sizeof(buff), stdin)) {
		resp_text_ok();
		puts(buff);
	}
}

static int get_cllient_line(char* buff, size_t buff_size, const char* field_name, size_t min_length)
{
	if(!fgets(buff, buff_size, stdin) || strlen(buff) < min_length) {
		resp_text_err();	
		printf("You must specify at least %lu character long %s\n", min_length, field_name);
		return 0;
	}
	return 1;
}

static char* db_create_enter_new_user_dir(char* buff)
{
	uuid_t uu;
	memset(buff, 0, sizeof(users_root) + UUID_SIZE);
	uuid_generate_time(uu);
	strcpy(buff, users_root);
	uuid_unparse(uu, buff + sizeof(users_root) - 1);
	if(mkdir(buff, 0700) || chdir(buff)) perror(buff);
	return buff + sizeof(users_root) - 1;
}

static void cmd_register()
{
	char buff[PAGE_SIZE];
	if(!get_cllient_line(buff, sizeof(buff), "password", MIN_PASSWORD_LENGTH)) {
		char* uuid_str = db_create_enter_new_user_dir(buff);
		resp_text_ok();	
		puts(uuid_str);
	}
}

static void* dispatch_cmd()
{
	char cmd_buff[MAX_CMD_LENGTH];
	if(!fgets(cmd_buff, sizeof(cmd_buff), stdin)) return cmd_unknown;
	if(!strcmp(cmd_buff, "echo-line\n")) return cmd_echo_line;
	if(!strcmp(cmd_buff, "register\n")) return cmd_register;
	return cmd_unknown;
}

int main(int argc, char* argv[])
{
	void (*cmd_handler)() = dispatch_cmd();
	cmd_handler();
	return 0;

}



