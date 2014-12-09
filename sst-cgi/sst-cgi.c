#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#ifndef NO_UUID_LIB
#include <uuid/uuid.h>
#endif //NO_UUID_LIB

#define PAGE_SIZE			0x1000
#define MAX_CMD_LENGTH			0x100
#define MIN_PASSWORD_LENGTH		5
#define UUID_SIZE			36
static const char users_root[]     =	"users/";
static const char user_dir_inbox[] =	"inbox/";

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
		return 1;
	}
	return 0;
}

static void gen_uuid(char* buff)
{
#ifdef NO_UUID_LIB
	FILE* pf = popen("uuidgen", "r");
	if(!pf) {
		perror("uuidgen");
		return;
	}
	fread(buff, UUID_SIZE, 1, pf);
	pclose(pf);
	buff[UUID_SIZE] = 0;
#else
	uuid_t uu;
	uuid_generate_time(uu);
	uuid_unparse(uu, buff);
#endif //NO_UUID_LIB
}

static char* db_create_enter_new_user_dir(char* buff)
{
	strcpy(buff, users_root);
	gen_uuid(buff + sizeof(users_root) - 1);
	if(mkdir(buff, 0700) || chdir(buff)) perror(buff);
	return buff + sizeof(users_root) - 1;
}

static void db_make_user_dirs()
{
	if(mkdir(user_dir_inbox, 0700)) perror(user_dir_inbox);
}

static void cmd_register()
{
	char buff[PAGE_SIZE];
	if(!get_cllient_line(buff, sizeof(buff), "password", MIN_PASSWORD_LENGTH)) {
		char* uuid_str = db_create_enter_new_user_dir(buff);
		db_make_user_dirs();
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



